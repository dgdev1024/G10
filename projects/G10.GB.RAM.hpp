/**
 * @file    G10.GB.RAM.hpp
 * @brief   Contains declarations for the G10.Boy's general-purpose RAM
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

// Classes *********************************************************************

namespace G10::GB
{
    class G10_API RAM final
    {
    public: // Constructors & Destructor ***************************************

        explicit RAM (System& pSystem);

    public: // Methods *********************************************************

        auto Reset () -> void;
        auto ResizeWorkRAM (std::uint32_t pSize) -> bool;
        auto ResizeExternalRAM (std::uint32_t pSize) -> bool;
        auto LoadExternalRAM (const fs::path& pPath) -> stx::expect_good;
        auto SaveExternalRAM (const fs::path& pPath) -> stx::expect_good;

    public: // Methods - Bus Access ********************************************

        auto ReadWorkRAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool;
        auto ReadExternalRAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool;
        auto ReadQuickRAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool;
        auto WriteWorkRAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool;
        auto WriteExternalRAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool;
        auto WriteQuickRAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool;

    private: // Members ********************************************************

        System& mSystem;
        std::vector<std::uint8_t> mWorkRAM {};
        std::vector<std::uint8_t> mExternalRAM {};
        std::array<std::uint8_t, kQramSize> mQuickRAM {};

    };
}