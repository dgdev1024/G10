/**
 * @file    G10.Boy.Application.cpp
 * @brief   Contains implementations for the G10.Boy emulator's application
 *          class, and related definitions.
 */

// Includes ********************************************************************

#include <print>
#include <pfd.hpp>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <G10.Boy.Application.hpp>

// Private Macros **************************************************************

// SDL3 removed this macro, so re-define it here.
#define SDL_INIT_EVERYTHING \
    SDL_INIT_VIDEO | \
    SDL_INIT_AUDIO | \
    SDL_INIT_GAMEPAD | \
    SDL_INIT_SENSOR | \
    SDL_INIT_CAMERA

// Static Functions ************************************************************

namespace G10::Boy
{
    template <typename... As>
    static auto SetWindowTitle (SDL_Window* pWindow, std::string_view pFormat, 
        As... pArgs) -> void
    {
        std::string windowTitle = std::vformat(
            pFormat,
            std::make_format_args(pArgs...)
        );
        SDL_SetWindowTitle(pWindow, windowTitle.c_str());
    }

    auto OnAudio (void* pUserdata, SDL_AudioStream* pStream, int pAdditional,
        int pTotal) -> void
    {
        auto& app = *reinterpret_cast<Application*>(pUserdata);

        // From `pAdditional` (provided in bytes), derive the number of
        // floating-point samples requested.
        std::uint32_t samplesRequested = pAdditional / sizeof(float);

        // Keep track of how many unread samples are actually in our ring
        // buffer.
        std::uint32_t
            samplesAvailable    = (app.mAudioWritePos - app.mAudioReadPos),
            samplesToRead       = std::min(samplesRequested, samplesAvailable);
        if (samplesToRead > 0)
        {
            std::uint32_t readIndex = (app.mAudioReadPos % kRingCapacity);

            // Would this read cross the end of the ring buffer's physical
            // array boundary?
            std::uint32_t
                firstChunkSamples   = std::min(samplesToRead, kRingCapacity - readIndex),
                secondChunkSamples  = samplesToRead - firstChunkSamples;

            // Put the first chunk from the read buffer. If it does cross that
            // boundary, then put a second chunk with the remaining data from
            // index zero.
            SDL_PutAudioStreamData(
                pStream,
                app.mAudioRingBuffer.data() + readIndex,
                firstChunkSamples * sizeof(float)
            );
            if (secondChunkSamples > 0)
            {
                SDL_PutAudioStreamData(
                    pStream,
                    app.mAudioRingBuffer.data(),
                    secondChunkSamples * sizeof(float)
                );
            }

            // Advance the read pointer as appropriate.
            app.mAudioReadPos += samplesToRead;
        }
    }
}

// Public - Constructors & Destructor ******************************************

namespace G10::Boy
{
    Application::Application (int argc, const char** argv)
    {
        ParseArguments(argc, argv);
        InitializeSDL();
        InitializeImGui();
        InitializeApplication();
    }

    Application::~Application ()
    {
        ShutdownImGui();
        ShutdownSDL();
    }
}

// Public Methods **************************************************************

namespace G10::Boy
{
    auto Application::Start () -> std::int32_t
    {
        std::uint64_t time = 0;
        std::uint64_t frameCount = 0;
        std::uint64_t fpsSampleStart = SDL_GetTicksNS();
        constexpr std::uint64_t kFpsUpdateIntervalNS = 1000000000ULL;

        while (mRunning == true)
        {
            if (mShowEmulationWindow == true)
                { mSystem.StepFrame(); }

            HandleEvents();
            Update();
            UpdateGUI();
            Render();

            ++frameCount;
            const std::uint64_t now = SDL_GetTicksNS();
            if (now - fpsSampleStart >= kFpsUpdateIntervalNS)
            {
                const double fps = static_cast<double>(frameCount) * 1'000'000'000.0 /
                    static_cast<double>(now - fpsSampleStart);
                fpsSampleStart = now;
                frameCount = 0;

                SetWindowTitle(mWindow, "{} - {:.1f} FPS",
                    mWindowTitleBase,
                    fps);
            }
        }

        return 0;
    }
}

// Private Methods - Initialization & Shutdown *********************************

