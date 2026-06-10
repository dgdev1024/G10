/**
 * @file    G10.GB.Joypad.hpp
 * @brief   Contains declarations for the G10.Boy's joypad input component, 
 *          and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.GB.Common.hpp>

// Constants & Enumeartions ****************************************************

namespace G10::GB
{
    enum class JoypadButton : std::uint8_t
    {
        Right,
        Left,
        Up,
        Down,
        A,
        B,
        Select,
        Start
    };
}

// Types ***********************************************************************

namespace G10::GB
{
    class System;

    using ButtonCallback = std::function<void (const System&, JoypadButton, bool)>;
}

// Unions & Structures *********************************************************

namespace G10::GB
{
    struct JoypadState final
    {
        bool mRight { false };
        bool mLeft { false };
        bool mUp { false };
        bool mDown { false };
        bool mA { false };
        bool mB { false };
        bool mSelect { false };
        bool mStart { false };
        bool mButtonsSelected { false };
        bool mDirectionsSelected { false };
    };
}

// Classes *********************************************************************

namespace G10::GB
{
    class G10_API Joypad final
    {
    public: // Constructors & Destructor ***************************************

        explicit Joypad (System& pSystem);

    public: // Methods *********************************************************

        auto Reset () -> void;

    public: // Methods - Callbacks *********************************************

        auto SetButtonCallback (const ButtonCallback& pCallback) -> void;

    public: // Methods - Port Registers ****************************************

        auto ReadJOYP (std::uint8_t& pDataOut) -> bool;
        auto WriteJOYP (std::uint8_t pDataIn) -> bool;

    public: // Methods - State *************************************************

        auto PressButton (JoypadButton pButton) -> bool;
        auto ReleaseButton (JoypadButton pButton) -> bool;

    private: // Members ********************************************************

        System& mSystem;

        // Callbacks
        ButtonCallback mButtonCallback { nullptr };

        // Intenral State
        JoypadState mState {};

    };
}
