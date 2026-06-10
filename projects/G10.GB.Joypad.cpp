/**
 * @file    G10.GB.Joypad.cpp
 * @brief   Contains implementations for the G10.Boy's joypad input component,
 *          and related definitions.
 */

// Includes ********************************************************************

#include <G10.GB.System.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::GB
{
    Joypad::Joypad (System& pSystem) :
        mSystem     { pSystem }
    {
    }
}

// Public Methods **************************************************************

namespace G10::GB
{
    auto Joypad::Reset () -> void
    {
        mState = {};
    }
}

// Public Methods - Callbacks **************************************************

namespace G10::GB
{
    auto Joypad::SetButtonCallback (const ButtonCallback& pCallback) -> void
    {
        mButtonCallback = pCallback;
    }
}

// Public Methods - Port Registers *********************************************

namespace G10::GB
{
    auto Joypad::ReadJOYP (std::uint8_t& pDataOut) -> bool
    {
        // Before Read
        // - Prepare read value from current joypad state.
        std::uint8_t out = 0xFF;
        if (mState.mButtonsSelected == true)
        {
            out &= ~(1 << 5);
            out &= ~(mState.mStart << 3);
            out &= ~(mState.mSelect << 2);
            out &= ~(mState.mB << 1);
            out &= ~(mState.mA << 0);
        }

        if (mState.mDirectionsSelected == true)
        {
            out &= ~(1 << 4);
            out &= ~(mState.mDown << 3);
            out &= ~(mState.mUp << 2);
            out &= ~(mState.mLeft << 1);
            out &= ~(mState.mRight << 0);
        }

        // Read
        // - Bits 6 and 7 are unused; read `1`.
        // - Bits 0 through 5 are readable.
        pDataOut = out;
        return true;
    }

    auto Joypad::WriteJOYP (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - Write button select state to bit 5.
        // - Write direction select state to bit 4.
        // - Note: Here, clear is true and set is false.
        mState.mButtonsSelected = (pDataIn & (1 << 5)) == 0;
        mState.mDirectionsSelected = (pDataIn & (1 << 4)) == 0;
        return true;
    }
}

// Public Methods - State ******************************************************

namespace G10::GB
{
    auto Joypad::PressButton (JoypadButton pButton) -> bool
    {
        bool
            wasReleased = false,
            nowPressed = false,
            groupSelected = false;
        switch (pButton)
        {
            case JoypadButton::A:
                wasReleased = (mState.mA == false);
                nowPressed = true;
                groupSelected = mState.mButtonsSelected;
                break;
            case JoypadButton::B:
                wasReleased = (mState.mB == false);
                nowPressed = true;
                groupSelected = mState.mButtonsSelected;
                break;
            case JoypadButton::Select:
                wasReleased = (mState.mSelect == false);
                nowPressed = true;
                groupSelected = mState.mButtonsSelected;
                break;
            case JoypadButton::Start:
                wasReleased = (mState.mStart == false);
                nowPressed = true;
                groupSelected = mState.mButtonsSelected;
                break;
            case JoypadButton::Up:
                wasReleased = (mState.mUp == false);
                nowPressed = true;
                groupSelected = mState.mDirectionsSelected;
                break;
            case JoypadButton::Down:
                wasReleased = (mState.mDown == false);
                nowPressed = true;
                groupSelected = mState.mDirectionsSelected;
                break;
            case JoypadButton::Left:
                wasReleased = (mState.mLeft == false);
                nowPressed = true;
                groupSelected = mState.mDirectionsSelected;
                break;
            case JoypadButton::Right:
                wasReleased = (mState.mRight == false);
                nowPressed = true;
                groupSelected = mState.mDirectionsSelected;
                break;
        }

        if (wasReleased == true && nowPressed == true)
        {
            if (groupSelected == true)
            {
                auto& cpu = mSystem.GetCPU();
                cpu.RequestInterrupt(stx::under(Interrupt::Joypad));
                cpu.Wake();
            }

            if (mButtonCallback != nullptr)
                { mButtonCallback(mSystem, pButton, true); }
        }

        return true;
    }

    auto Joypad::ReleaseButton (JoypadButton pButton) -> bool
    {
        bool
            wasPressed = false,
            nowReleased = false;
        switch (pButton)
        {
            case JoypadButton::A:
                wasPressed = (mState.mA == true);
                nowReleased = true;
                break;
            case JoypadButton::B:
                wasPressed = (mState.mB == true);
                nowReleased = true;
                break;
            case JoypadButton::Select:
                wasPressed = (mState.mSelect == true);
                nowReleased = true;
                break;
            case JoypadButton::Start:
                wasPressed = (mState.mStart == true);
                nowReleased = true;
                break;
            case JoypadButton::Up:
                wasPressed = (mState.mUp == true);
                nowReleased = true;
                break;
            case JoypadButton::Down:
                wasPressed = (mState.mDown == true);
                nowReleased = true;
                break;
            case JoypadButton::Left:
                wasPressed = (mState.mLeft == true);
                nowReleased = true;
                break;
            case JoypadButton::Right:
                wasPressed = (mState.mRight == true);
                nowReleased = true;
                break;
        }

        if (wasPressed == true && nowReleased == true)
        {
            if (mButtonCallback != nullptr)
                { mButtonCallback(mSystem, pButton, false); }
        }

        return true;
    }
}
