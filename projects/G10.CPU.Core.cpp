/**
 * @file    G10.CPU.Core.cpp
 * @brief   Contains implementations for the G10 CPU Core class, and related
 *          definitions.
 */

// Includes ********************************************************************

#include <G10.CPU.Core.hpp>

// Public Methods **************************************************************

namespace G10::CPU
{
    Core::Core (IBus& pBus) :
        mBus    { pBus }
    {
        mBus.Reset();
    }

    auto Core::Reset () -> void
    {
        mTicksPending = 0;
        mTicksConsumed = 0;
        mRegisters.fill(0);
        mProgramCounter = kInitialProgramCounter;
        mStackPointer = kInitialStackPointer;
        mFlagsRegister.mValue = 0;
        mInterruptEnableRegister = 1;
        mInterruptRequestRegister = 0;
        mSpeedRegister.mValue = 0b01111100;
        mInstructionRegister = 0;
        mInstructionPointer = 0;
        mImmediateData = 0;
        mImmediatePointer = 0;
        mException = Exception::None;
        mExceptionPending = Exception::None;
        mInterruptMasterEnable = false;
        mInterruptMasterPending = false;
        mHaltFlag = false;
        mStopFlag = false;
        mDoubleFaultFlag = false;
    }

    auto Core::Wake () -> void
    {
        if (mDoubleFaultFlag == false)
            { mStopFlag = false; }
    }
}

// Public Methods - Callbacks **************************************************

namespace G10::CPU
{
    auto Core::SetInstructionDecodeDelegate (const InstructionDecodeDelegate& delegate) -> void
    {
        mInstructionDecodeDelegate = delegate;
    }

    auto Core::SetInstructionExecuteCallback (const InstructionExecuteCallback& callback) -> void
    {
        mInstructionExecuteCallback = callback;
    }

    auto Core::SetInterruptServiceCallback (const InterruptServiceCallback& callback) -> void
    {
        mInterruptServiceCallback = callback;
    }

    auto Core::SetStoppedDelegate (const StoppedDelegate& callback) -> void
    {
        mStoppedDelegate = callback;
    }

    auto Core::SetThrottleCallback (const ThrottleCallback& callback) -> void
    {
        mThrottleCallback = callback;
    }
}

// Public Methods - Timing Management ******************************************

namespace G10::CPU
{
    auto Core::AddTickCycles (std::uint64_t pCycles) -> bool
    {
        mTicksPending += pCycles;
        return true;
    }

    auto Core::AddMachineCycles (std::uint64_t pCycles) -> bool
    {
        mTicksPending += (pCycles * kTicksPerMachineCycle);
        return true;
    }

    auto Core::FlushTickCycles () -> bool
    {
        static constexpr std::uint64_t kTickBatchSize = 1024;
        const auto& clockSpeed = (mSpeedRegister.mHigh == true) ? 
            mHighClockSpeed : 
            mClockSpeed;

        bool good = true;
        while (mTicksPending > 0)
        {
            auto batch = std::min(mTicksPending, kTickBatchSize);
            for (std::uint64_t i = 0; i < batch; ++i)
            {
                if (mBus.Clock(mTicksConsumed + i) == false)
                {
                    if (mExceptionPending == Exception::None)
                        { RaiseException(Exception::HardwareError); }

                    good = false;
                }
            }

            mTicksConsumed += batch;
            mTicksPending -= batch;

            if (mThrottleCallback != nullptr)
                { mThrottleCallback(*this, clockSpeed, batch); }
        }

        return good;
    }
}

// Public Methods - Interrupts & Exceptions ************************************

namespace G10::CPU
{
    auto Core::RequestInterrupt (std::uint8_t pLine) -> bool
    {
        if (pLine >= kInterruptCount)
            { return false; }

        mInterruptRequestRegister |= (1 << pLine);
        return true;
    }

    auto Core::RaiseException (Exception pException) -> bool
    {
        if (pException == Exception::None)
            { return true; }

        if (mExceptionPending != Exception::None ||
            pException == Exception::DoubleFault)
        {
            mDoubleFaultFlag = true;
            mStopFlag = true;
            mJustStopped = true;
            if (mStoppedDelegate != nullptr)
            {
                mStoppedDelegate(*this);
            }

            if (mExceptionPending != Exception::None)
            {
                mException = mExceptionPending;
            }
        }

        mExceptionPending = pException;
        return false;
    }

