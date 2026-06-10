/**
 * @file    G10.CPU.Executive.Instructions.cpp
 * @brief   Contains implementations for the G10 CPU Executive class's
 *          instruction execution methods, and related definitions.
 */

#include <G10.CPU.Executive.hpp>

// Private Methods - Instruction Execution *************************************

namespace G10::CPU
{
    auto Executive::ExecuteNOP (Core& pCore, const Instruction& pInst) -> bool
    {
        return true;
    }

    auto Executive::ExecuteSTOP (Core& pCore, const Instruction& pInst) -> bool
    {
        if (pCore.mSpeedRegister.mArmed == true)
        {
            pCore.mSpeedRegister.mArmed = false;
            pCore.mSpeedRegister.mActive = true;

            bool good =
                pCore.AddMachineCycles(2050) &&
                pCore.FlushTickCycles();
            if (good == true)
                { pCore.mSpeedRegister.mHigh = 
                    !pCore.mSpeedRegister.mHigh; }
                
            pCore.mSpeedRegister.mActive = false;
            return good;
        }
        
        pCore.mStopFlag = true;
        pCore.mJustStopped = true;
        if (pCore.mStoppedDelegate != nullptr)
        {
            pCore.mStoppedDelegate(pCore);
        }
        
        return true;
    }

    auto Executive::ExecuteHALT (Core& pCore, const Instruction& pInst) -> bool
    {
        pCore.mHaltFlag = true;
        return true;
    }

    auto Executive::ExecuteDI (Core& pCore, const Instruction& pInst) -> bool
    {
        pCore.mInterruptMasterEnable = false;
        pCore.mInterruptMasterPending = false;
        return true;
    }

    auto Executive::ExecuteEI (Core& pCore, const Instruction& pInst) -> bool
    {
        pCore.mInterruptMasterPending = true;
        return true;
    }

    auto Executive::ExecuteEII (Core& pCore, const Instruction& pInst) -> bool
    {
        pCore.mInterruptMasterEnable = true;
        return true;
    }

    auto Executive::ExecuteDAA (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t    byteAcc = 0,
                        byteAccAdjust = 0,
                        byteAccResult = 0;
        bool            subtract = pCore.mFlagsRegister.mSubtract,
                        halfCarry = pCore.mFlagsRegister.mHalfCarry,
                        carry = pCore.mFlagsRegister.mCarry;

        pCore.ReadRegisterLB(0, byteAcc);
        if (halfCarry == true || (subtract == false && (byteAcc & 0xF) > 0x9))
            { byteAccAdjust |= 0x6; }
        if (carry == true || (subtract == false && byteAcc > 0x99))
            { byteAccAdjust |= 0x60; carry = true; }
        else
            { carry = false; }

        byteAccResult = (subtract == true) ?
            (byteAcc - byteAccAdjust) :
            (byteAcc + byteAccAdjust);

        return
            pCore.WriteRegisterLB(0, byteAccResult) &&
            pCore.WriteFlag(Flag::Zero, (byteAccResult == 0x00)) &&
            pCore.WriteFlag(Flag::HalfCarry, false) &&
            pCore.WriteFlag(Flag::Carry, carry);
    }

    auto Executive::ExecuteSCF (Core& pCore, const Instruction& pInst) -> bool
    {
        return 
            pCore.WriteFlag(Flag::Subtract, false) &&
            pCore.WriteFlag(Flag::HalfCarry, false) &&
            pCore.WriteFlag(Flag::Carry, true);
    }

    auto Executive::ExecuteCCF (Core& pCore, const Instruction& pInst) -> bool
    {
        return 
            pCore.WriteFlag(Flag::Subtract, false) &&
            pCore.WriteFlag(Flag::HalfCarry, false) &&
            pCore.WriteFlag(Flag::Carry, !pCore.mFlagsRegister.mCarry);
    }

    auto Executive::ExecuteCLV (Core& pCore, const Instruction& pInst) -> bool
    {
        return pCore.WriteFlag(Flag::Overflow, false);
    }

    auto Executive::ExecuteSEV (Core& pCore, const Instruction& pInst) -> bool
    {
        return pCore.WriteFlag(Flag::Overflow, true);
    }

    auto Executive::ExecuteREX_XY (Core& pCore, const Instruction& pInst) -> bool
    {
        pCore.RaiseException(pInst.CombinedParam());
        return true;
    }

