/**
 * @file    G10.CPU.Common.hpp
 * @brief   Contains definitions commonly used throughout the `G10.CPU` library.
 */

#pragma once

// Includes ********************************************************************

#include <G10.Common.hpp>

// Constants & Enumerations ****************************************************

namespace G10::CPU
{
    constexpr double
        kDefaultClockSpeed      = 1.0 / 8388608.0,
        kDefaultHighClockSpeed  = 1.0 / 16777216.0;

    constexpr std::uint32_t 
        kMemMetadataStartAddr   = 0x00000000,
        kMemMetadataEndAddr     = 0x00000FFF,
        kMemInterruptStartAddr  = 0x00001000,
        kMemInterruptEndAddr    = 0x00001FFF,
        kMemProgramStartAddr    = 0x00002000,
        kMemProgramEndAddr      = 0x7FFFFFFF,
        kMemRamStartAddr        = 0x80000000,
        kMemQuickRamStartAddr   = 0xFFFF0000,
        kMemPortStartAddr       = 0xFFFFFF00,
        kMemRamEndAddr          = 0xFFFFFFFF;

    constexpr std::uint32_t 
        kStackHardBottom        = 0x80000000,
        kStackHardTop           = 0xFFFFFEFC;

    constexpr std::uint32_t
        kInitialProgramCounter  = 0x00002000,
        kInitialStackPointer    = 0xFFFFFEFC;

    constexpr std::uint8_t
        kRegisterCount          = 16,
        kInterruptCount         = 32;

    constexpr std::uint64_t
        kTicksPerMachineCycle   = 4;

    enum class RegisterAccess : std::uint8_t
    {
        DoubleWord  = 0b00010000,
        Word        = 0b00100000,
        HighByte    = 0b01000000,
        LowByte     = 0b10000000
    };

    enum class Register : std::uint8_t
    {
        D0 = 0b00010000, D1, D2,  D3,  D4,  D5,  D6,  D7,
        D8,              D9, D10, D11, D12, D13, D14, D15,
        W0 = 0b00100000, W1, W2,  W3,  W4,  W5,  W6,  W7,
        W8,              W9, W10, W11, W12, W13, W14, W15,
        H0 = 0b01000000, H1, H2,  H3,  H4,  H5,  H6,  H7,
        H8,              H9, H10, H11, H12, H13, H14, H15,
        L0 = 0b10000000, L1, L2,  L3,  L4,  L5,  L6,  L7,
        L8,              L9, L10, L11, L12, L13, L14, L15,

        ACC32 = D0,
        ACC16 = W0,
        ACC8  = L0
    };

    enum class HardwareRegister : std::uint8_t
    {
        IE0,
        IE1,
        IE2,
        IE3,
        IRQ0,
        IRQ1,
        IRQ2,
        IRQ3,
        SPD
    };

    enum class Flag : std::uint8_t
    {
        Overflow        = (1 << 3),
        Carry           = (1 << 4),
        HalfCarry       = (1 << 5),
        Subtract        = (1 << 6),
        Zero            = (1 << 7),

        V               = Overflow,
        C               = Carry,
        H               = HalfCarry,
        N               = Subtract,
        Z               = Zero
    };

    enum class Condition : std::uint8_t
    {
        None,
        ZeroSet,
        ZeroClear,
        CarrySet,
        CarryClear,
        OverflowSet,
        OverflowClear,

        NC              = None,
        ZS              = ZeroSet,
        ZC              = ZeroClear,
        CS              = CarrySet,
        CC              = CarryClear,
        VS              = OverflowSet,
        VC              = OverflowClear,
    };

    enum class Exception : std::uint8_t
    {
        None,
        InvalidInstruction,
        InvalidArgument,
        InvalidReadAccess,
        InvalidWriteAccess,
        InvalidExecuteAccess,
        DivideByZero,
        StackOverflow,
        StackUnderflow,
        HardwareError,
        WatchdogTimeout,
        DoubleFault
    };

