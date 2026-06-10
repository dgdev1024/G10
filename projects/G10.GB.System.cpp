/**
 * @file    G10.GB.System.cpp
 * @brief   Contains implementations for the G10.Boy's system bus class,
 *          and related definitions.
 */

// Includes ********************************************************************

#include <G10.GB.System.hpp>

// Public - Constructors & Destructors *****************************************

namespace G10::GB
{
    System::System () :
        CPU::IBus   {},
        mProgram    { std::nullopt },
        mRAM        { *this },
        mTimer      { *this },
        mSerial     { *this },
        mJoypad     { *this },
        mPPU        { *this },
        mAPU        { *this },
        mCore       { *this }
    {
        mCore.SetStoppedDelegate(std::bind(&System::OnCoreStopInstruction, this));
    }
}

// Public Methods - From CPU::IBus *********************************************

namespace G10::GB
{
    auto System::Reset () -> void
    {
        mCore.Reset();
        mRAM.Reset();
        mTimer.Reset();
        mSerial.Reset();
        mJoypad.Reset();
        mPPU.Reset();
        mAPU.Reset();
    }

    auto System::Clock (const std::uint64_t& pCycle) -> bool
    {
        return
            mTimer.Clock(pCycle) &&
            mSerial.Clock(pCycle) &&
            mPPU.Clock(pCycle) &&
            mAPU.Clock(pCycle);
    }