namespace G10::Boy
{
    auto Application::ParseArguments (int argc, const char** argv) -> void
    {
        for (int i = 1; i < argc; )
        {
            std::string arg = argv[i++];
            if (arg == "-r" || arg == "--rom")
            {
                if (i < argc)
                    { mProgramFile = argv[i++]; }
                else
                    { std::println(stderr, "Warning: Missing ROM file after '{}'", arg); }
            }
            else break;
        }
    }

    auto Application::InitializeSDL () -> void
    {
        // Application Metadata and Init
        SDL_SetAppMetadata(kApplicationWindowTitle.data(), "1.0.0", "G10.Boy");
        SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_CREATOR_STRING, "Dennis W. Griffin");
        if (SDL_Init(SDL_INIT_EVERYTHING) == false)
        {
            throw stx::runtime_error { 
                "Could not initialize SDL: '{}'", SDL_GetError() };
        }

        // Application Window
        mWindow = SDL_CreateWindow(
            kApplicationWindowTitle.data(),
            kApplicationWindowWidth,
            kApplicationWindowHeight,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN
        );
        if (mWindow == nullptr)
        {
            throw stx::runtime_error { 
                "Could not create window: '{}'", SDL_GetError() };
        }
        else
        {
            SDL_SetWindowPosition(mWindow, SDL_WINDOWPOS_CENTERED, 
                SDL_WINDOWPOS_CENTERED);
            SDL_ShowWindow(mWindow);
        }

        // Application Renderer
        mRenderer = SDL_CreateRenderer(mWindow, nullptr);
        if (mRenderer == nullptr)
        {
            throw stx::runtime_error { 
                "Could not create renderer: '{}'", SDL_GetError() };
        }
        else
        {
            SDL_SetRenderVSync(mRenderer, SDL_RENDERER_VSYNC_ADAPTIVE);
        }

        // Target Texture
        mTexture = SDL_CreateTexture(
            mRenderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            160, 144
        );
        if (mTexture == nullptr)
        {
            throw stx::runtime_error { 
                "Could not create target texture: '{}'", SDL_GetError() };
        }

        // Audio Device
        SDL_AudioSpec spec = {};
        spec.freq = kSampleRate;
        spec.format = SDL_AUDIO_F32;
        spec.channels = 2;
        mAudioStream = SDL_OpenAudioDeviceStream(
            SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
            &spec,
            OnAudio,
            this
        );
        if (mAudioStream == nullptr)
        {
            throw stx::runtime_error { 
                "Could not create audio stream: '{}'", SDL_GetError() };
        }
        else
        {
            SDL_ResumeAudioStreamDevice(mAudioStream);
        }
    }

    auto Application::InitializeImGui () -> void
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplSDL3_InitForSDLRenderer(mWindow, mRenderer);
        ImGui_ImplSDLRenderer3_Init(mRenderer);

        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();
    }

    auto Application::InitializeApplication () -> void
    {
        if (mProgramFile.empty() == false)
            { OpenProgram(mProgramFile); }

        auto& apu = mSystem.GetAPU();
        apu.SetSampleCallback([this] (const GB::System&, float pLeft, float pRight)
        {
            std::uint32_t
                writePos = mAudioWritePos,
                cap = kRingCapacity;
            if ((writePos + 2 - mAudioReadPos) < cap)
            {
                mAudioRingBuffer[writePos % cap] = pLeft;
                mAudioRingBuffer[(writePos + 1) % cap] = pRight;
                mAudioWritePos = (writePos + 2);
            }
        });
    }

    auto Application::ShutdownSDL () -> void
    {
        if (mAudioStream != nullptr)    { SDL_PauseAudioStreamDevice(mAudioStream); SDL_DestroyAudioStream(mAudioStream); }
        if (mTexture != nullptr)        { SDL_DestroyTexture(mTexture); }
        if (mRenderer != nullptr)       { SDL_DestroyRenderer(mRenderer); }
        if (mWindow != nullptr)         { SDL_DestroyWindow(mWindow); }
        SDL_Quit();
    }

    auto Application::ShutdownImGui () -> void
    {
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }
}

// Private Methods - Lifecycle *************************************************

namespace G10::Boy
{
    auto Application::HandleQuitEvent () -> void
    {
        mRunning = false;
    }

