/**
 * @file    G10.GB.Timer.hpp
 * @brief   Contains declarations for the G10.Boy's timer component, 
 *          and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.GB.Common.hpp>

// Types ***********************************************************************

namespace G10::GB
{
    class System;

    using TimerOverflowDelegate = std::function<void (System&, std::uint8_t&)>;
    // using TimerOverflowDelegate = std::function<void(System&, std::uint8_t&)>;
}

// Constants & Enumeartions ****************************************************

namespace G10::GB
{
    enum class TimerClockSelect : std::uint8_t
    {
        Select4096Hz        = 0b00,
        Select262144Hz      = 0b01,
        Select65536Hz       = 0b10,
        Select16384Hz       = 0b11
    };
}

// Unions & Structures *********************************************************

namespace G10::GB
{
    union TimerControlRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mClockSelect    : 2;
            std::uint8_t    mEnable         : 1;
            std::uint8_t                    : 5;
        };
    };
}

// Classes *********************************************************************

namespace G10::GB
{
    class G10_API Timer final
    {   
        friend class System;

    public: // Constructors & Destructor ***************************************

        explicit Timer (System& pSystem);

    public: // Methods *********************************************************

        auto Reset () -> void;
        auto Clock (const std::uint64_t& pCycle) -> bool;

    public: // Methods - Callbacks *********************************************

        auto SetOverflowDelegate (const TimerOverflowDelegate& pDelegate) -> void;

    public: // Methods - Port Registers ****************************************

        auto ReadDIV (std::uint8_t& pDataOut) -> bool;
        auto ReadTIMA (std::uint8_t& pDataOut) -> bool;
        auto ReadTMA (std::uint8_t& pDataOut) -> bool;
        auto ReadTAC (std::uint8_t& pDataOut) -> bool;

        auto WriteDIV () -> bool;
        auto WriteTIMA (std::uint8_t pDataIn) -> bool;
        auto WriteTMA (std::uint8_t pDataIn) -> bool;
        auto WriteTAC (std::uint8_t pDataIn) -> bool;

    public: // Methods - State *************************************************

        auto CheckSystemCounterBit (std::uint8_t pBitIndex, 
            bool pCheckHighSpeed = false) -> bool;
        auto CheckSystemCounterFallingEdge (std::uint8_t pBitIndex,
            bool pCheckHighSpeed = false) -> bool;

    public: // Methods - Accessors *********************************************

        inline auto GetSystemCounter () const -> std::uint16_t
            { return mSystemCounter; }

    private: // Methods ********************************************************

        inline constexpr auto GetClockSelectBit (TimerClockSelect pClockSelect) 
            const -> std::uint8_t
        {
            switch (pClockSelect)
            {
                case TimerClockSelect::Select4096Hz:    return 9;
                case TimerClockSelect::Select262144Hz:  return 3;
                case TimerClockSelect::Select65536Hz:   return 5;
                case TimerClockSelect::Select16384Hz:   return 7;
                default:                                return 9;
            }
        }

        inline constexpr auto GetClockSelectBit (std::uint8_t pClockSelect)
            const -> std::uint8_t
        {
            return GetClockSelectBit(static_cast<TimerClockSelect>(pClockSelect));
        }

        inline constexpr auto GetCurrentClockSelectBit () const -> std::uint8_t
            { return GetClockSelectBit(static_cast<TimerClockSelect>(
                mControl.mClockSelect)); }

        auto IncrementTimerCounter () -> void;
        auto SetSystemCounter (std::uint16_t pCounter) -> void;
        auto NotifyStopEvent () -> void;

    private: // Constants & Enumerations ***************************************

        enum class ReloadState
        {
            Normal,
            Overflow,
            Reload
        };

    private: // Members ********************************************************

        System& mSystem;

        // Callbacks
        TimerOverflowDelegate mOverflowDelegate { nullptr };

        // Port Registers
        std::uint16_t           mSystemCounter { 0 };
        std::uint8_t            mTimerCounter { 0 };
        std::uint8_t            mModulo { 0 };
        TimerControlRegister    mControl { .mValue = 0b11111000 };

        // Internal State
        ReloadState             mReloadState { ReloadState::Normal };
        std::uint8_t            mReloadDelay { 0 };
        std::uint16_t           mOldSystemCounter { 0 };

    };
}