    auto Core::RaiseException (std::uint8_t pException) -> bool
    {
        return RaiseException(static_cast<Exception>(pException));
    }
}

// Public Methods - General-Purpose Registers **********************************

namespace G10::CPU
{
    auto Core::ReadRegisterDW (std::uint8_t pIndex, std::uint32_t& pValueOut) const -> bool
    {
        if (pIndex >= kRegisterCount)
            { return false; }

        pValueOut = mRegisters[pIndex];
        return true;
    }

    auto Core::ReadRegisterW  (std::uint8_t pIndex, std::uint16_t& pValueOut) const -> bool
    {
        if (pIndex >= kRegisterCount)
            { return false; }

        pValueOut = (mRegisters[pIndex] & 0xFFFF);
        return true;
    }

    auto Core::ReadRegisterHB (std::uint8_t pIndex, std::uint8_t&  pValueOut) const -> bool
    {
        if (pIndex >= kRegisterCount)
            { return false; }

        pValueOut = ((mRegisters[pIndex] >> 8) & 0xFF);
        return true;
    }

    auto Core::ReadRegisterLB (std::uint8_t pIndex, std::uint8_t&  pValueOut) const -> bool
    {
        if (pIndex >= kRegisterCount)
            { return false; }

        pValueOut = (mRegisters[pIndex] & 0xFF);
        return true;
    }

    auto Core::WriteRegisterDW (std::uint8_t pIndex, std::uint32_t pValue) -> bool
    {
        if (pIndex >= kRegisterCount)
            { return false; }

        mRegisters[pIndex] = pValue;
        return true;
    }

    auto Core::WriteRegisterW  (std::uint8_t pIndex, std::uint16_t pValue) -> bool
    {
        if (pIndex >= kRegisterCount)
            { return false; }

        mRegisters[pIndex] =
            (mRegisters[pIndex] & 0xFFFF0000) |
            pValue;
        return true;
    }

    auto Core::WriteRegisterHB (std::uint8_t pIndex, std::uint8_t  pValue) -> bool
    {
        if (pIndex >= kRegisterCount)
            { return false; }

        mRegisters[pIndex] =
            (mRegisters[pIndex] & 0xFFFF00FF) |
            (pValue << 8);
        return true;
    }

    auto Core::WriteRegisterLB (std::uint8_t pIndex, std::uint8_t  pValue) -> bool
    {
        if (pIndex >= kRegisterCount)
            { return false; }
            
        mRegisters[pIndex] =
            (mRegisters[pIndex] & 0xFFFFFF00) |
            pValue;
        return true;
    }
}

// Public Methods - Special-Purpose Registers **********************************

namespace G10::CPU
{
    auto Core::ReadProgramCounter (std::uint32_t& pValueOut) const -> bool
    {
        pValueOut = mProgramCounter;
        return true;
    }

    auto Core::ReadStackPointer (std::uint32_t& pValueOut) const -> bool
    {
        pValueOut = stx::clamp(mStackPointer, kStackHardBottom, kStackHardTop);
        return true;
    }

    auto Core::ReadFlagsRegister (std::uint8_t& pValueOut) const -> bool
    {
        pValueOut = mFlagsRegister.mValue;
        return true;
    }

    auto Core::ReadFlag (Flag pFlag, bool& pValueOut) const -> bool
    {
        switch (pFlag)
        {
            case Flag::Overflow:    pValueOut = mFlagsRegister.mOverflow; break;
            case Flag::Carry:       pValueOut = mFlagsRegister.mCarry; break;
            case Flag::HalfCarry:   pValueOut = mFlagsRegister.mHalfCarry; break;
            case Flag::Subtract:    pValueOut = mFlagsRegister.mSubtract; break;
            case Flag::Zero:        pValueOut = mFlagsRegister.mZero; break;
            default:                return false;
        }

        return true;
    }

    auto Core::WriteProgramCounter (std::uint32_t pValue) -> bool
    {
        mProgramCounter = pValue;
        return AddMachineCycles(1);
    }

