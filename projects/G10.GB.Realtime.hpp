/**
 * @file    G10.GB.Realtime.hpp
 * @brief   Contains declarations for the G10.Boy's real-time clock
 *          component, and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.GB.Common.hpp>

// Types ***********************************************************************

namespace G10::GB
{
    class System;
}

// Unions & Structures *********************************************************

namespace G10::GB
{
    union RealtimeControl final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t                    : 5;
            std::uint8_t    mLatchToSystem  : 1;
            std::uint8_t    mIsHalted       : 1;
            std::uint8_t                    : 1;
        };
    };
}

// Classes *********************************************************************

namespace G10::GB
{
    class G10_API Realtime final
    {
    public: // Constructors & Destructor ***************************************

        explicit Realtime (System& pSystem);

    public: // Methods *********************************************************

        auto Reset () -> void;
        auto Clock (const std::uint64_t& pCycle) -> bool;

    public: // Methods - Port Registers ****************************************

        auto ReadRTCS (std::uint8_t& pDataOut) -> bool;
        auto ReadRTCM (std::uint8_t& pDataOut) -> bool;
        auto ReadRTCH (std::uint8_t& pDataOut) -> bool;
        auto ReadRTCDL (std::uint8_t& pDataOut) -> bool;
        auto ReadRTCDH (std::uint8_t& pDataOut) -> bool;
        auto ReadRTCC (std::uint8_t& pDataOut) -> bool;

        auto WriteRTCS (std::uint8_t pDataIn) -> bool;
        auto WriteRTCM (std::uint8_t pDataIn) -> bool;
        auto WriteRTCH (std::uint8_t pDataIn) -> bool;
        auto WriteRTCDL (std::uint8_t pDataIn) -> bool;
        auto WriteRTCDH (std::uint8_t pDataIn) -> bool;
        auto WriteRTCC (std::uint8_t pDataIn) -> bool;
        auto WriteRTCL () -> bool;

    private: // Methods ********************************************************

        auto UpdateTimeBuffer () -> void;

    private: // Members ********************************************************

        System&             mSystem;

        // Port Registers
        std::uint8_t        mSeconds { 0 };
        std::uint8_t        mMinutes { 0 };
        std::uint8_t        mHours { 0 };
        std::uint16_t       mDays { 0 };
        RealtimeControl     mControl;

        // Internal State
        std::uint8_t        mLatchedSeconds { 0 };
        std::uint8_t        mLatchedMinutes { 0 };
        std::uint8_t        mLatchedHours { 0 };
        std::uint16_t       mLatchedDays { 0 };
        std::time_t         mUpdateTime { 0 };
        std::tm             mUpdateTimeBuffer {};

    };
}