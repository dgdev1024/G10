/**
 * @file    G10.GB.System.hpp
 * @brief   Contains declarations for the G10.Boy system bus class, and
 *          related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.GB.RAM.hpp>
#include <G10.GB.Timer.hpp>
#include <G10.GB.Serial.hpp>
#include <G10.GB.Joypad.hpp>
#include <G10.GB.PPU.hpp>
#include <G10.GB.APU.hpp>
#include <G10.CPU.Executive.hpp>
#include <G10.CPU.Program.hpp>

// Classes *********************************************************************

namespace G10::GB
{
    class G10_API System final : public CPU::IBus
    {
        friend class PPU;

    public: // Constructors & Destructor ***************************************

        System ();

    public: // Methods - From CPU::IBus ****************************************

        auto Reset () -> void override;
        auto Clock (const std::uint64_t& pCycle) -> bool override;
        auto Read (std::uint32_t pAddress, std::uint8_t& pDataOut) -> bool override;
        auto Write (std::uint32_t pAddress, std::uint8_t pDataIn) -> bool override;

    public: // Methods *********************************************************

        auto Step () -> bool;
        auto StepFrame () -> bool;
        auto ValidateProgram (CPU::Program& pProgram) -> stx::expect<ProgramMetadata>;
        auto SetProgram (CPU::Program& pProgram) -> stx::expect_good;
        auto ClearProgram () -> void;

    public: // Methods - Accessors *********************************************

        inline auto GetProgramMetadata () const -> const ProgramMetadata&
            { return mProgramMetadata; }
        inline auto GetCPU () -> CPU::Core&
            { return mCore; }
        inline auto GetCPU () const -> const CPU::Core&
            { return mCore; }
        inline auto GetRAM () -> RAM&
            { return mRAM; }
        inline auto GetRAM () const -> const RAM&
            { return mRAM; }
        inline auto GetTimer () -> Timer&
            { return mTimer; }
        inline auto GetTimer () const -> const Timer&
            { return mTimer; }
        inline auto GetSerial () -> Serial&
            { return mSerial; }
        inline auto GetSerial () const -> const Serial&
            { return mSerial; }
        inline auto GetJoypad () -> Joypad&
            { return mJoypad; }
        inline auto GetJoypad () const -> const Joypad&
            { return mJoypad; }
        inline auto GetPPU () -> PPU&
            { return mPPU; }
        inline auto GetPPU () const -> const PPU&
            { return mPPU; }
        inline auto GetAPU () -> APU&
            { return mAPU; }
        inline auto GetAPU () const -> const APU&
            { return mAPU; }
        inline auto IsCGB () const -> bool
            { return mProgramMetadata.mCGBMode; }

    private: // Methods ********************************************************

        auto OnCoreStopInstruction () -> void;
        auto PrepareProgram (CPU::Program& pProgram) -> stx::expect<ProgramMetadata>;

    private: // Members ********************************************************

        stx::optional_ref<const CPU::Program> mProgram { std::nullopt };
        RAM mRAM;
        Timer mTimer;
        Serial mSerial;
        Joypad mJoypad;
        PPU mPPU;
        APU mAPU;
        CPU::Core mCore;
        ProgramMetadata mProgramMetadata {};
        bool mNoRestrict { false };

    };  
}