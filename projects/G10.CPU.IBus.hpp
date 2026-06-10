/**
 * @file    G10.CPU.IBus.hpp
 * @brief   Contains declarations for the G10 CPU's system bus interface, and
 *          related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.CPU.Common.hpp>

// Classes *********************************************************************

namespace G10::CPU
{
    class G10_API IBus
    {
    public: // Methods *********************************************************

        virtual auto Reset () -> void = 0;
        virtual auto Clock (const std::uint64_t&) -> bool = 0;
        virtual auto Read (std::uint32_t, std::uint8_t&) -> bool = 0;
        virtual auto Write (std::uint32_t, std::uint8_t) -> bool = 0;
        
    };
}