    auto Application::HandleKeyDownEvent (const SDL_KeyboardEvent& event) -> void
    {
        if (event.key == SDLK_Q)
        {
            if (event.mod & SDL_KMOD_CTRL)
                { HandleQuitEvent(); }
        }
        else if (event.key == SDLK_O)
        {
            if (event.mod & SDL_KMOD_CTRL)
                { ShowOpenProgramDialog(); }
        }

        #if defined(G10_CONFIG_DEBUG)
        if (event.key == SDLK_ESCAPE)
        {
            mRunning = false;
        }
        #endif

        if (mFocusEmulationWindow == true)
        {
            auto& joypad = mSystem.GetJoypad();
            switch (event.key)
            {
                case SDLK_W:        joypad.PressButton(GB::JoypadButton::Up); break;
                case SDLK_A:        joypad.PressButton(GB::JoypadButton::Left); break;
                case SDLK_S:        joypad.PressButton(GB::JoypadButton::Down); break;
                case SDLK_D:        joypad.PressButton(GB::JoypadButton::Right); break;
                case SDLK_J:        joypad.PressButton(GB::JoypadButton::A); break;
                case SDLK_K:        joypad.PressButton(GB::JoypadButton::B); break;
                case SDLK_RETURN:   joypad.PressButton(GB::JoypadButton::Start); break;
                case SDLK_SPACE:    joypad.PressButton(GB::JoypadButton::Select); break;
            }
        }
    }

    auto Application::HandleKeyUpEvent (const SDL_KeyboardEvent& event) -> void
    {
        if (mFocusEmulationWindow == true)
        {
            auto& joypad = mSystem.GetJoypad();
            switch (event.key)
            {
                case SDLK_W:        joypad.ReleaseButton(GB::JoypadButton::Up); break;
                case SDLK_A:        joypad.ReleaseButton(GB::JoypadButton::Left); break;
                case SDLK_S:        joypad.ReleaseButton(GB::JoypadButton::Down); break;
                case SDLK_D:        joypad.ReleaseButton(GB::JoypadButton::Right); break;
                case SDLK_J:        joypad.ReleaseButton(GB::JoypadButton::A); break;
                case SDLK_K:        joypad.ReleaseButton(GB::JoypadButton::B); break;
                case SDLK_RETURN:   joypad.ReleaseButton(GB::JoypadButton::Start); break;
                case SDLK_SPACE:    joypad.ReleaseButton(GB::JoypadButton::Select); break;
            }
        }
    }

    auto Application::HandleGamepadDownEvent (const SDL_GamepadButtonEvent& event) -> void
    {
        if (mFocusEmulationWindow == true)
        {
            auto& joypad = mSystem.GetJoypad();
            switch (event.button)
            {
                case SDL_GAMEPAD_BUTTON_DPAD_UP:        joypad.PressButton(GB::JoypadButton::Up); break;
                case SDL_GAMEPAD_BUTTON_DPAD_DOWN:      joypad.PressButton(GB::JoypadButton::Down); break;
                case SDL_GAMEPAD_BUTTON_DPAD_LEFT:      joypad.PressButton(GB::JoypadButton::Left); break;
                case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:     joypad.PressButton(GB::JoypadButton::Right); break;
                case SDL_GAMEPAD_BUTTON_SOUTH:           joypad.PressButton(GB::JoypadButton::A); break;
                case SDL_GAMEPAD_BUTTON_EAST:              joypad.PressButton(GB::JoypadButton::B); break;
                case SDL_GAMEPAD_BUTTON_START:          joypad.PressButton(GB::JoypadButton::Start); break;
                case SDL_GAMEPAD_BUTTON_BACK:           joypad.PressButton(GB::JoypadButton::Select); break;
            }
        }
    }

