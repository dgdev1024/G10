/**
 * @file    G10.GB.Realtime.cpp
 * @brief   Contains implementations for the G10.Boy's real-time clock
 *          component, and related definitions.
 */

// Includes ********************************************************************

#include <G10.GB.System.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::GB
{
    Realtime::Realtime (System& pSystem) :
        mSystem { pSystem }
    {
    }
}

// Public Methods **************************************************************

namespace G10::GB
{
    auto Realtime::Reset () -> void
    {
        mSeconds = 0;
        mMinutes = 0;
        mHours = 0;
        mDays = 0;
        mLatchedSeconds = 0;
        mLatchedMinutes = 0;
        mLatchedHours = 0;
        mLatchedDays = 0;
        mControl.mValue = 0;
        UpdateTimeBuffer();
    }

    auto Realtime::Clock (const std::uint64_t& pCycle) -> bool
    {

        if ((pCycle & 0x3FFFFF) != 0x3FFFFF)
            { return true; }

        auto oldTime = mUpdateTime;
        UpdateTimeBuffer();

        if (mControl.mIsHalted == true || mControl.mLatchToSystem == true)
            { return true; }

        // The RTC is clocked every half second, and each clock updates the
        // non-latched time registers accordingly.
        if (mUpdateTime != oldTime)
        {
            auto elapsed = mUpdateTime - oldTime;
            while (elapsed > 0)
            {
                if (++mSeconds >= 60)
                {
                    mSeconds = 0;
                    if (++mMinutes >= 60)
                    {
                        mMinutes = 0;
                        if (++mHours >= 24)
                        {
                            mHours = 0;
                            ++mDays;
                        }
                    }
                }

                elapsed--;
            }

            mSystem.GetCPU().RequestInterrupt(stx::under(Interrupt::Realtime));
        }

        return true;
    }
}

// Public Methods - Port Registers *********************************************

namespace G10::GB
{
    auto Realtime::ReadRTCS (std::uint8_t& pDataOut) -> bool
    {
        pDataOut = mLatchedSeconds;
        return true;
    }

    auto Realtime::ReadRTCM (std::uint8_t& pDataOut) -> bool
    {
        pDataOut = mLatchedMinutes;
        return true;
    }

    auto Realtime::ReadRTCH (std::uint8_t& pDataOut) -> bool
    {
        pDataOut = mLatchedHours;
        return true;
    }

    auto Realtime::ReadRTCDL (std::uint8_t& pDataOut) -> bool
    {
        pDataOut = static_cast<std::uint8_t>(mLatchedDays & 0xFF);
        return true;
    }

    auto Realtime::ReadRTCDH (std::uint8_t& pDataOut) -> bool
    {
        pDataOut = static_cast<std::uint8_t>((mLatchedDays >> 8) & 0xFF);
        return true;
    }

    auto Realtime::ReadRTCC (std::uint8_t& pDataOut) -> bool
    {
        pDataOut = 0b10011111 | (mControl.mValue & 0b01100000);
        return true;
    }

    auto Realtime::WriteRTCS (std::uint8_t pDataIn) -> bool
    {
        if (mControl.mIsHalted == true)
            { mSeconds = pDataIn; }

        return true;
    }

    auto Realtime::WriteRTCM (std::uint8_t pDataIn) -> bool
    {
        if (mControl.mIsHalted == true)
            { mMinutes = pDataIn; }

        return true;
    }

    auto Realtime::WriteRTCH (std::uint8_t pDataIn) -> bool
    {
        if (mControl.mIsHalted == true)
            { mHours = pDataIn; }

        return true;
    }

    auto Realtime::WriteRTCDL (std::uint8_t pDataIn) -> bool
    {
        if (mControl.mIsHalted == true)
            { mDays = (mDays & 0xFF00) | pDataIn; }

        return true;
    }

    auto Realtime::WriteRTCDH (std::uint8_t pDataIn) -> bool
    {
        if (mControl.mIsHalted == true)
            { mDays = (mDays & 0x00FF) | (pDataIn << 8); }

        return true;
    }

    auto Realtime::WriteRTCC (std::uint8_t pDataIn) -> bool
    {
        mControl.mValue = 0b10011111 | (pDataIn & 0b01100000);
        return true;
    }

    auto Realtime::WriteRTCL () -> bool
    {
        if (mControl.mLatchToSystem == true)
        {
            // Latch our RTC registers according to the host system's current
            // time.
            auto now = std::chrono::system_clock::now();
            auto nowTimeT = std::chrono::system_clock::to_time_t(now);
            std::tm timeBuffer {};
            localtime_r(&nowTimeT, &timeBuffer);

            mLatchedSeconds = timeBuffer.tm_sec;
            mLatchedMinutes = timeBuffer.tm_min;
            mLatchedHours = timeBuffer.tm_hour;
            mLatchedDays = 
                ((timeBuffer.tm_year - 1) * 365) + 
                ((timeBuffer.tm_year - 1) / 4) + 
                timeBuffer.tm_yday;
        }
        else
        {
            mLatchedSeconds = mSeconds;
            mLatchedMinutes = mMinutes;
            mLatchedHours = mHours;
            mLatchedDays = mDays;
        }

        return true;
    }
}

// Private Methods *************************************************************

namespace G10::GB
{
    auto Realtime::UpdateTimeBuffer () -> void
    {
        auto now = std::chrono::system_clock::now();
        auto nowTimeT = std::chrono::system_clock::to_time_t(now);
        mUpdateTime = nowTimeT;
        localtime_r(&nowTimeT, &mUpdateTimeBuffer);
    }
}