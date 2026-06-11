/**
 * @file    G10.Boy.Application.hpp
 * @brief   Contains declarations for the G10.Boy emulator's application
 *          class, and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <atomic>
#include <SDL3/SDL.h>
#include <G10.GB.System.hpp>

// Types ***********************************************************************

struct ImFont;

// Constants & Enumerations ****************************************************

namespace G10::Boy
{
    constexpr std::int32_t
        kApplicationWindowWidth     = 1280,
        kApplicationWindowHeight    = 720;
    constexpr std::string_view
        kApplicationWindowTitle     = "G10.Boy - G10-based Game Boy Emulator";
    constexpr double
        kTargetFramerate            = 60.0;
    constexpr std::uint64_t
        kFrameDelayNS               = 
            static_cast<std::uint64_t>(1000000000.0 / kTargetFramerate);
    constexpr std::uint32_t
        kSampleRate                 = 44100,
        kSampleCount                = 8192,
        kRingCapacity               = kSampleCount * 2;
}

// Classes *********************************************************************

namespace G10::Boy
{
    class G10_API Application final
    {
    public: // Friends *********************************************************

        friend auto OnAudio (void*, SDL_AudioStream*, int, int) -> void;

    public: // Constructors & Destructor ***************************************

        explicit Application (int argc, const char** argv);
        ~Application ();

    public: // Methods *********************************************************

        auto Start () -> std::int32_t;

    private: // Methods - Initialization & Shutdown ****************************

        auto ParseArguments (int argc, const char** argv) -> void;
        auto InitializeSDL () -> void;
        auto InitializeImGui () -> void;
        auto InitializeApplication () -> void;
        auto ShutdownSDL () -> void;
        auto ShutdownImGui () -> void;

    private: // Methods - Lifecycle ********************************************
    
        auto HandleQuitEvent () -> void;
        auto HandleKeyDownEvent (const SDL_KeyboardEvent& event) -> void;
        auto HandleKeyUpEvent (const SDL_KeyboardEvent& event) -> void;
        auto HandleGamepadDownEvent (const SDL_GamepadButtonEvent& event) -> void;
        auto HandleGamepadUpEvent (const SDL_GamepadButtonEvent& event) -> void;
        auto HandleEvents () -> void;
        auto Update () -> void;
        auto UpdateGUI () -> void;
        auto Render () -> void;

    private: // Methods - General **********************************************

        auto OpenProgram (const fs::path& pPath) -> bool;
        auto DumpVideoRAM (const fs::path& pPath) -> bool;
        auto CloseProgram () -> void;

    private: // Methods - Dialogs **********************************************

        auto ShowOpenProgramDialog () -> void;
        auto ShowDumpVideoRAMDialog () -> void;

    private: // Methods - Main Menu Bar GUI ************************************

        auto UpdateMainMenuBarGUI () -> void;
        auto UpdateFileMenuGUI () -> void;
        auto UpdateViewMenuGUI () -> void;

    private: // Methods - Emulation Window GUI *********************************

        auto UpdateEmulationWindowGUI () -> void;
        
    private: // Methods - Registers Window GUI *********************************

        auto UpdateRegistersWindowGUI () -> void;

    private: // Methods - Memory Window GUI ************************************

        auto UpdateMemoryWindowGUI () -> void;

    private: // Members ********************************************************

        CPU::Program                        mProgram {};
        GB::System                          mSystem {};
        stx::ptr<SDL_Window>                mWindow { nullptr };
        stx::ptr<SDL_Renderer>              mRenderer { nullptr };
        stx::ptr<SDL_Texture>               mTexture { nullptr };
        stx::ptr<SDL_AudioStream>           mAudioStream { nullptr };
        std::array<float, kRingCapacity>    mAudioRingBuffer {};
        std::atomic_uint32_t                mAudioReadPos { 0 };
        std::atomic_uint32_t                mAudioWritePos { 0 };

        bool                                mRunning { true };
        std::string                         mProgramFilename {};
        std::string                         mWindowTitleBase { std::string(kApplicationWindowTitle) };

    private: // Members - CLI Arguments ****************************************

        std::string             mProgramFile {};

    private: // Members - Window States ****************************************

        bool        mShowDemoWindow { false };
        bool        mShowEmulationWindow { true };
        bool        mShowRegistersWindow { true };
        bool        mShowMemoryWindow { true };

        bool        mHoverEmulationWindow { false };
        bool        mHoverRegistersWindow { false };
        bool        mHoverMemoryWindow { false };

        bool        mFocusEmulationWindow { false };
        bool        mFocusRegistersWindow { false };
        bool        mFocusMemoryWindow { false };

        bool        mFirstFrame { false };

        std::uint32_t   mMemoryViewingAddress { 0x00000000 };
        bool            mMemoryFollowPC { true };

    };
}