    auto Executive::ExecuteLEC (Core& pCore, const Instruction& pInst) -> bool
    {
        return pCore.WriteRegisterLB(pInst.mParamX, 
            std::to_underlying(pCore.mException));
    }

    auto Executive::ExecuteLD_LX_IMM8 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t mem = 0;
        return
            pCore.FetchMemoryB(mem) && 
            pCore.WriteRegisterLB(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLD_LX_pIMM32 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t address = 0;
        std::uint8_t mem = 0;
        return
            pCore.FetchMemoryDW(address) &&
            pCore.ReadMemoryB(address, mem) &&
            pCore.WriteRegisterLB(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLD_LX_pDY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t address = 0;
        std::uint8_t mem = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamY, address) &&
            pCore.ReadMemoryB(address, mem) &&
            pCore.WriteRegisterLB(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLDQ_LX_pIMM16 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t address = 0;
        std::uint8_t mem = 0;
        return
            pCore.FetchMemoryW(address) &&
            pCore.ReadMemoryB(kMemQuickRamStartAddr + address, mem) &&
            pCore.WriteRegisterLB(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLDQ_LX_pWY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t address = 0;
        std::uint8_t mem = 0;
        return
            pCore.ReadRegisterW(pInst.mParamY, address) &&
            pCore.ReadMemoryB(kMemQuickRamStartAddr + address, mem) &&
            pCore.WriteRegisterLB(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLDP_LX_pIMM8 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t address = 0;
        std::uint8_t mem = 0;
        return
            pCore.FetchMemoryB(address) &&
            pCore.ReadMemoryB(kMemPortStartAddr + address, mem) &&
            pCore.WriteRegisterLB(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLDP_LX_pLY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t address = 0;
        std::uint8_t mem = 0;
        return
            pCore.ReadRegisterW(pInst.mParamY, address) &&
            pCore.ReadMemoryB(kMemQuickRamStartAddr + address, mem) &&
            pCore.WriteRegisterLB(pInst.mParamX, mem);
    }

    auto Executive::ExecuteST_pIMM32_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t address = 0;
        std::uint8_t mem = 0;
        return
            pCore.FetchMemoryDW(address) &&
            pCore.ReadRegisterLB(pInst.mParamY, mem) &&
            pCore.WriteMemoryB(address, mem);
    }

    auto Executive::ExecuteST_pDX_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t address = 0;
        std::uint8_t mem = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, address) &&
            pCore.ReadRegisterLB(pInst.mParamY, mem) &&
            pCore.WriteMemoryB(address, mem);
    }

    auto Executive::ExecuteSTQ_pIMM16_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t address = 0;
        std::uint8_t mem = 0;
        return
            pCore.FetchMemoryW(address) &&
            pCore.ReadRegisterLB(pInst.mParamY, mem) &&
            pCore.WriteMemoryB(kMemQuickRamStartAddr + address, mem);
    }

