/**
 * @file    G10.GB.Timer.cpp
 * @brief   Contains implementations for the G10.Boy's timer component,
 *          and related definitions.
 */

// Includes ********************************************************************

#include <G10.GB.System.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::GB
{
    Timer::Timer (System& pSystem) :
        mSystem     { pSystem }
    {
    }
}

// Public Methods **************************************************************

namespace G10::GB
{
    auto Timer::Reset () -> void
    {
        // Hardware Registers
        mTimerCounter = 0x00;
        mModulo = 0x00;
        mControl.mValue = 0xF8;
        if (mSystem.IsCGB() == false)
        {
            mSystemCounter = 0xAB00;
        }
        else
        {
            mSystemCounter = 0x0000;
        }

        // Internal State
        mReloadState = ReloadState::Normal;
        mReloadDelay = 0;
        mOldSystemCounter = mSystemCounter;
    }

    auto Timer::Clock (const std::uint64_t& pCycle) -> bool
    {
        // Component Clock Rate: 1 Dot per 2 CPU Cycles
        // if ((pCycle & 1) != 1)
        //     { return true; }

        // Check to see if the system's CPU is in a `STOP` state, or in the
        // middle of a speed switch.
        // - If so, then don't increment the system counter.
        auto& cpu = mSystem.GetCPU();
        if (cpu.IsStopped() == true || cpu.IsSwitchingSpeed() == true)
            { return true; }

        // Advance the `TIMA` reload state machine.
        // - Each state persists for four dots.
        if (mReloadState != ReloadState::Normal)
        {
            if (--mReloadDelay == 0)
            {
                if (mReloadState == ReloadState::Overflow)
                {
                    if (mOverflowDelegate != nullptr)
                    {
                        std::uint8_t modulo = mModulo;
                        mOverflowDelegate(mSystem, modulo);
                        mTimerCounter = modulo;
                    }

                    mReloadState = ReloadState::Reload;
                    mReloadDelay = 4;
                    cpu.RequestInterrupt(stx::under(Interrupt::Timer));
                }
                else
                    { mReloadState = ReloadState::Normal; }
            }
        }

        // Advance the system counter.
        SetSystemCounter(mSystemCounter + 1);
        return true;
    }
}

// Public Methods - Callbacks **************************************************

namespace G10::GB
{
    auto Timer::SetOverflowDelegate (const TimerOverflowDelegate& pDelegate) -> void
        { mOverflowDelegate = pDelegate; }
}

// Public Methods - Port Registers *********************************************

namespace G10::GB
{
    auto Timer::ReadDIV (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = static_cast<std::uint8_t>(mSystemCounter >> 8);
        return true;
    }

    auto Timer::ReadTIMA (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - If `TIMA` reload is pending, read `0`.
        // - Otherwise, all bits readable.
        if (mReloadState == ReloadState::Overflow)
            { pDataOut = 0x00; }
        else
            { pDataOut = mTimerCounter; }

        return true;
    }

    auto Timer::ReadTMA (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mModulo;
        return true;
    }

    auto Timer::ReadTAC (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - Bits 3 through 7 are unused; read `1`.
        // - Bits 0 through 2 are readable.
        pDataOut = (mControl.mValue & 0b00000111) | 0b11111000;
        return true;
    }

    auto Timer::WriteDIV () -> bool
    {
        // Write
        // - Any write to `DIV` resets the system counter to zero.
        SetSystemCounter(0x0000);
        return true;
    }

    auto Timer::WriteTIMA (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - If `TIMA` was just reloaded, then write is ignored.
        // - Otherwise, all bits writable.
        if (mReloadState != ReloadState::Reload)
            { mTimerCounter = pDataIn; }

        return true;
    }

    auto Timer::WriteTMA (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - All bits writable.
        // - If `TIMA` was just reloaded, or a reload is pending, then `TIMA`
        //   is also written to.
        mModulo = pDataIn;
        if (mReloadState != ReloadState::Normal)
            { mTimerCounter = pDataIn; }

        return true;
    }

