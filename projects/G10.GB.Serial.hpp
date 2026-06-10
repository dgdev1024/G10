/**
 * @file    G10.GB.Serial.hpp
 * @brief   Contains declarations for the G10.Boy's serial communication
 *          component, and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.GB.Common.hpp>

// Types ***********************************************************************

namespace G10::GB
{
    class System;

    using SerialProgressDelegate = std::function<bool (System&, std::uint8_t, std::uint8_t&)>;
    using SerialFinishCallback = std::function<void (const System&, std::uint8_t, std::uint8_t)>;

    // using SerialProgressDelegate = std::function<bool(System&,
    //     std::uint8_t /* pSentBit */, std::uint8_t& /* pReceivedBit */)>;
    // using SerialFinishCallback = std::function<void(const System&,
    //     std::uint8_t /* pSentByte */, std::uint8_t /* pReceivedByte */)>;    
}

// Constants & Enumeartions ****************************************************

namespace G10::GB
{

}

// Unions & Structures *********************************************************

namespace G10::GB
{
    union SerialControlRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mInternalClock  : 1;
            std::uint8_t    mHighSpeed      : 1;
            std::uint8_t                    : 5;
            std::uint8_t    mTransferEnable : 1;
        };
    };
}

// Classes *********************************************************************

namespace G10::GB
{
    class G10_API Serial final
    {   
    public: // Constructors & Destructor ***************************************

        explicit Serial (System& pSystem);

    public: // Methods *********************************************************

        auto Reset () -> void;
        auto Clock (const std::uint64_t& pCycle) -> bool;

    public: // Methods - Callbacks *********************************************

        auto SetProgressDelegate (const SerialProgressDelegate& pDelegate) -> void;
        auto SetFinishCallback (const SerialFinishCallback& pCallback) -> void;

    public: // Methods - Port Registers ****************************************

        auto ReadSB (std::uint8_t& pDataOut) -> bool;
        auto ReadSC (std::uint8_t& pDataOut) -> bool;

        auto WriteSB (std::uint8_t pDataIn) -> bool;
        auto WriteSC (std::uint8_t pDataIn) -> bool;

    public: // Methods - State *************************************************

        inline auto GetNextBit () const -> std::uint8_t
            { return (mByte >> 7) & 0b1; }

    private: // Members ********************************************************

        System& mSystem;

        // Callbacks
        SerialProgressDelegate mProgressDelegate;
        SerialFinishCallback mFinishCallback;

        // Port Registers
        std::uint8_t            mByte { 0x00 };
        SerialControlRegister   mControl { .mValue = 0x00 };

        // Internal State
        std::uint16_t   mDotCounter { 0 };
        std::uint8_t    mTransferByte { 0 };
        std::uint8_t    mBitsTransferred { 0 };
        bool            mTransferActive { false };
        bool            mIsFirstTransfer { false };

    };
}