    auto System::Read (std::uint32_t pAddress, std::uint8_t& pDataOut) -> bool
    {
        if (pAddress <= kMemProgramEndAddr)
        {
            if (mProgram.has_value() == true &&
                mProgram->Read(pAddress, pDataOut) == true)
                { return true; }

            pDataOut = (pAddress <= CPU::kMemMetadataEndAddr) ? 0x00 : 0xFF;
        }
        else if (pAddress >= kMemVideoRamStartAddr && 
                 pAddress <= kMemVideoRamEndAddr)
        {
            if (mPPU.ReadVideoRAM(pAddress - kMemVideoRamStartAddr, pDataOut) == false)
                { pDataOut = 0xFF; }
        }
        else if (pAddress >= kMemOamStartAddr &&
                 pAddress <= kMemOamEndAddr)
        {
            if (mPPU.ReadOAM(pAddress - kMemOamStartAddr, pDataOut) == false)
                { pDataOut = 0xFF; }
        }
        else if (pAddress >= kMemWaveRamStartAddr &&
                 pAddress <= kMemWaveRamEndAddr)
        {
            if (mAPU.ReadWaveRAM(pAddress - kMemWaveRamStartAddr, pDataOut) == false)
                { pDataOut = 0xFF; }
        }
        else if (pAddress >= kMemWorkRamStartAddr &&
                 pAddress <= kMemWorkRamEndAddr)
        {
            if (mRAM.ReadWorkRAM(pAddress - kMemWorkRamStartAddr, pDataOut) == false)
                { pDataOut = 0xFF; }
        }
        else if (pAddress >= kMemSaveRamStartAddr &&
                 pAddress <= kMemSaveRamEndAddr)
        {
            if (mRAM.ReadExternalRAM(pAddress - kMemSaveRamStartAddr, pDataOut) == false)
                { pDataOut = 0xFF; }
        }
        else if (pAddress >= kMemExpansionRamStartAddr &&
                 pAddress <= kMemExpansionRamEndAddr)
        {
            // Handle expansion RAM read
        }
        else if (pAddress >= kMemQuickRamStartAddr &&
                 pAddress <= kMemQuickRamEndAddr)
        {
            if (mRAM.ReadQuickRAM(pAddress - kMemQuickRamStartAddr, pDataOut) == false)
                { pDataOut = 0xFF; }
        }
        else switch (static_cast<PortRegister>(pAddress))
        {
            case PortRegister::IE0:     return mCore.ReadHardwareRegister(CPU::HardwareRegister::IE0, pDataOut);
            case PortRegister::IE1:     return mCore.ReadHardwareRegister(CPU::HardwareRegister::IE1, pDataOut);
            case PortRegister::IE2:     return mCore.ReadHardwareRegister(CPU::HardwareRegister::IE2, pDataOut);
            case PortRegister::IE3:     return mCore.ReadHardwareRegister(CPU::HardwareRegister::IE3, pDataOut);
            case PortRegister::IRQ0:    return mCore.ReadHardwareRegister(CPU::HardwareRegister::IRQ0, pDataOut);
            case PortRegister::IRQ1:    return mCore.ReadHardwareRegister(CPU::HardwareRegister::IRQ1, pDataOut);
            case PortRegister::IRQ2:    return mCore.ReadHardwareRegister(CPU::HardwareRegister::IRQ2, pDataOut);
            case PortRegister::IRQ3:    return mCore.ReadHardwareRegister(CPU::HardwareRegister::IRQ3, pDataOut);
            case PortRegister::SPD:     return mCore.ReadHardwareRegister(CPU::HardwareRegister::SPD, pDataOut);
            case PortRegister::DIV:     return mTimer.ReadDIV(pDataOut);
            case PortRegister::TIMA:    return mTimer.ReadTIMA(pDataOut);
            case PortRegister::TMA:     return mTimer.ReadTMA(pDataOut);
            case PortRegister::TAC:     return mTimer.ReadTAC(pDataOut);
            case PortRegister::SB:      return mSerial.ReadSB(pDataOut);
            case PortRegister::SC:      return mSerial.ReadSC(pDataOut);
            case PortRegister::RP:      return true;
            case PortRegister::NR10:    return mAPU.ReadNR10(pDataOut);
            case PortRegister::NR11:    return mAPU.ReadNR11(pDataOut);
            case PortRegister::NR12:    return mAPU.ReadNR12(pDataOut);
            case PortRegister::NR13:    return true;
            case PortRegister::NR14:    return mAPU.ReadNR14(pDataOut);
            case PortRegister::NR21:    return mAPU.ReadNR21(pDataOut);
            case PortRegister::NR22:    return mAPU.ReadNR22(pDataOut);
            case PortRegister::NR23:    return true;
            case PortRegister::NR24:    return mAPU.ReadNR24(pDataOut);
            case PortRegister::NR30:    return mAPU.ReadNR30(pDataOut);
            case PortRegister::NR31:    return true;
            case PortRegister::NR32:    return mAPU.ReadNR32(pDataOut);
            case PortRegister::NR33:    return true;
            case PortRegister::NR34:    return mAPU.ReadNR34(pDataOut);
            case PortRegister::NR41:    return true;
            case PortRegister::NR42:    return mAPU.ReadNR42(pDataOut);
            case PortRegister::NR43:    return mAPU.ReadNR43(pDataOut);
            case PortRegister::NR44:    return mAPU.ReadNR44(pDataOut);
            case PortRegister::NR50:    return mAPU.ReadNR50(pDataOut);
            case PortRegister::NR51:    return mAPU.ReadNR51(pDataOut);
            case PortRegister::NR52:    return mAPU.ReadNR52(pDataOut);
            case PortRegister::PCM12:   return mAPU.ReadPCM12(pDataOut);
            case PortRegister::PCM34:   return mAPU.ReadPCM34(pDataOut);
            case PortRegister::LCDC:    return mPPU.ReadLCDC(pDataOut);
            case PortRegister::STAT:    return mPPU.ReadSTAT(pDataOut);
            case PortRegister::SCY:     return mPPU.ReadSCY(pDataOut);
            case PortRegister::SCX:     return mPPU.ReadSCX(pDataOut);
            case PortRegister::WY:      return mPPU.ReadWY(pDataOut);
            case PortRegister::WX:      return mPPU.ReadWX(pDataOut);
            case PortRegister::LY:      return mPPU.ReadLY(pDataOut);
            case PortRegister::LYC:     return mPPU.ReadLYC(pDataOut);
            case PortRegister::BGP:     return mPPU.ReadBGP(pDataOut);
            case PortRegister::OBP0:    return mPPU.ReadOBP0(pDataOut);
            case PortRegister::OBP1:    return mPPU.ReadOBP1(pDataOut);
            case PortRegister::BGPI:    return mPPU.ReadBGPI(pDataOut);
            case PortRegister::BGPD:    return mPPU.ReadBGPD(pDataOut);
            case PortRegister::OBPI:    return mPPU.ReadOBPI(pDataOut);
            case PortRegister::OBPD:    return mPPU.ReadOBPD(pDataOut);
            case PortRegister::PPUX:    return mPPU.ReadPPUX(pDataOut);
            case PortRegister::ODMAS:   return mPPU.ReadODMAS(pDataOut);
            case PortRegister::VDMA6:   return mPPU.ReadVDMA6(pDataOut);
            case PortRegister::JOYP:    return mJoypad.ReadJOYP(pDataOut);
            default:                    pDataOut = 0xFF; break;
        }

        return true;
    }