    auto Timer::WriteTAC (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - If both `TAC.2` (enable bit) and the selected clock bit (pointed
        //   to by `TAC.0-1`) are set, then writing to `TAC` may trigger a
        //   spurious increment of `TIMA` if the write disables the timer or
        //   selects a clock bit which is currently low (triggering a falling
        //   edge on the clock signal).
        TimerControlRegister
            oldControl = mControl,
            newControl = { 
                .mValue = 
                    static_cast<std::uint8_t>((pDataIn & 0b00000111) | 0b11111000) 
            };
        if (oldControl.mEnable == true)
        {
            std::uint8_t oldBitIndex = GetClockSelectBit(oldControl.mClockSelect);
            if (CheckSystemCounterBit(oldBitIndex, true) == true)
            {
                bool newBitHigh = false;
                if (newControl.mEnable == true)
                {
                    std::uint8_t newBitIndex = 
                        GetClockSelectBit(newControl.mClockSelect);
                    newBitHigh = CheckSystemCounterBit(newBitIndex, true);
                }

                if (newBitHigh == true)
                    { IncrementTimerCounter(); }
            }
        }

        // Write
        // - Bits 3 through 7 are unused; write `1`.
        // - Bits 0 through 2 are writable.
        mControl.mValue = (pDataIn & 0b00000111) | 0b11111000;
        return true;
    }
}

// Public Methods - State ******************************************************

namespace G10::GB
{
    auto Timer::CheckSystemCounterBit (std::uint8_t pBitIndex, 
        bool pCheckHighSpeed) -> bool
    {
        if (pBitIndex < 15 &&
            pCheckHighSpeed == true && 
            mSystem.GetCPU().IsHighSpeed() == true)
            { pBitIndex++; }
        else if (pBitIndex > 15)
            { pBitIndex = GetCurrentClockSelectBit(); }

        return (mSystemCounter & (1 << pBitIndex)) != 0;
    }

    auto Timer::CheckSystemCounterFallingEdge (std::uint8_t pBitIndex, 
        bool pCheckHighSpeed) -> bool
    {
        if (pBitIndex < 15 &&
            pCheckHighSpeed == true && 
            mSystem.GetCPU().IsHighSpeed() == true)
            { pBitIndex++; }
        else if (pBitIndex > 15)
            { pBitIndex = GetCurrentClockSelectBit(); }

        return
            (mOldSystemCounter & (1 << pBitIndex)) != 0 &&
            (mSystemCounter & (1 << pBitIndex)) == 0;
    }
}

// Private Methods *************************************************************

namespace G10::GB
{
    auto Timer::IncrementTimerCounter () -> void
    {
        if (++mTimerCounter == 0x00)
        {
            mTimerCounter = mModulo;
            mReloadState = ReloadState::Overflow;
            mReloadDelay = 4;
        }
    }

    auto Timer::SetSystemCounter (std::uint16_t pCounter) -> void
    {
        // Save the old value for APU falling edge detection.
        mOldSystemCounter = mSystemCounter;

        // Detect all falling edges that will occur by setting the new counter value.
        // - If one of those falling edges is the clock select bit, and the timer
        //   is currently enabled, then increment `TIMA`.
        std::uint16_t fallingEdges = (mSystemCounter & ~pCounter);
        if (mControl.mEnable == true &&
            (fallingEdges & (1 << GetCurrentClockSelectBit())) != 0)
            { IncrementTimerCounter(); }

        // Set the new system counter value.
        mSystemCounter = pCounter;

        // If a falling edge occurs on the `APU-DIV` bit (bit 12 in normal speed
        // mode, bit 13 in high speed mode), notify the APU.
        if (CheckSystemCounterFallingEdge(12, true) == true)
            { mSystem.GetAPU().ClockFrameSequencer(); }
    }

    auto Timer::NotifyStopEvent () -> void
    {
        SetSystemCounter(0x0000);
    }
}