    auto Core::WriteStackPointer (std::uint32_t pValue) -> bool
    {
        mStackPointer = stx::clamp(pValue, kStackHardBottom, kStackHardTop);
        return AddMachineCycles(1);
    }

    auto Core::WriteFlagsRegister (std::uint8_t pValue) -> bool
    {
        mFlagsRegister.mValue = (pValue & 0b11111000);
        return true;
    }

    auto Core::WriteFlag (Flag pFlag, bool pValue) -> bool
    {
        switch (pFlag)
        {
            case Flag::Overflow:    mFlagsRegister.mOverflow = pValue; break;
            case Flag::Carry:       mFlagsRegister.mCarry = pValue; break;
            case Flag::HalfCarry:   mFlagsRegister.mHalfCarry = pValue; break;
            case Flag::Subtract:    mFlagsRegister.mSubtract = pValue; break;
            case Flag::Zero:        mFlagsRegister.mZero = pValue; break;
            default:                return false;
        }

        return true;
    }
}

// Public Methods - Hardware Registers *****************************************

namespace G10::CPU
{
    auto Core::ReadHardwareRegister (HardwareRegister pRegister, std::uint8_t& pValueOut) -> bool
    {
        switch (pRegister)
        {
            case HardwareRegister::IE0: 
                pValueOut =
                    ((mInterruptEnableRegister >>  0) & 0xFF);
                break;
            case HardwareRegister::IE1:
                pValueOut =
                    ((mInterruptEnableRegister >>  8) & 0xFF);
                break;
            case HardwareRegister::IE2:
                pValueOut =
                    ((mInterruptEnableRegister >> 16) & 0xFF);
                break;
            case HardwareRegister::IE3:
                pValueOut =
                    ((mInterruptEnableRegister >> 24) & 0xFF);
                break;
            case HardwareRegister::IRQ0: 
                pValueOut =
                    ((mInterruptRequestRegister >>  0) & 0xFF); 
                break;
            case HardwareRegister::IRQ1:
                pValueOut =
                    ((mInterruptRequestRegister >>  8) & 0xFF); 
                break;
            case HardwareRegister::IRQ2:
                pValueOut =
                    ((mInterruptRequestRegister >> 16) & 0xFF); 
                break;
            case HardwareRegister::IRQ3:
                pValueOut =
                    ((mInterruptRequestRegister >> 24) & 0xFF); 
                break;
            case HardwareRegister::SPD: 
                pValueOut = 
                    0b01111100 |
                    (mSpeedRegister.mValue & 0b10000011);
                break;
            default: return false;
        }

        return true;
    }

    auto Core::WriteHardwareRegister (HardwareRegister pRegister, std::uint8_t pValue) -> bool
    {
        switch (pRegister)
        {
            case HardwareRegister::IE0: 
                mInterruptEnableRegister =
                    (mInterruptEnableRegister & 0xFFFFFF00) |
                    (pValue <<  0);
                break;
            case HardwareRegister::IE1: 
                mInterruptEnableRegister =
                    (mInterruptEnableRegister & 0xFFFF00FF) |
                    (pValue <<  8);
                break;
            case HardwareRegister::IE2: 
                mInterruptEnableRegister =
                    (mInterruptEnableRegister & 0xFF00FFFF) |
                    (pValue << 16);
                break;
            case HardwareRegister::IE3: 
                mInterruptEnableRegister =
                    (mInterruptEnableRegister & 0x00FFFFFF) |
                    (pValue << 24);
                break;
            case HardwareRegister::IRQ0: 
                mInterruptRequestRegister =
                    (mInterruptRequestRegister & 0xFFFFFF00) |
                    (pValue <<  0);
                break;
            case HardwareRegister::IRQ1: 
                mInterruptRequestRegister =
                    (mInterruptRequestRegister & 0xFFFF00FF) |
                    (pValue <<  8);
                break;
            case HardwareRegister::IRQ2: 
                mInterruptRequestRegister =
                    (mInterruptRequestRegister & 0xFF00FFFF) |
                    (pValue << 16);
                break;
            case HardwareRegister::IRQ3: 
                mInterruptRequestRegister =
                    (mInterruptRequestRegister & 0x00FFFFFF) |
                    (pValue << 24);
                break;
            case HardwareRegister::SPD:
                mSpeedRegister.mValue =
                    (mSpeedRegister.mValue & 0b10000010) |
                    0b01111100 |
                    (pValue & 0b00000001);
            default: return false;
        }

        return true;
    }
}