    auto Executive::ExecuteSTQ_pWX_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t address = 0;
        std::uint8_t mem = 0;
        return
            pCore.ReadRegisterW(pInst.mParamX, address) &&
            pCore.ReadRegisterLB(pInst.mParamY, mem) &&
            pCore.WriteMemoryB(kMemQuickRamStartAddr + address, mem);
    }

    auto Executive::ExecuteSTP_pIMM8_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t address = 0;
        std::uint8_t mem = 0;
        return
            pCore.FetchMemoryB(address) &&
            pCore.ReadRegisterLB(pInst.mParamY, mem) &&
            pCore.WriteMemoryB(kMemPortStartAddr + address, mem);
    }

    auto Executive::ExecuteSTP_pLX_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t address = 0;
        std::uint8_t mem = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, address) &&
            pCore.ReadRegisterLB(pInst.mParamY, mem) &&
            pCore.WriteMemoryB(kMemPortStartAddr + address, mem);
    }

    auto Executive::ExecuteMV_LX_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t val = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamY, val) &&
            pCore.WriteRegisterLB(pInst.mParamX, val);
    }

    auto Executive::ExecuteMV_HX_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t val = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamY, val) &&
            pCore.WriteRegisterHB(pInst.mParamX, val);
    }

    auto Executive::ExecuteMV_LX_HY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t val = 0;
        return
            pCore.ReadRegisterHB(pInst.mParamY, val) &&
            pCore.WriteRegisterLB(pInst.mParamX, val);
    }

    auto Executive::ExecuteLD_WX_IMM16 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t mem = 0;
        return 
            pCore.FetchMemoryW(mem) &&
            pCore.WriteRegisterW(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLD_WX_pIMM32 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t address = 0;
        std::uint16_t mem = 0;
        return
            pCore.FetchMemoryDW(address) &&
            pCore.ReadMemoryW(address, mem) &&
            pCore.WriteRegisterW(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLD_WX_pDY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t address = 0;
        std::uint16_t mem = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamY, address) &&
            pCore.ReadMemoryW(address, mem) &&
            pCore.WriteRegisterW(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLDQ_WX_pIMM16 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t address = 0;
        std::uint16_t mem = 0;
        return
            pCore.FetchMemoryW(address) &&
            pCore.ReadMemoryW(kMemQuickRamStartAddr + address, mem) &&
            pCore.WriteRegisterW(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLDQ_WX_pWY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t address = 0;
        std::uint16_t mem = 0;
        return
            pCore.ReadRegisterW(pInst.mParamY, address) &&
            pCore.ReadMemoryW(kMemQuickRamStartAddr + address, mem) &&
            pCore.WriteRegisterW(pInst.mParamX, mem);
    }

    auto Executive::ExecuteST_pIMM32_WY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t address = 0;
        std::uint16_t mem = 0;
        return
            pCore.FetchMemoryDW(address) &&
            pCore.ReadRegisterW(pInst.mParamY, mem) &&
            pCore.WriteMemoryW(address, mem);
    }

    auto Executive::ExecuteST_pDX_WY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t address = 0;
        std::uint16_t mem = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, address) &&
            pCore.ReadRegisterW(pInst.mParamY, mem) &&
            pCore.WriteMemoryW(address, mem);
    }

    auto Executive::ExecuteSTQ_pIMM16_WY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t address = 0;
        std::uint16_t mem = 0;
        return
            pCore.FetchMemoryW(address) &&
            pCore.ReadRegisterW(pInst.mParamY, mem) &&
            pCore.WriteMemoryW(kMemQuickRamStartAddr + address, mem);
    }

    auto Executive::ExecuteSTQ_pWX_WY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t address = 0;
        std::uint16_t mem = 0;
        return
            pCore.ReadRegisterW(pInst.mParamX, address) &&
            pCore.ReadRegisterW(pInst.mParamY, mem) &&
            pCore.WriteMemoryW(kMemQuickRamStartAddr + address, mem);
    }

    auto Executive::ExecuteMV_WX_WY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t val = 0;
        return
            pCore.ReadRegisterW(pInst.mParamY, val) &&
            pCore.WriteRegisterW(pInst.mParamX, val);
    }

    auto Executive::ExecuteMWH_DX_WY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t dx = 0;
        std::uint16_t wy = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, dx) &&
            pCore.ReadRegisterW(pInst.mParamY, wy) &&
            pCore.WriteRegisterDW(pInst.mParamX,
                (dx & 0x0000FFFF) | (static_cast<std::uint32_t>(wy) << 16));
    }

    auto Executive::ExecuteMWL_WX_DY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t wx = 0;
        std::uint32_t dy = 0;
        return
            pCore.ReadRegisterW(pInst.mParamX, wx) &&
            pCore.ReadRegisterDW(pInst.mParamY, dy) &&
            pCore.WriteRegisterW(pInst.mParamX, 
                static_cast<std::uint16_t>((dy >> 16) & 0xFFFF));
    }

    auto Executive::ExecuteLD_DX_IMM32 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t mem = 0;
        bool good =  
            pCore.FetchMemoryDW(mem) &&
            pCore.WriteRegisterDW(pInst.mParamX, mem);

        return good;
    }

    auto Executive::ExecuteLD_DX_pIMM32 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t address = 0;
        std::uint32_t mem = 0;
        return
            pCore.FetchMemoryDW(address) &&
            pCore.ReadMemoryDW(address, mem) &&
            pCore.WriteRegisterDW(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLD_DX_pDY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t address = 0;
        std::uint32_t mem = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamY, address) &&
            pCore.ReadMemoryDW(address, mem) &&
            pCore.WriteRegisterDW(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLDQ_DX_pIMM16 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t address = 0;
        std::uint32_t mem = 0;
        return
            pCore.FetchMemoryW(address) &&
            pCore.ReadMemoryDW(kMemQuickRamStartAddr + address, mem) &&
            pCore.WriteRegisterDW(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLDQ_DX_pWY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t address = 0;
        std::uint32_t mem = 0;
        return
            pCore.ReadRegisterW(pInst.mParamY, address) &&
            pCore.ReadMemoryDW(kMemQuickRamStartAddr + address, mem) &&
            pCore.WriteRegisterDW(pInst.mParamX, mem);
    }

    auto Executive::ExecuteLSP_IMM32 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t mem = 0;
        return
            pCore.FetchMemoryDW(mem) &&
            pCore.WriteStackPointer(mem);
    }

    auto Executive::ExecutePOP_DX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t mem = 0;
        return
            pCore.PopStackDW(mem) &&
            pCore.WriteRegisterDW(pInst.mParamX, mem);
    }

    auto Executive::ExecuteST_pIMM32_DY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t address = 0;
        std::uint32_t mem = 0;
        return
            pCore.FetchMemoryDW(address) &&
            pCore.ReadRegisterDW(pInst.mParamY, mem) &&
            pCore.WriteMemoryDW(address, mem);
    }

    auto Executive::ExecuteST_pDX_DY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t address = 0;
        std::uint32_t mem = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, address) &&
            pCore.ReadRegisterDW(pInst.mParamY, mem) &&
            pCore.WriteMemoryDW(address, mem);
    }

    auto Executive::ExecuteSTQ_pIMM16_DY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t address = 0;
        std::uint32_t mem = 0;
        return
            pCore.FetchMemoryW(address) &&
            pCore.ReadRegisterDW(pInst.mParamY, mem) &&
            pCore.WriteMemoryDW(kMemQuickRamStartAddr + address, mem);
    }

    auto Executive::ExecuteSTQ_pWX_DY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t address = 0;
        std::uint32_t mem = 0;
        return
            pCore.ReadRegisterW(pInst.mParamX, address) &&
            pCore.ReadRegisterDW(pInst.mParamY, mem) &&
            pCore.WriteMemoryDW(kMemQuickRamStartAddr + address, mem);
    }

    auto Executive::ExecuteSSP_pIMM32 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t address = 0;
        std::uint32_t sp = 0;
        return
            pCore.FetchMemoryDW(address) &&
            pCore.ReadStackPointer(sp) &&
            pCore.WriteMemoryDW(address, sp);
    }

    auto Executive::ExecutePUSH_DY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t mem = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamY, mem) &&
            pCore.WriteStackPointer(mem);
    }

    auto Executive::ExecuteMV_DX_DY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t val = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamY, val) &&
            pCore.WriteRegisterDW(pInst.mParamX, val);
    }

    auto Executive::ExecuteSPO_DX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t sp = 0;
        return
            pCore.ReadStackPointer(sp) &&
            pCore.WriteRegisterDW(pInst.mParamX, sp);
    }

    auto Executive::ExecuteSPI_DY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t dy = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamY, dy) &&
            pCore.WriteStackPointer(dy);
    }

    auto Executive::ExecuteJMP_X_IMM32 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t mem = 0;
        if (pCore.FetchMemoryDW(mem) == false)
            { return false; }

        bool taken = CheckCondition(pCore, pInst.mParamX);
        if (taken == true)
            { return pCore.WriteProgramCounter(mem); }

        return true;
    }

    auto Executive::ExecuteJMP_X_DY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t dy = 0;
        if (pCore.ReadRegisterDW(pInst.mParamY, dy) == false)
            { return false; }

        bool taken = CheckCondition(pCore, pInst.mParamX);
        if (taken == true)
            { return pCore.WriteProgramCounter(dy); }

        return true;
    }

    auto Executive::ExecuteJPB_X_SIMM16 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t imm16 = 0;
        if (pCore.FetchMemoryW(imm16) == false)
            { return false; }

        bool taken = CheckCondition(pCore, pInst.mParamX);
        if (taken == true)
        {
            std::uint32_t pc = pCore.mProgramCounter;
            std::int16_t simm16 = static_cast<std::int16_t>(imm16);
            return pCore.WriteProgramCounter(pc + simm16);
        }

        return true;
    }

    auto Executive::ExecuteCALL_X_IMM32 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t mem = 0;
        if (pCore.FetchMemoryDW(mem) == false)
            { return false; }

        bool taken = CheckCondition(pCore, pInst.mParamX);
        if (taken == true)
        {
            std::uint32_t pc = pCore.mProgramCounter; 
            return 
                pCore.PushStackDW(pc) &&
                pCore.WriteProgramCounter(mem); 
        }

        return true;
    }

    auto Executive::ExecuteINT_XY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t xy = pInst.CombinedParam();
        if (xy >= kInterruptCount)
        { return pCore.RaiseException(Exception::InvalidArgument); }
        
        std::uint32_t pc = pCore.mProgramCounter;
        return
            pCore.PushStackDW(pc) &&
            pCore.WriteProgramCounter(kMemInterruptStartAddr + (xy * 0x80));
    }

    auto Executive::ExecuteRET_X (Core& pCore, const Instruction& pInst) -> bool
    {
        bool taken = CheckCondition(pCore, pInst.mParamX);
        if (pInst.mParamX > 0)
            { pCore.AddMachineCycles(1); }

        if (taken == true)
        {
            std::uint32_t pc = 0;
            return
                pCore.PopStackDW(pc) &&
                pCore.WriteProgramCounter(pc);
        }

        return true;
    }

    auto Executive::ExecuteRETI (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t pc = 0;
        bool good = 
            pCore.PopStackDW(pc) &&
            pCore.WriteProgramCounter(pc);

        if (good == true)
        {
            pCore.mInterruptMasterEnable = true;
            pCore.mInterruptMasterPending = false;
            pCore.mException = Exception::None;
        }

        return good;
    }

    auto Executive::ExecuteMFI_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t fr = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamY, fr) &&
            pCore.WriteFlagsRegister(fr);
    }

    auto Executive::ExecuteMFO_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t fr = 0;
        return
            pCore.ReadFlagsRegister(fr) &&
            pCore.WriteRegisterLB(pInst.mParamX, fr);
    }

    auto Executive::ExecuteADD_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t imm = 0;
        std::uint8_t res = 0;
        bool good = 
            pCore.FetchMemoryB(imm) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformADD8(pCore, false, acc, imm, res) &&
            pCore.WriteRegisterLB(0, res);

        return good;
    }

    auto Executive::ExecuteADD_L0_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t ly = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamY, ly) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformADD8(pCore, false, acc, ly, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteADD_L0_pDY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t acc = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(0, acc) &&
            pCore.ReadRegisterDW(pInst.mParamY, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformADD8(pCore, false, acc, mem, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteADC_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t imm = 0;
        std::uint8_t res = 0;
        return
            pCore.FetchMemoryB(imm) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformADD8(pCore, true, acc, imm, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteADC_L0_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t ly = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamY, ly) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformADD8(pCore, true, acc, ly, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteADC_L0_pDY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t acc = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(0, acc) &&
            pCore.ReadRegisterDW(pInst.mParamY, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformADD8(pCore, true, acc, mem, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteSUB_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t imm = 0;
        std::uint8_t res = 0;
        return
            pCore.FetchMemoryB(imm) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformSUB8(pCore, false, acc, imm, &res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteSUB_L0_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t ly = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamY, ly) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformSUB8(pCore, false, acc, ly, &res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteSUB_L0_pDY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t acc = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(0, acc) &&
            pCore.ReadRegisterDW(pInst.mParamY, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformSUB8(pCore, false, acc, mem, &res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteSBC_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t imm = 0;
        std::uint8_t res = 0;
        return
            pCore.FetchMemoryB(imm) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformSUB8(pCore, true, acc, imm, &res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteSBC_L0_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t ly = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamY, ly) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformSUB8(pCore, true, acc, ly, &res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteSBC_L0_pDY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t acc = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(0, acc) &&
            pCore.ReadRegisterDW(pInst.mParamY, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformSUB8(pCore, true, acc, mem, &res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteINC_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformINC8(pCore, lx, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteINC_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformINC8(pCore, mem, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteDEC_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformDEC8(pCore, lx, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteDEC_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformDEC8(pCore, mem, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteADD_W0_IMM16 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t acc = 0;
        std::uint16_t imm = 0;
        std::uint16_t res = 0;
        return
            pCore.FetchMemoryW(imm) &&
            pCore.ReadRegisterW(0, acc) &&
            PerformADD16(pCore, acc, imm, res) &&
            pCore.WriteRegisterW(0, res);
    }

    auto Executive::ExecuteADD_W0_WY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t acc = 0;
        std::uint16_t ly = 0;
        std::uint16_t res = 0;
        return
            pCore.ReadRegisterW(pInst.mParamY, ly) &&
            pCore.ReadRegisterW(0, acc) &&
            PerformADD16(pCore, acc, ly, res) &&
            pCore.WriteRegisterW(0, res);
    }

    auto Executive::ExecuteADD_D0_IMM32 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t acc = 0;
        std::uint32_t imm = 0;
        std::uint32_t res = 0;
        return
            pCore.FetchMemoryDW(imm) &&
            pCore.ReadRegisterDW(0, acc) &&
            PerformADD32(pCore, acc, imm, res) &&
            pCore.WriteRegisterDW(0, res);
    }

    auto Executive::ExecuteADD_D0_DY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t acc = 0;
        std::uint32_t ly = 0;
        std::uint32_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamY, ly) &&
            pCore.ReadRegisterDW(0, acc) &&
            PerformADD32(pCore, acc, ly, res) &&
            pCore.WriteRegisterDW(0, res);
    }

    auto Executive::ExecuteSUB_W0_IMM16 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t acc = 0;
        std::uint16_t imm = 0;
        std::uint16_t res = 0;
        return
            pCore.FetchMemoryW(imm) &&
            pCore.ReadRegisterW(0, acc) &&
            PerformSUB16(pCore, acc, imm, res) &&
            pCore.WriteRegisterW(0, res);
    }

    auto Executive::ExecuteSUB_W0_WY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t acc = 0;
        std::uint16_t ly = 0;
        std::uint16_t res = 0;
        return
            pCore.ReadRegisterW(pInst.mParamY, ly) &&
            pCore.ReadRegisterW(0, acc) &&
            PerformSUB16(pCore, acc, ly, res) &&
            pCore.WriteRegisterW(0, res);
    }

    auto Executive::ExecuteSUB_D0_IMM32 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t acc = 0;
        std::uint32_t imm = 0;
        std::uint32_t res = 0;
        return
            pCore.FetchMemoryDW(imm) &&
            pCore.ReadRegisterDW(0, acc) &&
            PerformSUB32(pCore, acc, imm, res) &&
            pCore.WriteRegisterDW(0, res);
    }

    auto Executive::ExecuteSUB_D0_DY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t acc = 0;
        std::uint32_t ly = 0;
        std::uint32_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamY, ly) &&
            pCore.ReadRegisterDW(0, acc) &&
            PerformSUB32(pCore, acc, ly, res) &&
            pCore.WriteRegisterDW(0, res);
    }

    auto Executive::ExecuteINC_WX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t lx = 0;
        std::uint16_t res = 0;
        return
            pCore.ReadRegisterW(pInst.mParamX, lx) &&
            PerformINC16(pCore, lx, res) &&
            pCore.WriteRegisterW(pInst.mParamX, res);
    }

    auto Executive::ExecuteINC_DX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t lx = 0;
        std::uint32_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, lx) &&
            PerformINC32(pCore, lx, res) &&
            pCore.WriteRegisterDW(pInst.mParamX, res);
    }

    auto Executive::ExecuteDEC_WX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t lx = 0;
        std::uint16_t res = 0;
        return
            pCore.ReadRegisterW(pInst.mParamX, lx) &&
            PerformDEC16(pCore, lx, res) &&
            pCore.WriteRegisterW(pInst.mParamX, res);
    }

    auto Executive::ExecuteDEC_DX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t lx = 0;
        std::uint32_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, lx) &&
            PerformDEC32(pCore, lx, res) &&
            pCore.WriteRegisterDW(pInst.mParamX, res);
    }

    auto Executive::ExecuteAND_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t imm = 0;
        std::uint8_t res = 0;
        return
            pCore.FetchMemoryB(imm) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformAND8(pCore, acc, imm, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteAND_L0_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t ly = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamY, ly) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformAND8(pCore, acc, ly, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteAND_L0_pDY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t acc = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(0, acc) &&
            pCore.ReadRegisterDW(pInst.mParamY, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformAND8(pCore, acc, mem, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteOR_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t imm = 0;
        std::uint8_t res = 0;
        return
            pCore.FetchMemoryB(imm) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformOR8(pCore, acc, imm, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteOR_L0_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t ly = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamY, ly) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformOR8(pCore, acc, ly, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteOR_L0_pDY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t acc = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(0, acc) &&
            pCore.ReadRegisterDW(pInst.mParamY, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformOR8(pCore, acc, mem, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteXOR_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t imm = 0;
        std::uint8_t res = 0;
        return
            pCore.FetchMemoryB(imm) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformXOR8(pCore, acc, imm, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteXOR_L0_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t ly = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamY, ly) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformXOR8(pCore, acc, ly, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteXOR_L0_pDY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t acc = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(0, acc) &&
            pCore.ReadRegisterDW(pInst.mParamY, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformXOR8(pCore, acc, mem, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteNOT_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformNOT8(pCore, lx, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteNOT_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformNOT8(pCore, mem, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteCMP_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t imm = 0;
        return
            pCore.FetchMemoryB(imm) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformSUB8(pCore, false, acc, imm, nullptr);
    }

    auto Executive::ExecuteCMP_L0_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t ly = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamY, ly) &&
            pCore.ReadRegisterLB(0, acc) &&
            PerformSUB8(pCore, false, acc, ly, nullptr);
    }

    auto Executive::ExecuteCMP_L0_pDY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t acc = 0;
        std::uint8_t mem = 0;
        return
            pCore.ReadRegisterLB(0, acc) &&
            pCore.ReadRegisterDW(pInst.mParamY, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformSUB8(pCore, false, acc, mem, nullptr);
    }

    auto Executive::ExecuteSLA_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformSHL8(pCore, lx, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteSLA_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformSHL8(pCore, mem, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteSRA_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformSHR8(pCore, false, lx, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteSRA_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformSHR8(pCore, false, mem, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteSRL_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformSHR8(pCore, true, lx, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteSRL_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformSHR8(pCore, true, mem, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteSWAP_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformSWAP8(pCore, lx, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteSWAP_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformSWAP8(pCore, mem, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteSWAP_WX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint16_t lx = 0;
        std::uint16_t res = 0;
        return
            pCore.ReadRegisterW(pInst.mParamX, lx) &&
            PerformSWAP16(pCore, lx, res) &&
            pCore.WriteRegisterW(pInst.mParamX, res);
    }

    auto Executive::ExecuteSWAP_DX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t lx = 0;
        std::uint32_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, lx) &&
            PerformSWAP32(pCore, lx, res) &&
            pCore.WriteRegisterDW(pInst.mParamX, res);
    }

    auto Executive::ExecuteRLA (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(0, acc) &&
            PerformROL8(pCore, true, true, acc, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteRL_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformROL8(pCore, true, false, lx, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteRL_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformROL8(pCore, true, false, mem, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteRLCA (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(0, acc) &&
            PerformROL8(pCore, false, true, acc, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteRLC_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformROL8(pCore, false, false, lx, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteRLC_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformROL8(pCore, false, false, mem, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteRRA (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(0, acc) &&
            PerformROR8(pCore, true, true, acc, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteRR_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformROR8(pCore, true, false, lx, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteRR_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformROR8(pCore, true, false, mem, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteRRCA (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t acc = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(0, acc) &&
            PerformROR8(pCore, false, true, acc, res) &&
            pCore.WriteRegisterLB(0, res);
    }

    auto Executive::ExecuteRRC_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformROR8(pCore, false, false, lx, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteRRC_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformROR8(pCore, false, false, mem, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteBIT_Y_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformBIT8(pCore, lx, pInst.mParamY);
    }

    auto Executive::ExecuteBIT_Y_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformBIT8(pCore, mem, pInst.mParamY);
    }

    auto Executive::ExecuteSET_Y_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformSET8(pCore, lx, pInst.mParamY, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteSET_Y_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformSET8(pCore, mem, pInst.mParamY, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteRES_Y_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformRES8(pCore, lx, pInst.mParamY, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteRES_Y_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformRES8(pCore, mem, pInst.mParamY, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteTOG_Y_LX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint8_t lx = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterLB(pInst.mParamX, lx) &&
            PerformTOG8(pCore, lx, pInst.mParamY, res) &&
            pCore.WriteRegisterLB(pInst.mParamX, res);
    }

    auto Executive::ExecuteTOG_Y_pDX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        std::uint8_t res = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            PerformTOG8(pCore, mem, pInst.mParamY, res) &&
            pCore.WriteMemoryB(addr, res);
    }

    auto Executive::ExecuteLDI_LX_pDY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        bool good =
            pCore.ReadRegisterDW(pInst.mParamY, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            pCore.WriteRegisterLB(pInst.mParamX, mem) &&
            pCore.WriteRegisterDW(pInst.mParamY, addr + 1);
            
        return good;
    }

    auto Executive::ExecuteLDD_LX_pDY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamY, addr) &&
            pCore.ReadMemoryB(addr, mem) &&
            pCore.WriteRegisterLB(pInst.mParamX, mem) &&
            pCore.WriteRegisterDW(pInst.mParamY, addr - 1);
    }

    auto Executive::ExecuteSTI_pDX_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t ly = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadRegisterLB(pInst.mParamY, ly) &&
            pCore.WriteMemoryB(addr, ly) &&
            pCore.WriteRegisterDW(pInst.mParamX, addr + 1);
    }

    auto Executive::ExecuteSTD_pDX_LY (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t ly = 0;
        return
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.ReadRegisterLB(pInst.mParamY, ly) &&
            pCore.WriteMemoryB(addr, ly) &&
            pCore.WriteRegisterDW(pInst.mParamX, addr - 1);
    }

    auto Executive::ExecuteASP_SIMM8 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t sp = 0;
        std::uint8_t imm8 = 0;
        bool good =
            pCore.ReadStackPointer(sp) &&
            pCore.FetchMemoryB(imm8);
        if (good == false)
            { return false; }

        std::int8_t simm8 = static_cast<std::int8_t>(imm8);
        std::uint32_t result = sp + simm8;
        std::uint16_t carrySum = static_cast<std::uint16_t>((sp & 0xFF) + imm8);
        std::uint8_t halfCarrySum = static_cast<std::uint8_t>((sp & 0x0F) + (imm8 & 0x0F));
        bool overflowCheck =
            ((sp ^ static_cast<std::uint32_t>(simm8) ^ 0x80) & (sp ^ result) & 0x80) != 0;

        return
            pCore.WriteStackPointer(result) &&
            pCore.WriteFlag(Flag::Zero, false) &&
            pCore.WriteFlag(Flag::Subtract, false) &&
            pCore.WriteFlag(Flag::HalfCarry, halfCarrySum > 0x0F) &&
            pCore.WriteFlag(Flag::Carry, carrySum > 0xFF) &&
            pCore.WriteFlag(Flag::Overflow, overflowCheck);
    }

    auto Executive::ExecuteST_pDX_IMM8 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t addr = 0;
        std::uint8_t mem = 0;
        return
            pCore.FetchMemoryB(mem) &&
            pCore.ReadRegisterDW(pInst.mParamX, addr) &&
            pCore.WriteMemoryB(addr, mem);
    }

    auto Executive::ExecuteLASP_DX_SIMM8 (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t sp = 0;
        std::uint8_t imm8 = 0;
        bool good =
            pCore.ReadStackPointer(sp) &&
            pCore.FetchMemoryB(imm8);
        if (good == false)
            { return false; }

        std::int8_t simm8 = static_cast<std::int8_t>(imm8);
        std::uint32_t result = sp + simm8;
        std::uint16_t carrySum = static_cast<std::uint16_t>((sp & 0xFF) + imm8);
        std::uint8_t halfCarrySum = static_cast<std::uint8_t>((sp & 0x0F) + (imm8 & 0x0F));
        bool overflowCheck =
            ((sp ^ static_cast<std::uint32_t>(simm8) ^ 0x80) & (sp ^ result) & 0x80) != 0;

        return
            pCore.AddMachineCycles(1) &&
            pCore.WriteRegisterDW(pInst.mParamX, result) &&
            pCore.WriteFlag(Flag::Zero, false) &&
            pCore.WriteFlag(Flag::Subtract, false) &&
            pCore.WriteFlag(Flag::HalfCarry, halfCarrySum > 0x0F) &&
            pCore.WriteFlag(Flag::Carry, carrySum > 0xFF) &&
            pCore.WriteFlag(Flag::Overflow, overflowCheck);
    }

    auto Executive::ExecuteISP (Core& pCore, const Instruction& pInst) -> bool
    {
        return pCore.WriteStackPointer(pCore.mStackPointer + 1);
    }

    auto Executive::ExecuteDSP (Core& pCore, const Instruction& pInst) -> bool
    {
        return pCore.WriteStackPointer(pCore.mStackPointer - 1);
    }

    auto Executive::ExecuteASR_DX (Core& pCore, const Instruction& pInst) -> bool
    {
        std::uint32_t sp = 0;
        std::uint32_t dx = 0;
        std::uint32_t res = 0;
        return
            pCore.ReadStackPointer(sp) &&
            pCore.ReadRegisterDW(pInst.mParamX, dx) &&
            PerformADD32(pCore, sp, dx, res) &&
            pCore.WriteRegisterDW(pInst.mParamX, res);
    }
}