    auto Application::HandleGamepadUpEvent (const SDL_GamepadButtonEvent& event) -> void
    {
        if (mFocusEmulationWindow == true)
        {
            auto& joypad = mSystem.GetJoypad();
            switch (event.button)
            {
                case SDL_GAMEPAD_BUTTON_DPAD_UP:        joypad.ReleaseButton(GB::JoypadButton::Up); break;
                case SDL_GAMEPAD_BUTTON_DPAD_DOWN:      joypad.ReleaseButton(GB::JoypadButton::Down); break;
                case SDL_GAMEPAD_BUTTON_DPAD_LEFT:      joypad.ReleaseButton(GB::JoypadButton::Left); break;
                case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:     joypad.ReleaseButton(GB::JoypadButton::Right); break;
                case SDL_GAMEPAD_BUTTON_SOUTH:           joypad.ReleaseButton(GB::JoypadButton::A); break;
                case SDL_GAMEPAD_BUTTON_EAST:              joypad.ReleaseButton(GB::JoypadButton::B); break;
                case SDL_GAMEPAD_BUTTON_START:          joypad.ReleaseButton(GB::JoypadButton::Start); break;
                case SDL_GAMEPAD_BUTTON_BACK:           joypad.ReleaseButton(GB::JoypadButton::Select); break;
            }
        }
    }

    auto Application::HandleEvents () -> void
    {
        SDL_Event event;
        while (SDL_PollEvent(&event) == true)
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            switch (event.type)
            {
                case SDL_EVENT_QUIT:                HandleQuitEvent(); break;
                case SDL_EVENT_KEY_DOWN:            HandleKeyDownEvent(event.key); break;
                case SDL_EVENT_KEY_UP:              HandleKeyUpEvent(event.key); break;
                case SDL_EVENT_GAMEPAD_BUTTON_DOWN: HandleGamepadDownEvent(event.gbutton); break;
                case SDL_EVENT_GAMEPAD_BUTTON_UP:   HandleGamepadUpEvent(event.gbutton); break;
                default:                            break;
            }
        }
    }

    auto Application::Update () -> void
    {
        if (mTexture != nullptr && 
            mShowEmulationWindow == true)
        {
            const auto& frameBuffer = mSystem.GetPPU().GetFrameBuffer();

            std::int32_t pitch = 0;
            std::uint32_t* pixels = nullptr;
            SDL_LockTexture(mTexture, nullptr, 
                reinterpret_cast<void**>(&pixels), &pitch);
            SDL_memcpy(pixels, frameBuffer.data(), 
                frameBuffer.size() * sizeof(std::uint32_t));
            SDL_UnlockTexture(mTexture);
        }
    }

    auto Application::UpdateGUI () -> void
    {
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport();
        UpdateMainMenuBarGUI();

        if (mShowDemoWindow == true)
            { ImGui::ShowDemoWindow(&mShowDemoWindow); }
        if (mShowEmulationWindow == true)
            { UpdateEmulationWindowGUI(); }
    }

    auto Application::Render () -> void
    {
        ImGui::Render();
        SDL_RenderClear(mRenderer);

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), mRenderer);
        SDL_RenderPresent(mRenderer);
    }
}

// Private Methods - General ***************************************************

namespace G10::Boy
{
    auto Application::OpenProgram (const fs::path& pPath) -> bool
    {
        CPU::Program program;
        auto loadFile = program.LoadFile(pPath);
        if (loadFile.has_value() == false)
        {
            pfd::message(
                "Error Opening Program",
                std::format(
                    "Could not load program file '{}'\n"
                    " - {}",
                    pPath.string(),
                    loadFile.error()),
                pfd::choice::ok,
                pfd::icon::error
            );

            return false;
        }

        auto metadata = mSystem.ValidateProgram(program);
        if (metadata.has_value() == false)
        {
            pfd::message(
                "Error Opening Program",
                std::format(
                    "Could not validate program file '{}'\n"
                    " - {}",
                    pPath.string(),
                    metadata.error()),
                pfd::choice::ok,
                pfd::icon::error
            );

            return false;
        }

        mProgram = std::move(program);
        mSystem.SetProgram(mProgram);
        mProgramFilename = pPath.string();
        mWindowTitleBase = std::format("{} [{}] - {}",
            mSystem.GetProgramMetadata().mTitle,
            mProgramFilename,
            kApplicationWindowTitle);
        SetWindowTitle(mWindow, "{}", mWindowTitleBase);

        return true;
    }