    auto System::Write (std::uint32_t pAddress, std::uint8_t pDataIn) -> bool
    {
        if (pAddress >= kMemVideoRamStartAddr && 
            pAddress <= kMemVideoRamEndAddr)
        {
            mPPU.WriteVideoRAM(pAddress - kMemVideoRamStartAddr, pDataIn);
        }
        else if (pAddress >= kMemOamStartAddr &&
                 pAddress <= kMemOamEndAddr)
        {
            mPPU.WriteOAM(pAddress - kMemOamStartAddr, pDataIn);
        }
        else if (pAddress >= kMemWaveRamStartAddr &&
                 pAddress <= kMemWaveRamEndAddr)
            { mAPU.WriteWaveRAM(pAddress - kMemWaveRamStartAddr, pDataIn); }
        else if (pAddress >= kMemWorkRamStartAddr &&
                 pAddress <= kMemWorkRamEndAddr)
            { mRAM.WriteWorkRAM(pAddress - kMemWorkRamStartAddr, pDataIn); }
        else if (pAddress >= kMemSaveRamStartAddr &&
                 pAddress <= kMemSaveRamEndAddr)
            { mRAM.WriteExternalRAM(pAddress - kMemSaveRamStartAddr, pDataIn); }
        else if (pAddress >= kMemExpansionRamStartAddr &&
                 pAddress <= kMemExpansionRamEndAddr)
        {
            // Handle expansion RAM write
        }
        else if (pAddress >= kMemQuickRamStartAddr &&
                 pAddress <= kMemQuickRamEndAddr)
            { mRAM.WriteQuickRAM(pAddress - kMemQuickRamStartAddr, pDataIn); }
        else switch (static_cast<PortRegister>(pAddress))
        {
            case PortRegister::IE0:     return mCore.WriteHardwareRegister(CPU::HardwareRegister::IE0, pDataIn);
            case PortRegister::IE1:     return mCore.WriteHardwareRegister(CPU::HardwareRegister::IE1, pDataIn);
            case PortRegister::IE2:     return mCore.WriteHardwareRegister(CPU::HardwareRegister::IE2, pDataIn);
            case PortRegister::IE3:     return mCore.WriteHardwareRegister(CPU::HardwareRegister::IE3, pDataIn);
            case PortRegister::IRQ0:    return mCore.WriteHardwareRegister(CPU::HardwareRegister::IRQ0, pDataIn);
            case PortRegister::IRQ1:    return mCore.WriteHardwareRegister(CPU::HardwareRegister::IRQ1, pDataIn);
            case PortRegister::IRQ2:    return mCore.WriteHardwareRegister(CPU::HardwareRegister::IRQ2, pDataIn);
            case PortRegister::IRQ3:    return mCore.WriteHardwareRegister(CPU::HardwareRegister::IRQ3, pDataIn);
            case PortRegister::SPD:     return mCore.WriteHardwareRegister(CPU::HardwareRegister::SPD, pDataIn);
            case PortRegister::DIV:     return mTimer.WriteDIV();
            case PortRegister::TIMA:    return mTimer.WriteTIMA(pDataIn);
            case PortRegister::TMA:     return mTimer.WriteTMA(pDataIn);
            case PortRegister::TAC:     return mTimer.WriteTAC(pDataIn);
            case PortRegister::SB:      return mSerial.WriteSB(pDataIn);
            case PortRegister::SC:      return mSerial.WriteSC(pDataIn);
            case PortRegister::RP:      return true;
            case PortRegister::NR10:    return mAPU.WriteNR10(pDataIn);
            case PortRegister::NR11:    return mAPU.WriteNR11(pDataIn);
            case PortRegister::NR12:    return mAPU.WriteNR12(pDataIn);
            case PortRegister::NR13:    return mAPU.WriteNR13(pDataIn);
            case PortRegister::NR14:    return mAPU.WriteNR14(pDataIn);
            case PortRegister::NR21:    return mAPU.WriteNR21(pDataIn);
            case PortRegister::NR22:    return mAPU.WriteNR22(pDataIn);
            case PortRegister::NR23:    return mAPU.WriteNR23(pDataIn);
            case PortRegister::NR24:    return mAPU.WriteNR24(pDataIn);
            case PortRegister::NR30:    return mAPU.WriteNR30(pDataIn);
            case PortRegister::NR31:    return mAPU.WriteNR31(pDataIn);
            case PortRegister::NR32:    return mAPU.WriteNR32(pDataIn);
            case PortRegister::NR33:    return mAPU.WriteNR33(pDataIn);
            case PortRegister::NR34:    return mAPU.WriteNR34(pDataIn);
            case PortRegister::NR41:    return mAPU.WriteNR41(pDataIn);
            case PortRegister::NR42:    return mAPU.WriteNR42(pDataIn);
            case PortRegister::NR43:    return mAPU.WriteNR43(pDataIn);
            case PortRegister::NR44:    return mAPU.WriteNR44(pDataIn);
            case PortRegister::NR50:    return mAPU.WriteNR50(pDataIn);
            case PortRegister::NR51:    return mAPU.WriteNR51(pDataIn);
            case PortRegister::NR52:    return mAPU.WriteNR52(pDataIn);
            case PortRegister::PCM12:   return true;
            case PortRegister::PCM34:   return true;
            case PortRegister::LCDC:    return mPPU.WriteLCDC(pDataIn);
            case PortRegister::STAT:    return mPPU.WriteSTAT(pDataIn);
            case PortRegister::SCY:     return mPPU.WriteSCY(pDataIn);
            case PortRegister::SCX:     return mPPU.WriteSCX(pDataIn);
            case PortRegister::WY:      return mPPU.WriteWY(pDataIn);
            case PortRegister::WX:      return mPPU.WriteWX(pDataIn);
            case PortRegister::LYC:     return mPPU.WriteLYC(pDataIn);
            case PortRegister::BGP:     return mPPU.WriteBGP(pDataIn);
            case PortRegister::OBP0:    return mPPU.WriteOBP0(pDataIn);
            case PortRegister::OBP1:    return mPPU.WriteOBP1(pDataIn);
            case PortRegister::BGPI:    return mPPU.WriteBGPI(pDataIn);
            case PortRegister::BGPD:    return mPPU.WriteBGPD(pDataIn);
            case PortRegister::OBPI:    return mPPU.WriteOBPI(pDataIn);
            case PortRegister::OBPD:    return mPPU.WriteOBPD(pDataIn);
            case PortRegister::PPUX:    return mPPU.WritePPUX(pDataIn);
            case PortRegister::ODMAS:   return mPPU.WriteODMAS();
            case PortRegister::ODMA1:   return mPPU.WriteODMA1(pDataIn);
            case PortRegister::ODMA2:   return mPPU.WriteODMA2(pDataIn);
            case PortRegister::ODMA3:   return mPPU.WriteODMA3(pDataIn);
            case PortRegister::VDMA0:   return mPPU.WriteVDMA0(pDataIn);
            case PortRegister::VDMA1:   return mPPU.WriteVDMA1(pDataIn);
            case PortRegister::VDMA2:   return mPPU.WriteVDMA2(pDataIn);
            case PortRegister::VDMA3:   return mPPU.WriteVDMA3(pDataIn);
            case PortRegister::VDMA4:   return mPPU.WriteVDMA4(pDataIn);
            case PortRegister::VDMA5:   return mPPU.WriteVDMA5(pDataIn);
            case PortRegister::VDMA6:   return mPPU.WriteVDMA6(pDataIn);
            case PortRegister::JOYP:    return mJoypad.WriteJOYP(pDataIn);
            default:                    break;
        }

        return true;
    }
}