    enum class InstructionType : std::uint8_t
    {
        NOP,
        STOP,
        HALT,
        DI,
        EI,
        EII,
        DAA,
        SCF,
        CCF,
        CLV,
        SEV,
        REX,
        LEC,
        LD,
        LDQ,
        LDP,
        ST,
        STQ,
        STP,
        MV,
        MWH,
        MWL,
        LSP,
        POP,
        SSP,
        PUSH,
        SPO,
        SPI,
        JMP,
        JPB,
        CALL,
        INT,
        RET,
        RETI,
        MFI,
        MFO,
        ADD,
        ADC,
        SUB,
        SBC,
        INC,
        DEC,
        AND,
        OR,
        XOR,
        NOT,
        CMP,
        SLA,
        SRA,
        SRL,
        SWAP,
        RLA,
        RL,
        RLCA,
        RLC,
        RRA,
        RR,
        RRCA,
        RRC,
        BIT,
        SET,
        RES,
        TOG,
        LDI,
        LDD,
        STI,
        STD,
        ASP,
        LASP,
        ISP,
        DSP,
        ASR
    };

    enum class InstructionMnemonic : std::uint8_t
    {
        NOP                         = 0x00,
        STOP                        = 0x01,
        HALT                        = 0x02,
        DI                          = 0x03,
        EI                          = 0x04,
        EII                         = 0x05,
        DAA                         = 0x06,
        SCF                         = 0x07,
        CCF                         = 0x08,
        CLV                         = 0x09,
        SEV                         = 0x0A,
        REX_XY                      = 0x0B,
        LEC                         = 0x0C,
        LD_LX_IMM8                  = 0x10,
        LD_LX_pIMM32                = 0x11,
        LD_LX_pDY                   = 0x12,
        LDQ_LX_pIMM16               = 0x13,
        LDQ_LX_pWY                  = 0x14,
        LDP_LX_pIMM8                = 0x15,
        LDP_LX_pLY                  = 0x16,
        ST_pIMM32_LY                = 0x17,
        ST_pDX_LY                   = 0x18,
        STQ_pIMM16_LY               = 0x19,
        STQ_pWX_LY                  = 0x1A,
        STP_pIMM8_LY                = 0x1B,
        STP_pLX_LY                  = 0x1C,
        MV_LX_LY                    = 0x1D,
        MV_HX_LY                    = 0x1E,
        MV_LX_HY                    = 0x1F,
        LD_WX_IMM16                 = 0x20,
        LD_WX_pIMM32                = 0x21,
        LD_WX_pDY                   = 0x22,
        LDQ_WX_pIMM16               = 0x23,
        LDQ_WX_pWY                  = 0x24,
        ST_pIMM32_WY                = 0x27,
        ST_pDX_WY                   = 0x28,
        STQ_pIMM16_WY               = 0x29,
        STQ_pWX_WY                  = 0x2A,
        MV_WX_WY                    = 0x2D,
        MWH_DX_WY                   = 0x2E,
        MWL_WX_DY                   = 0x2F,
        LD_DX_IMM32                 = 0x30,
        LD_DX_pIMM32                = 0x31,
        LD_DX_pDY                   = 0x32,
        LDQ_DX_pIMM16               = 0x33,
        LDQ_DX_pWY                  = 0x34,
        LSP_IMM32                   = 0x35,
        POP_DX                      = 0x36,
        ST_pIMM32_DY                = 0x37,
        ST_pDX_DY                   = 0x38,
        STQ_pIMM16_DY               = 0x39,
        STQ_pWX_DY                  = 0x3A,
        SSP_pIMM32                  = 0x3B,
        PUSH_DY                     = 0x3C,
        MV_DX_DY                    = 0x3D,
        SPO_DX                      = 0x3E,
        SPI_DY                      = 0x3F,
        JMP_X_IMM32                 = 0x40,
        JMP_X_DY                    = 0x41,
        JPB_X_SIMM16                = 0x42,
        CALL_X_IMM32                = 0x43,
        INT_XY                      = 0x44,
        RET_X                       = 0x45,
        RETI                        = 0x46,
        MFI_LY                      = 0x4E,
        MFO_LX                      = 0x4F,
        ADD_L0_IMM8                 = 0x50,
        ADD_L0_LY                   = 0x51,
        ADD_L0_pDY                  = 0x52,
        ADC_L0_IMM8                 = 0x53,
        ADC_L0_LY                   = 0x54,
        ADC_L0_pDY                  = 0x55,
        SUB_L0_IMM8                 = 0x56,
        SUB_L0_LY                   = 0x57,
        SUB_L0_pDY                  = 0x58,
        SBC_L0_IMM8                 = 0x59,
        SBC_L0_LY                   = 0x5A,
        SBC_L0_pDY                  = 0x5B,
        INC_LX                      = 0x5C,
        INC_pDX                     = 0x5D,
        DEC_LX                      = 0x5E,
        DEC_pDX                     = 0x5F,
        ADD_W0_IMM16                = 0x60,
        ADD_W0_WY                   = 0x61,
        ADD_D0_IMM32                = 0x62,
        ADD_D0_DY                   = 0x63,
        SUB_W0_IMM16                = 0x64,
        SUB_W0_WY                   = 0x65,
        SUB_D0_IMM32                = 0x66,
        SUB_D0_DY                   = 0x67,
        INC_WX                      = 0x6C,
        INC_DX                      = 0x6D,
        DEC_WX                      = 0x6E,
        DEC_DX                      = 0x6F,
        AND_L0_IMM8                 = 0x70,
        AND_L0_LY                   = 0x71,
        AND_L0_pDY                  = 0x72,
        OR_L0_IMM8                  = 0x73,
        OR_L0_LY                    = 0x74,
        OR_L0_pDY                   = 0x75,
        XOR_L0_IMM8                 = 0x76,
        XOR_L0_LY                   = 0x77,
        XOR_L0_pDY                  = 0x78,
        NOT_LX                      = 0x79,
        NOT_pDX                     = 0x7A,
        CMP_L0_IMM8                 = 0x7D,
        CMP_L0_LY                   = 0x7E,
        CMP_L0_pDY                  = 0x7F,
        SLA_LX                      = 0x80,
        SLA_pDX                     = 0x81,
        SRA_LX                      = 0x82,
        SRA_pDX                     = 0x83,
        SRL_LX                      = 0x84,
        SRL_pDX                     = 0x85,
        SWAP_LX                     = 0x86,
        SWAP_pDX                    = 0x87,
        SWAP_WX                     = 0x88,
        SWAP_DX                     = 0x89,
        RLA                         = 0x90,
        RL_LX                       = 0x91,
        RL_pDX                      = 0x92,
        RLCA                        = 0x93,
        RLC_LX                      = 0x94,
        RLC_pDX                     = 0x95,
        RRA                         = 0x96,
        RR_LX                       = 0x97,
        RR_pDX                      = 0x98,
        RRCA                        = 0x99,
        RRC_LX                      = 0x9A,
        RRC_pDX                     = 0x9B,
        BIT_Y_LX                    = 0xA0,
        BIT_Y_pDX                   = 0xA1,
        SET_Y_LX                    = 0xA2,
        SET_Y_pDX                   = 0xA3,
        RES_Y_LX                    = 0xA4,
        RES_Y_pDX                   = 0xA5,
        TOG_Y_LX                    = 0xA6,
        TOG_Y_pDX                   = 0xA7,
        LDI_LX_pDY                  = 0xB0,
        LDD_LX_pDY                  = 0xB1,
        STI_pDX_LY                  = 0xB2,
        STD_pDX_LY                  = 0xB3,
        ASP_SIMM8                   = 0xB4,
        ST_pDX_IMM8                 = 0xB5,
        LASP_DX_SIMM8               = 0xB6,
        ISP                         = 0xB7,
        DSP                         = 0xB8,
        ASR_DX                      = 0xB9,
    };
}
