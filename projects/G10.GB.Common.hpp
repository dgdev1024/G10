/**
 * @file    G10.GB.Common.hpp
 * @brief   Contains definitions commonly used throughout the G10 Game Boy
 *          virtual hardware library.
 */

#pragma once

// Includes ********************************************************************

#include <G10.CPU.Common.hpp>

// Constants & Enumerations ****************************************************

namespace G10::GB
{
    constexpr std::uint32_t
        kMemProgramStartAddr            = 0x00000000,
        kMemProgramEndAddr              = 0x7FFFFFFF,
        kMemSpecialRamStartAddr         = 0x80000000,
            kMemVideoRamStartAddr       = 0x80008000,
            kMemVideoRamEndAddr         = 0x80009FFF,
            kMemOamStartAddr            = 0x8000FE00,
            kMemOamEndAddr              = 0x8000FE9F,
            kMemWaveRamStartAddr        = 0x8000FF30,
            kMemWaveRamEndAddr          = 0x8000FF3F,
        kMemSpecialRamEndAddr           = 0x9FFFFFFF,
        kMemGeneralRamStartAddr         = 0xA0000000,
            kMemWorkRamStartAddr        = 0xA0000000,
            kMemWorkRamEndAddr          = 0xAFFFFFFF,
            kMemSaveRamStartAddr        = 0xB0000000,
            kMemSaveRamEndAddr          = 0xBFFFFFFF,
        kMemGeneralRamEndAddr           = 0xBFFFFFFF,
        kMemExpansionRamStartAddr       = 0xC0000000,
        kMemExpansionRamEndAddr         = 0xFFFEFFFF,
        kMemQuickRamStartAddr           = 0xFFFF0000,
        kMemQuickRamEndAddr             = 0xFFFFFEFF;

    constexpr std::uint32_t
        kMinWramSize                    = 0x00002000,
        kMaxWramSize                    = 0x10000000,
        kMinSramSize                    = 0x00002000,
        kMaxSramSize                    = 0x10000000,
        kQramSize                       = 0x0000FF00;

    enum class Interrupt : std::uint8_t
    {
        VerticalBlank                   = 1,
        DisplayStatus                   = 2,
        Timer                           = 3,
        Serial                          = 4,
        Joypad                          = 5,
        Realtime                        = 6
    };

    enum class PortRegister : std::uint32_t
    {
        // G10 CPU Registers
        IE0                             = 0xFFFFFF00,
        IE1                             = 0xFFFFFF01,
        IE2                             = 0xFFFFFF02,
        IE3                             = 0xFFFFFF03,
        IRQ0                            = 0xFFFFFF04,
        IRQ1                            = 0xFFFFFF05,
        IRQ2                            = 0xFFFFFF06,
        IRQ3                            = 0xFFFFFF07,
        SPD                             = 0xFFFFFF08,

        // Timekeeping Port Registers
        // - Timer
        DIV                             = 0xFFFFFF10,
        TIMA                            = 0xFFFFFF11,
        TMA                             = 0xFFFFFF12,
        TAC                             = 0xFFFFFF13,
        // - Real-Time Clock
        RTCS                            = 0xFFFFFF18,
        RTCM                            = 0xFFFFFF19,
        RTCH                            = 0xFFFFFF1A,
        RTCDL                           = 0xFFFFFF1B,
        RTCDH                           = 0xFFFFFF1C,
        RTCC                            = 0xFFFFFF1D,
        RTCL                            = 0xFFFFFF1E,

        // Communications Port Registers
        // - Serial
        SB                              = 0xFFFFFF20,
        SC                              = 0xFFFFFF21,
        // - Infrared
        RP                              = 0xFFFFFF24,

        // APU Port Registers
        NR10                            = 0xFFFFFF30,
        NR11                            = 0xFFFFFF31,
        NR12                            = 0xFFFFFF32,
        NR13                            = 0xFFFFFF33,
        NR14                            = 0xFFFFFF34,
        NR21                            = 0xFFFFFF39,
        NR22                            = 0xFFFFFF3A,
        NR23                            = 0xFFFFFF3B,
        NR24                            = 0xFFFFFF3C,
        NR30                            = 0xFFFFFF40,
        NR31                            = 0xFFFFFF41,
        NR32                            = 0xFFFFFF42,
        NR33                            = 0xFFFFFF43,
        NR34                            = 0xFFFFFF44,
        NR41                            = 0xFFFFFF49,
        NR42                            = 0xFFFFFF4A,
        NR43                            = 0xFFFFFF4B,
        NR44                            = 0xFFFFFF4C,
        NR50                            = 0xFFFFFF50,
        NR51                            = 0xFFFFFF51,
        NR52                            = 0xFFFFFF52,
        PCM12                           = 0xFFFFFF58,
        PCM34                           = 0xFFFFFF59,

        // PPU Port Registers
        LCDC                            = 0xFFFFFF60,
        STAT                            = 0xFFFFFF61,
        SCY                             = 0xFFFFFF62,
        SCX                             = 0xFFFFFF63,
        WY                              = 0xFFFFFF64,
        WX                              = 0xFFFFFF65,
        LY                              = 0xFFFFFF66,
        LYC                             = 0xFFFFFF67,
        BGP                             = 0xFFFFFF68,
        OBP0                            = 0xFFFFFF69,
        OBP1                            = 0xFFFFFF6A,
        BGPI                            = 0xFFFFFF6B,
        BGPD                            = 0xFFFFFF6C,
        OBPI                            = 0xFFFFFF6D,
        OBPD                            = 0xFFFFFF6E,
        PPUX                            = 0xFFFFFF6F,

        // DMA Port Registers
        // - OAM DMA
        ODMAS                           = 0xFFFFFF70,
        ODMA1                           = 0xFFFFFF71,
        ODMA2                           = 0xFFFFFF72,
        ODMA3                           = 0xFFFFFF73,
        // - VRAM DMA
        VDMA0                           = 0xFFFFFF74,
        VDMA1                           = 0xFFFFFF75,
        VDMA2                           = 0xFFFFFF76,
        VDMA3                           = 0xFFFFFF77,
        VDMA4                           = 0xFFFFFF78,
        VDMA5                           = 0xFFFFFF79,
        VDMA6                           = 0xFFFFFF7A,

        // Input Port Registers
        // - Joypad
        JOYP                            = 0xFFFFFF80,
    };
}

// Structures ******************************************************************

namespace G10::GB
{
    struct ProgramMetadata final
    {
        std::string     mMagic {};
        std::uint32_t   mRequestedWramSize { 0 };
        std::uint32_t   mRequestedSramSize { 0 };
        std::string     mTitle {};
        std::string     mAuthor {};
        std::string     mDescription {};
        bool            mCGBMode { false };

    public: // Methods *********************************************************

        inline auto Reset () -> void
        {
            mMagic.clear();
            mRequestedWramSize = 0;
            mRequestedSramSize = 0;
            mTitle.clear();
            mAuthor.clear();
            mDescription.clear();
            mCGBMode = false;
        }

    };
}