// Public Methods - Bus Access *************************************************

namespace G10::CPU
{
    auto Core::ReadMemoryDW (std::uint32_t pAddress, std::uint32_t& pValueOut, bool pBigEndian) -> bool
    {
        bool good = true;
        std::uint8_t bytes[4] { 0 };
        for (auto i = 0; i < 4; ++i)
        {
            if (ReadFromBus(pAddress + i, bytes[i]) == false)
                { good = false; }
        }

        if (pBigEndian == true)
        {
            pValueOut =
                (static_cast<std::uint32_t>(bytes[3]) <<  0) |
                (static_cast<std::uint32_t>(bytes[2]) <<  8) |
                (static_cast<std::uint32_t>(bytes[1]) << 16) |
                (static_cast<std::uint32_t>(bytes[0]) << 24);
        }
        else
        {
            pValueOut =
                (static_cast<std::uint32_t>(bytes[0]) <<  0) |
                (static_cast<std::uint32_t>(bytes[1]) <<  8) |
                (static_cast<std::uint32_t>(bytes[2]) << 16) |
                (static_cast<std::uint32_t>(bytes[3]) << 24);
        }

        return good;
    }

    auto Core::ReadMemoryW  (std::uint32_t pAddress, std::uint16_t& pValueOut, bool pBigEndian) -> bool
    {
        bool good = true;
        std::uint8_t bytes[2] { 0 };
        for (auto i = 0; i < 2; ++i)
        {
            if (ReadFromBus(pAddress + i, bytes[i]) == false)
                { good = false; }
        }

        if (pBigEndian == true)
        {   
            pValueOut =
                (static_cast<std::uint16_t>(bytes[1]) <<  0) |
                (static_cast<std::uint16_t>(bytes[0]) <<  8);
        }
        else
        {
            pValueOut =
                (static_cast<std::uint16_t>(bytes[0]) <<  0) |
                (static_cast<std::uint16_t>(bytes[1]) <<  8);
        }

        return good;
    }

    auto Core::ReadMemoryB  (std::uint32_t pAddress, std::uint8_t&  pValueOut) -> bool
    {
        return ReadFromBus(pAddress, pValueOut);
    }

    auto Core::WriteMemoryDW (std::uint32_t pAddress, std::uint32_t pValue, bool pBigEndian) -> bool
    {
        bool good = true;
        std::uint8_t bytes[4] = { 0 };
        if (pBigEndian == true)
        {
            bytes[3] = static_cast<std::uint8_t>((pValue >>  0) & 0xFF);
            bytes[2] = static_cast<std::uint8_t>((pValue >>  8) & 0xFF);
            bytes[1] = static_cast<std::uint8_t>((pValue >> 16) & 0xFF);
            bytes[0] = static_cast<std::uint8_t>((pValue >> 24) & 0xFF);
        }
        else
        {
            bytes[0] = static_cast<std::uint8_t>((pValue >>  0) & 0xFF);
            bytes[1] = static_cast<std::uint8_t>((pValue >>  8) & 0xFF);
            bytes[2] = static_cast<std::uint8_t>((pValue >> 16) & 0xFF);
            bytes[3] = static_cast<std::uint8_t>((pValue >> 24) & 0xFF);
        }

        for (auto i = 0; i < 4; ++i)
        {
            if (WriteToBus(pAddress + i, bytes[i]) == false)
                { good = false; }
        }

        return true;
    }