    auto Application::DumpVideoRAM (const fs::path& pPath) -> bool
    {
        auto vram0 = mSystem.GetPPU().GetVideoRAM(false);
        auto vram1 = mSystem.GetPPU().GetVideoRAM(true);
        const auto resolved = fs::absolute(pPath).lexically_normal();

        std::fstream file { resolved, std::ios::out | std::ios::binary };
        if (file.is_open() == false)
        {
            pfd::message(
                "Error Dumping Video RAM",
                std::format(
                    "Could not create file '{}'\n"
                    " - {}",
                    pPath.string(),
                    strerror(errno)),
                pfd::choice::ok,
                pfd::icon::error
            );

            return false;
        }

        file.write(reinterpret_cast<const char*>(vram0.data()), vram0.size());
        file.write(reinterpret_cast<const char*>(vram1.data()), vram1.size());
        file.close();

        return true;
    }

    auto Application::CloseProgram () -> void
    {
        mSystem.ClearProgram();
        mProgram.Clear();
    }
}

// Private Methods - Dialogs ***************************************************

namespace G10::Boy
{
    auto Application::ShowOpenProgramDialog () -> void
    {
        auto result = pfd::open_file(
            "Open Program...",
            (mProgramFilename.empty() ?
                fs::current_path() :
                fs::path(mProgramFilename).parent_path()),
            {
                "G10 Executable File (*.g10)", "*.g10",
                "Generic Binary File (*.bin)", "*.bin"
            }
        ).result();

        if (result.empty() == false)
        {
            OpenProgram(result[0]);
        }
    }

    auto Application::ShowDumpVideoRAMDialog () -> void
    {
        auto result = pfd::save_file(
            "Dump Video RAM...",
            (mProgramFilename.empty() ?
                fs::current_path() :
                fs::path(mProgramFilename).parent_path()),
            {
                "Binary File (*.bin)", "*.bin"
            }
        ).result();

        if (result.empty() == false)
        {
            DumpVideoRAM(result);
        }
    }
}

// Private Methods - Main Menu Bar GUI *****************************************

namespace G10::Boy
{
    auto Application::UpdateMainMenuBarGUI () -> void
    {
        if (ImGui::BeginMainMenuBar())
        {
            UpdateFileMenuGUI();
            UpdateViewMenuGUI();
            ImGui::EndMainMenuBar();
        }
    }

    auto Application::UpdateFileMenuGUI () -> void
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "Ctrl+O"))
                { ShowOpenProgramDialog(); }
            ImGui::Separator();
            if (ImGui::MenuItem("Dump Video RAM...", nullptr, nullptr, 
                (mProgramFilename.empty() == false)))
                { ShowDumpVideoRAMDialog(); }

            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Ctrl+Q"))
                { HandleQuitEvent(); }

            ImGui::EndMenu();
        }
    }

    auto Application::UpdateViewMenuGUI () -> void
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Emulation Window", nullptr, &mShowEmulationWindow);
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo Window", nullptr, &mShowDemoWindow);

            ImGui::EndMenu();
        }
    }
}

// Private Methods - Emulation Window GUI **************************************

namespace G10::Boy
{
    auto Application::UpdateEmulationWindowGUI () -> void
    {
        // Emulation Window
        // - Minimum size: 160, 144 (10:9 aspect ratio)
        ImGui::SetNextWindowSize(ImVec2(160, 144), ImGuiCond_FirstUseEver);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        ImGui::Begin("Emulation", &mShowEmulationWindow);
        {
            mHoverEmulationWindow = ImGui::IsWindowHovered();
            mFocusEmulationWindow = ImGui::IsWindowFocused();

            ImVec2 cursorPos = ImGui::GetCursorPos();
            ImVec2 windowSize = ImGui::GetContentRegionAvail();
            ImVec2 scaleVec = ImVec2(windowSize.x / 160, windowSize.y / 144);
            float scale = (scaleVec.x < scaleVec.y) ? scaleVec.x : scaleVec.y;
            ImVec2 imageSize = ImVec2(160 * scale, 144 * scale);
            ImGui::SetCursorPosX(cursorPos.x + (windowSize.x - imageSize.x) * 0.5f);
            ImGui::SetCursorPosY(cursorPos.y + (windowSize.y - imageSize.y) * 0.5f);
            ImGui::Image(mTexture, imageSize);
        }
        ImGui::End();
        ImGui::PopStyleVar(1);
    }
}
