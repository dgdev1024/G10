/**
 * @file    G10.GB.Serial.cpp
 * @brief   Contains implementations for the G10.Boy's serial communication
 *          component, and related definitions.
 */

// Includes ********************************************************************

#include <G10.GB.System.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::GB
{
    Serial::Serial (System& pSystem) :
        mSystem     { pSystem }
    {
    }
}

// Public Methods **************************************************************

namespace G10::GB
{
    auto Serial::Reset () -> void
    {
        bool isCGB = mSystem.IsCGB();

        // Port Registers
        mByte = 0x00;
        if (isCGB == true)
        {
            mControl.mValue = 0x7F;
        }
        else
        {
            mControl.mValue = 0x7E;
        }

        // Internal State
        mDotCounter = 0;
        mBitsTransferred = 0;
        mPeerByte = 0xFF;
        mTransferActive = false;
        mWaitingForPeer = false;
        mPeerByteReady = false;
    }

    auto Serial::Clock (const std::uint64_t& pCycle) -> bool
    {
        // Component Clock Rate: 1 Dot per 2 CPU Cycles
        if ((pCycle & 1) != 1)
            { return true; }

        if (mTransferActive == false)
            { TryStartTransfer(); }
        else
            { ClockTransfer(); }

        return true;
    }

    auto Serial::ReceiveByte (const std::uint8_t pByte) -> void
    {
        if (mControl.mInternalClock == true)
        {
            // Host receives client's reply byte.
            mPeerByte = pByte;
            mWaitingForPeer = false;
            mPeerByteReady = true;
        }
        else
        {
            // Client receives host's clock / byte.
            // Send our `SB` back to acknowledge the host.
            if (mTransmitCallback != nullptr)
                { mTransmitCallback(mSystem, mByte); }    
            mPeerByte = pByte;

            // If we've already enabled the transfer on our end, proceed.
            // If not, queue the byte so we catch it when we do enable it to 
            // prevent jitter desync.
            if (mTransferActive == true && mWaitingForPeer == true)
                { mWaitingForPeer = false; }
            else
                { mPeerByteReady = true; }
        }
    }
}

// Public Methods - Callbacks **************************************************

namespace G10::GB
{
    auto Serial::SetTransmitCallback (const SerialTransmitCallback& pCallback) -> void
        { mTransmitCallback = pCallback; }
}

// Public Methods - Hardware Registers *****************************************

namespace G10::GB
{
    auto Serial::ReadSB (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mByte;
        return true;
    }

    auto Serial::ReadSC (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - Bits 2 through 6 are unused; read `1`.
        // - Bit 1 is readable in CGB Mode; unused and reads `1` in DMG Mode.
        // - Bits 0 and 7 are readable.
        if (mSystem.IsCGB() == true)
            { pDataOut = (mControl.mValue & 0b10000011) | 0b01111100; }
        else
            { pDataOut = (mControl.mValue & 0b10000001) | 0b01111110; }

        return true;
    }

    auto Serial::WriteSB (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - All bits writable.
        mByte = pDataIn;
        return true;
    }

    auto Serial::WriteSC (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - Bits 2 through 6 are unused; write `1`.
        // - Bit 1 is writable in CGB Mode; unused and writes `1` in DMG Mode.
        // - Bits 0 and 7 are writable.
        if (mSystem.IsCGB() == true)
            { mControl.mValue = (pDataIn & 0b10000011) | 0b01111100; }
        else
            { mControl.mValue = (pDataIn & 0b10000001) | 0b01111110; }

        return true;
    }
}

// Private Methods *************************************************************

namespace G10::GB
{
    auto Serial::TryStartTransfer () -> void
    {
        // Early out if transfer is not currently enabled.
        if (mControl.mTransferEnable == false)
            { return; }

        // Prepare to start transfer.
        mBitsTransferred = 0;
        mDotCounter = 0;
        mTimeout = 0;
        mTransferActive = true;
        mWaitingForPeer = true;

        if (mControl.mInternalClock == true)
        {
            // Host: Transmit byte and wait for reply.
            mPeerByteReady = false;
            if (mTransmitCallback != nullptr)
                { mTransmitCallback(mSystem, mByte); }
            else
                { ReceiveByte(0xFF); /* Link cable disconnected. */ }
        }
        else
        {
            // Client: Wait for host to clock us.
            if (mPeerByteReady == true)
            {
                mWaitingForPeer = false;
                mPeerByteReady = false;
            }
        }
    }
    
    auto Serial::ClockTransfer () -> void
    {
        if (mWaitingForPeer == true)
        {
            if (++mTimeout >= 0x3FFFFF)
            {
                AbortTransfer();
            }

            return;
        }

        if (++mDotCounter < GetDotsPerBit())
            { return; }
        else
            { mDotCounter = 0; }

        const std::uint8_t recvBit = static_cast<std::uint8_t>(
            (mPeerByte >> (7 - mBitsTransferred)) & 0b1);
        mByte = static_cast<std::uint8_t>((mByte << 1) | recvBit);

        if (++mBitsTransferred >= 8)
        {
            mTransferActive = false;
            mControl.mTransferEnable = false;
            mSystem.GetCPU().RequestInterrupt(stx::under(Interrupt::Serial));
        }
    }
    
    auto Serial::AbortTransfer () -> void
    {
        mTimeout = 0;
        mTransferActive = false;
        mWaitingForPeer = false;
        mPeerByteReady = false;
        mBitsTransferred = 0;
        mDotCounter = 0;
        mPeerByte = 0xFF;
        mControl.mTransferEnable = false;
    }
    
    auto Serial::GetDotsPerBit () const -> std::uint16_t
    {
        if (mSystem.IsCGB() == true)
        {
            if (mSystem.GetCPU().IsHighSpeed() == true)
                { return (mControl.mHighSpeed == true) ? 16 : 32; }
            else
                { return (mControl.mHighSpeed == true) ? 256 : 512; }
        }
        return 512;
    }
}
