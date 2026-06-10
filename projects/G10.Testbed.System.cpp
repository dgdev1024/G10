/**
 * @file    G10.Testbed.System.cpp
 * @brief   Contains implementations for the G10 CPU Testbed's virtual hardware
 *          implementation, and related definitions.
 */

// Includes ********************************************************************

#include <print>
#include <G10.Testbed.System.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::Testbed
{
    System::System () :
        CPU::IBus {},
        mProgram { std::nullopt },
        mCore { *this }
    {
        Reset();
    }
}

// Public Methods **************************************************************

namespace G10::Testbed
{
    auto System::Reset () -> void
    {
        mCore.Reset();
        if (mProgram.has_value() == true)
        { 
            mCore.SetProgramCounter(mProgram->GetEntryPoint()); 
        }

        mRAM.fill(0x00);
        mQuickRAM.fill(0x00);
    }

    auto System::Clock (const std::uint64_t& pTick) -> bool
    {
        // Implementation for clocking the system
        UpdateTime();
        return true;
    }

    auto System::Read (std::uint32_t pAddress, std::uint8_t& pDataOut) -> bool
    {
        if (mProgram && pAddress < 0x80000000)
        {
            if (mProgram->Read(pAddress, pDataOut))
                { return true; }
        }
        else if (pAddress >= 0x80000000 && pAddress < 0x80000000 + kRAMSize)
        {
            pDataOut = mRAM[pAddress - 0x80000000];
            return true;
        }
        else if (pAddress >= 0xFFFF0000 && pAddress <= 0xFFFFFEFF)
        {
            pDataOut = mQuickRAM[pAddress - 0xFFFF0000];
            return true;
        }
        else
        {
            switch (pAddress)
            {
                case 0xFFFFFF00: mCore.ReadHardwareRegister(CPU::HardwareRegister::IE0, pDataOut); break;
                case 0xFFFFFF01: mCore.ReadHardwareRegister(CPU::HardwareRegister::IE1, pDataOut); break;
                case 0xFFFFFF02: mCore.ReadHardwareRegister(CPU::HardwareRegister::IE2, pDataOut); break;
                case 0xFFFFFF03: mCore.ReadHardwareRegister(CPU::HardwareRegister::IE3, pDataOut); break;
                case 0xFFFFFF10: mCore.ReadHardwareRegister(CPU::HardwareRegister::IRQ0, pDataOut); break;
                case 0xFFFFFF11: mCore.ReadHardwareRegister(CPU::HardwareRegister::IRQ1, pDataOut); break;
                case 0xFFFFFF12: mCore.ReadHardwareRegister(CPU::HardwareRegister::IRQ2, pDataOut); break;
                case 0xFFFFFF13: mCore.ReadHardwareRegister(CPU::HardwareRegister::IRQ3, pDataOut); break;
                case 0xFFFFFF20: mCore.ReadHardwareRegister(CPU::HardwareRegister::SPD, pDataOut); break;
                default: break;
            }
        }

        pDataOut = 0xFF;
        return true;
    }

    auto System::Write (std::uint32_t pAddress, std::uint8_t pData) -> bool
    {
        if (pAddress >= 0x80000000 && pAddress < 0x80000000 + kRAMSize)
        {
            mRAM[pAddress - 0x80000000] = pData;
            return true;
        }
        else if (pAddress >= 0xFFFF0000 && pAddress <= 0xFFFFFEFF)
        {
            mQuickRAM[pAddress - 0xFFFF0000] = pData;
            return true;
        }
        else if (pAddress == 0xFFFFFF42)
        {
            if (std::isprint(pData))
                { std::print("{}", static_cast<char>(pData)); }
            else if (pData == '\n')
                { std::println(); }
        }
        else
        {
            switch (pAddress)
            {
                case 0xFFFFFF00: mCore.WriteHardwareRegister(CPU::HardwareRegister::IE0, pData); break;
                case 0xFFFFFF01: mCore.WriteHardwareRegister(CPU::HardwareRegister::IE1, pData); break;
                case 0xFFFFFF02: mCore.WriteHardwareRegister(CPU::HardwareRegister::IE2, pData); break;
                case 0xFFFFFF03: mCore.WriteHardwareRegister(CPU::HardwareRegister::IE3, pData); break;
                case 0xFFFFFF10: mCore.WriteHardwareRegister(CPU::HardwareRegister::IRQ0, pData); break;
                case 0xFFFFFF11: mCore.WriteHardwareRegister(CPU::HardwareRegister::IRQ1, pData); break;
                case 0xFFFFFF12: mCore.WriteHardwareRegister(CPU::HardwareRegister::IRQ2, pData); break;
                case 0xFFFFFF13: mCore.WriteHardwareRegister(CPU::HardwareRegister::IRQ3, pData); break;
                case 0xFFFFFF20: mCore.WriteHardwareRegister(CPU::HardwareRegister::SPD, pData); break;
                default: break;
            }
        }

        return true;
    }

    auto System::Report () -> void
    {
        std::println("CPU Registers:");
        std::uint8_t val8 = 0;
        std::uint16_t val16 = 0;
        std::uint32_t val32 = 0;
        for (std::uint8_t i = 0; i < CPU::kRegisterCount; ++i)
        {
            mCore.ReadRegisterDW(i, val32);
            std::println(" - D{}: 0x{:08X}", i, val32);
        }

        std::println("\nSpecial Registers:");

        mCore.ReadProgramCounter(val32);
        std::println(" - PC: 0x{:08X}", val32);

        mCore.ReadStackPointer(val32);
        std::println(" - SP: 0x{:08X}", val32);

        mCore.ReadFlagsRegister(val8);
        std::println(" - FLAGS: 0b{:B}", val8);

        std::println("\nRAM:");
        for (std::size_t i = 0; i < kRAMSize; ++i)
        {
            if (i % 16 == 15)
                { std::println("0x{:02X}", mRAM[i]); }
            else
                { std::print("0x{:02X}  ", mRAM[i]); }
        }

        std::println();
    }

    auto System::SetProgram (const CPU::Program& pProgram) -> void
    {
        mProgram = pProgram;
        Reset();
    }
}

// Private Methods *************************************************************

namespace G10::Testbed
{
    auto System::InitTime () -> void
    {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        localtime_r(&now_time_t, &mTime);
    }

    auto System::UpdateTime () -> void
    {
        auto oldTime = mTime;
        InitTime();
        if (mTime.tm_sec != oldTime.tm_sec)
        {
            mCore.RequestInterrupt(1);
        }
    }
}