// Public Methods **************************************************************

namespace G10::GB
{
    auto System::Step () -> bool
    {
        if (mProgram.has_value() == true)
        {
            return CPU::Executive::StepCore(mCore);
        }

        return false;
    }

    auto System::StepFrame () -> bool
    {
        // This function will call `Step` on a loop until the PPU component
        // enters a vertical blanking period.
        // while (mPPU.mFrameJustFinished == false &&
        //        mCore.IsStopped() == false)
        // {
        //     if (Step() == false)
        //         { return false; }
        // }

        if (mCore.IsStopped() == false)
        {
            do 
            {
                if (Step() == false)
                    { return false; }
            } 
            while ( mCore.IsStopped() == false &&
                    mPPU.AcknowledgeFrame() == false);
        }

        return true;
    }

    auto System::ValidateProgram (CPU::Program& pProgram) -> stx::expect<ProgramMetadata>
    {
        return PrepareProgram(pProgram);
    }

    auto System::SetProgram (CPU::Program& pProgram) -> stx::expect_good
    {
        auto metadata = ValidateProgram(pProgram);
        if (metadata.has_value() == false)
        {
            return stx::fmt_unexpected("Failed to prepare program: {}", 
                metadata.error());
        }

        mProgram = pProgram;
        mProgramMetadata = std::move(*metadata);
        Reset();
        mCore.SetProgramCounter(mProgram->GetEntryPoint());
        return {};
    }