    auto Core::WriteMemoryW  (std::uint32_t pAddress, std::uint16_t pValue, bool pBigEndian) -> bool
    {
        bool good = true;
        std::uint8_t bytes[2] = { 0 };
        if (pBigEndian == true)
        {
            bytes[1] = static_cast<std::uint8_t>((pValue >>  0) & 0xFF);
            bytes[0] = static_cast<std::uint8_t>((pValue >>  8) & 0xFF);
        }
        else
        {
            bytes[0] = static_cast<std::uint8_t>((pValue >>  0) & 0xFF);
            bytes[1] = static_cast<std::uint8_t>((pValue >>  8) & 0xFF);
        }

        for (auto i = 0; i < 2; ++i)
        {
            if (WriteToBus(pAddress + i, bytes[i]) == false)
                { good = false; }
        }

        return true;
    }

    auto Core::WriteMemoryB  (std::uint32_t pAddress, std::uint8_t  pValue) -> bool
    {
        return WriteToBus(pAddress, pValue);
    }

    auto Core::FetchMemoryDW (std::uint32_t& pValueOut, bool pBigEndian) -> bool
    {
        mImmediatePointer = mProgramCounter;

        bool good = ReadMemoryDW(mProgramCounter, pValueOut, pBigEndian);
        if (good == true) { mImmediateData = pValueOut; }
        else { mImmediateData = pValueOut = 0xFFFFFFFF; }

        mProgramCounter += 4;
        return good;
    }

    auto Core::FetchMemoryW  (std::uint16_t& pValueOut, bool pBigEndian) -> bool
    {
        mImmediatePointer = mProgramCounter;

        bool good = ReadMemoryW(mProgramCounter, pValueOut, pBigEndian);
        if (good == true) { mImmediateData = pValueOut; }
        else
        {
            mImmediateData = 0xFFFFFFFF;
            pValueOut = 0xFFFF;
        }

        mProgramCounter += 2;
        return good;
    }

    auto Core::FetchMemoryB  (std::uint8_t&  pValueOut) -> bool
    {
        mImmediatePointer = mProgramCounter;

        bool good = ReadMemoryB(mProgramCounter, pValueOut);
        if (good == true) { mImmediateData = pValueOut; }
        else
        {
            mImmediateData = 0xFFFFFFFF;
            pValueOut = 0xFF;
        }

        mProgramCounter++;
        return good;
    }

    auto Core::PopStackDW (std::uint32_t& pValueOut, bool pBigEndian) -> bool
    {
        if (mStackPointer + 4 > kStackHardTop)
            { return RaiseException(Exception::StackUnderflow); }

        bool good = ReadMemoryDW(mStackPointer, pValueOut, pBigEndian);
        if (good == false)
        {
            if (mExceptionPending == Exception::InvalidReadAccess)
                { mExceptionPending = Exception::StackUnderflow; }

            return false;
        }
        
        mStackPointer += 4;
        return AddMachineCycles(1);
    }

    auto Core::PushStackDW (std::uint32_t& pValue, bool pBigEndian) -> bool
    {
        if (mStackPointer - 4 < kStackHardBottom)
            { return RaiseException(Exception::StackOverflow); }

        bool good = WriteMemoryDW(mStackPointer - 4, pValue, pBigEndian);
        if (good == false)
        {
            if (mExceptionPending == Exception::InvalidWriteAccess)
                { mExceptionPending = Exception::StackOverflow; }

            return false;
        }

        mStackPointer -= 4;
        return AddMachineCycles(1);
    }
}

// Private Methods - Bus Access Timing *****************************************

namespace G10::CPU
{
    auto Core::ReadFromBus (std::uint32_t pAddress, std::uint8_t& pValueOut) -> bool
    {
        if (FlushTickCycles() == false)
        {
            pValueOut = 0xFF;
            return false;
        }

        bool good = mBus.Read(pAddress, pValueOut);
        if (good == false)
        {
            if (mExceptionPending == Exception::None)
                { RaiseException(Exception::HardwareError); }

            pValueOut = 0xFF;
            return false;
        }

        return AddMachineCycles(1) && (good == true);
    }

    auto Core::WriteToBus (std::uint32_t pAddress, std::uint8_t pValue) -> bool
    {
        if (FlushTickCycles() == false)
        {
            return false;
        }

        bool good = mBus.Write(pAddress, pValue);
        if (good == false)
        {
            if (mExceptionPending == Exception::None)
                { RaiseException(Exception::HardwareError); }

            return false;
        }

        return AddMachineCycles(1) && (good == true);
    }
}
