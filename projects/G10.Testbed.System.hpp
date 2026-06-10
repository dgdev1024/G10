/**
 * @file    G10.Testbed.System.hpp
 * @brief   Contains declarations for the G10 CPU Testbed's virtual hardware
 *          implementation, and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.CPU.Executive.hpp>
#include <G10.CPU.Program.hpp>

// Constants & Enumerations ****************************************************

namespace G10::Testbed
{
    constexpr std::uint32_t kRAMSize = 0x40;
    constexpr std::uint32_t kQRAMSize = 0x10000;
}

// Classes *********************************************************************

namespace G10::Testbed
{
    class G10_API System final : public CPU::IBus
    {
    public: // Constructors & Destructor ***************************************

        System ();

    public: // Methods *********************************************************

        auto Reset () -> void override;
        auto Clock (const std::uint64_t& pTick) -> bool override;
        auto Read (std::uint32_t pAddress, std::uint8_t& pDataOut) -> bool override;
        auto Write (std::uint32_t pAddress, std::uint8_t pData) -> bool override;

        auto Report () -> void;
        auto SetProgram (const CPU::Program& pProgram) -> void;

        inline auto GetCPU () -> CPU::Core&
            { return mCore; }
        inline auto GetCPU () const -> const CPU::Core&
            { return mCore; }

    private: // Methods ********************************************************

        void InitTime ();
        void UpdateTime ();

    private: // Members ********************************************************

        stx::optional_ref<const CPU::Program> mProgram { std::nullopt };
        CPU::Core mCore;
        std::array<std::uint8_t, kRAMSize> mRAM {};
        std::array<std::uint8_t, kQRAMSize> mQuickRAM {};
        std::tm mTime {};

    };
}