    auto System::ClearProgram () -> void
    {
        mProgram.reset();
        mProgramMetadata.Reset();
    }
}

// Private Methods *************************************************************

namespace G10::GB
{
    auto System::OnCoreStopInstruction () -> void
    {
        mTimer.NotifyStopEvent();
    }

    auto System::PrepareProgram (CPU::Program& pProgram) 
        -> stx::expect<ProgramMetadata>
    {
        ProgramMetadata metadata {};
        stx::byte_view view { pProgram.GetMetadataBuffer() };

        // Offset `$0000`: Magic String
        view.set_read_ptr(0x0000);
        metadata.mTitle = view.read_string();
        if (metadata.mTitle != "G10.BOY")
        {
            return stx::fmt_unexpected("Magic string is missing or incorrect.");
        }

        // Offset `$0008`: Requested WRAM Size (UInt32)
        view.set_read_ptr(0x0008);
        metadata.mRequestedWramSize = view.read_word_le();
        if (
            metadata.mRequestedWramSize < kMinWramSize ||
            metadata.mRequestedWramSize > kMaxWramSize
        )
        {
            return stx::fmt_unexpected(
                "Requested WRAM size is out of bounds. "
                "Expected between {} and {} bytes, got {}.",
                kMinWramSize,
                kMaxWramSize,
                metadata.mRequestedWramSize);
        }

        // Offset `$000C`: Requested SRAM Size (UInt32, Optional)
        view.set_read_ptr(0x000C);
        metadata.mRequestedSramSize = view.read_word_le();
        if (
            metadata.mRequestedSramSize != 0 && (
                metadata.mRequestedSramSize < kMinSramSize || 
                metadata.mRequestedSramSize > kMaxSramSize)
        )
        {
            return stx::fmt_unexpected(
                "Requested SRAM size is out of bounds. "
                "Expected between {} and {} bytes, got {}.",
                kMinSramSize,
                kMaxSramSize,
                metadata.mRequestedSramSize);
        }

        // Offset `$0010`: Program Title (String, Max 32 Bytes Including Null Terminator)
        view.set_read_ptr(0x0010);
        metadata.mTitle = view.read_string();
        if (metadata.mTitle.empty())
        {
            return stx::fmt_unexpected("Program title string is missing.");
        }
        else if (metadata.mTitle.length() >= 32)
        {
            return stx::fmt_unexpected(
                "Program title string is too long. "
                "Expected at most 31 characters, got {}.",
                metadata.mTitle.length()
            );
        }

        // Offset `$0030`: Program Author (String, Max 32 Bytes Including Null Terminator)
        view.set_read_ptr(0x0030);
        metadata.mAuthor = view.read_string();
        if (metadata.mAuthor.length() >= 32)
        {
            return stx::fmt_unexpected(
                "Program author string is too long. "
                "Expected at most 31 characters, got {}.",
                metadata.mAuthor.length()
            );
        }

        // Offset `$0050`: Program Description (String, Max 64 Bytes Including Null Terminator)
        view.set_read_ptr(0x0050);
        metadata.mDescription = view.read_string();
        if (metadata.mDescription.length() >= 64)
        {
            return stx::fmt_unexpected(
                "Program description string is too long. "
                "Expected at most 63 characters, got {}.",
                metadata.mDescription.length()
            );
        }

        view.set_read_ptr(0x0070);
        metadata.mCGBMode = (view.read_byte() != 0);

        return metadata;
    }
}
