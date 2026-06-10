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
        mTransferByte = 0;
        mBitsTransferred = 0;
        mTransferActive = false;
        mIsFirstTransfer = (isCGB == false);
    }

    auto Serial::Clock (const std::uint64_t& pCycle) -> bool
    {
        // Component Clock Rate: 1 Dot per 2 CPU Cycles
        // if ((pCycle & 1) != 1)
        //     { return true; }

        // Only tick if a transfer is active with the internal clock.
        // - If not, then continue if a transfer was just requested.
        if (mTransferActive == false)
        {
            if (mControl.mTransferEnable == true &&
                mControl.mInternalClock == true)
            {
                mTransferActive = true;
                mBitsTransferred = 0;
                mDotCounter = 0;
                mTransferByte = mByte;
                mIsFirstTransfer = false;
            }
            else
                { return true; }
        }

        // Determine the number of dots per bit.
        std::uint16_t dotsPerBit = 512;
        if (mSystem.IsCGB() == true)
        {
            if (mSystem.GetCPU().IsHighSpeed() == true)
                { dotsPerBit = (mControl.mHighSpeed == true) ? 16 : 32; }
            else
                { dotsPerBit = (mControl.mHighSpeed == true) ? 256 : 512; }
        }

        // Advance the dot counter.
        if (++mDotCounter >= dotsPerBit)
        {
            mDotCounter = 0;

            // Grab the next bit from the external source.
            // - If there is no source, then assume a default value of `1`.
            std::uint8_t bit = 1;
            if (mProgressDelegate != nullptr)
            {
                bool good = mProgressDelegate(mSystem, ((mByte >> 7) & 0b1), bit);
                if (good == false)
                {
                    mTransferActive = false;
                    mControl.mTransferEnable = false;
                    if (mFinishCallback != nullptr)
                        { mFinishCallback(mSystem, mTransferByte, mByte); }

                    return true;
                }
            }

            // Shift `SB` left by one bit, pulling in the received bit.
            mByte = (mByte << 1) | (bit & 0b1);
            if (++mBitsTransferred >= 8)
            {
                mTransferActive = false;
                mControl.mTransferEnable = false;
                mSystem.GetCPU().RequestInterrupt(stx::under(Interrupt::Serial));
                if (mFinishCallback != nullptr)
                    { mFinishCallback(mSystem, mTransferByte, mByte); }
            }
        }

        return true;
    }
}

// Public Methods - Callbacks **************************************************

namespace G10::GB
{
    auto Serial::SetProgressDelegate (const SerialProgressDelegate& pDelegate) -> void
        { mProgressDelegate = pDelegate; }
    auto Serial::SetFinishCallback (const SerialFinishCallback& pCallback) -> void
        { mFinishCallback = pCallback; }
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
