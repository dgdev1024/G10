/**
 * @file    G10.CPU.Executive.cpp
 * @brief   Contains implementations for the G10 CPU Executive class, and 
 *          related definitions.
 */

// Includes ********************************************************************

#include <G10.CPU.Executive.hpp>

// Public Methods **************************************************************

namespace G10::CPU
{
    auto Executive::StepCore (Core& pCore) -> bool
    {
        pCore.mJustStopped = false;

        if (pCore.mStopFlag == true || pCore.mDoubleFaultFlag == true)
            { return true; }

        if (pCore.FlushTickCycles() == false)
            { return false; }

        if (pCore.mHaltFlag == true)
        {
            if (pCore.mInterruptEnableRegister == 0)
                { return pCore.RaiseException(Exception::WatchdogTimeout); }

            bool anyIntPending = (
                pCore.mInterruptEnableRegister &
                pCore.mInterruptRequestRegister
            ) != 0;
            if (anyIntPending == true)
                { pCore.mHaltFlag = false; }
            else
                { return pCore.AddMachineCycles(1); }
        }

        std::uint8_t servicedLine = 0xFF;
        if (ServiceInterrupt(pCore, servicedLine) == false)
            { return false; }
        else if (servicedLine != 0xFF && pCore.mInterruptServiceCallback != nullptr)
            { pCore.mInterruptServiceCallback(pCore, servicedLine); }

        return DecodeNextInstruction(pCore);
    }
}

// Private Methods - Lifecycle *************************************************

namespace G10::CPU
{
    auto Executive::ServiceInterrupt (Core& pCore, std::uint8_t& pServicedLine) -> bool
    {
        if (pCore.mInterruptMasterEnable == false)
        {
            if (pCore.mExceptionPending != Exception::None)
                { return true; }
        }
        else if (pCore.mInterruptMasterPending == true)
        {
            pCore.mInterruptMasterPending = false;
            pCore.mInterruptMasterEnable = true;
            if (pCore.mExceptionPending != Exception::None)
                { return true; }
        }

        // Check for exceptions first (highest priority)
        if (pCore.mExceptionPending != Exception::None)
        {
            bool doubleFaultCondition = (pCore.mException != Exception::None);

            pCore.mException = pCore.mExceptionPending;
            pCore.mExceptionPending = Exception::None;
            if (doubleFaultCondition == true)
            {
                pCore.mDoubleFaultFlag = true;
                pCore.mStopFlag = true;
                pCore.mJustStopped = true;
                if (pCore.mStoppedDelegate != nullptr)
                {
                    pCore.mStoppedDelegate(pCore);
                }

                return false;
            }

            pCore.mInterruptMasterEnable = false;
            pCore.mHaltFlag = false;

            bool good =
                pCore.AddMachineCycles(2) &&
                pCore.PushStackDW(pCore.mProgramCounter) &&
                pCore.WriteProgramCounter(kMemInterruptStartAddr);
            if (good == false)
            {
                pCore.mDoubleFaultFlag = true;
                pCore.mStopFlag = true;
                pCore.mJustStopped = true;
                if (pCore.mStoppedDelegate != nullptr)
                {
                    pCore.mStoppedDelegate(pCore);
                }
                    
                return false;
            }

            pServicedLine = 0;
            return good;
        }

        // Check for pending interrupts using CTZ (count trailing zeros) for O(1) discovery
        std::uint32_t pending = pCore.mInterruptEnableRegister & 
                               pCore.mInterruptRequestRegister;
        if (pending != 0)
        {
            // Find the first set bit in O(1) time using count trailing zeros
            std::uint8_t i = std::countr_zero(pending);
            
            pCore.mInterruptRequestRegister &= ~(1 << i);
            pCore.mInterruptMasterEnable = false;
            pCore.mHaltFlag = false;

            bool good =
                pCore.AddMachineCycles(2) &&
                pCore.PushStackDW(pCore.mProgramCounter) &&
                pCore.WriteProgramCounter(kMemInterruptStartAddr + (i * 0x80));
            if (good == false)
            {
                pCore.mDoubleFaultFlag = true;
                pCore.mStopFlag = true;
                pCore.mJustStopped = true;
                if (pCore.mStoppedDelegate != nullptr)
                {
                    pCore.mStoppedDelegate(pCore);
                }
                    
                return false;
            }

            pServicedLine = i;
            return good;
        }

        return true;
    }

    auto Executive::DecodeNextInstruction (Core& pCore) -> bool
    {
        // Fetch and decode the instruction...
        {
            bool good = true;
            std::uint16_t raw = 0xFFFF;

            pCore.mInstructionPointer = pCore.mProgramCounter;
            good = pCore.FetchMemoryW(raw, true);
            pCore.mInstructionRegister = raw;
            pCore.mImmediateData = raw;

            // debug("IP = ${:08X} | OP = 0x{:02X} | PX = 0x{:01X} | PY = 0x{:01X}",
            //     pCore.mInstructionPointer, 
            //     pCore.mInstructionRegister.mOpcode,
            //     pCore.mInstructionRegister.mParamX,
            //     pCore.mInstructionRegister.mParamY);

            if (good == false)
                { return false; }
        }

        // Delegate the instruction for custom handling...
        if (pCore.mInstructionDecodeDelegate != nullptr)
        {
            bool consumed = pCore.mInstructionDecodeDelegate(pCore, 
                pCore.mInstructionRegister);
            if (consumed == true)
            {
                if (pCore.mInstructionExecuteCallback != nullptr)
                {
                    pCore.mInstructionExecuteCallback(pCore,
                        pCore.mInstructionRegister);
                }

                return true;
            }
        }

        // Instruction Table
        using Executor = bool (*) (Core&, const Instruction&);
        static constexpr Executor kExecutors[] = {
            /* 0x00 */ Executive::ExecuteNOP,
            /* 0x01 */ Executive::ExecuteSTOP,
            /* 0x02 */ Executive::ExecuteHALT,
            /* 0x03 */ Executive::ExecuteDI,
            /* 0x04 */ Executive::ExecuteEI,
            /* 0x05 */ Executive::ExecuteEII,
            /* 0x06 */ Executive::ExecuteDAA,
            /* 0x07 */ Executive::ExecuteSCF,
            /* 0x08 */ Executive::ExecuteCCF,
            /* 0x09 */ Executive::ExecuteCLV,
            /* 0x0A */ Executive::ExecuteSEV,
            /* 0x0B */ Executive::ExecuteREX_XY,
            /* 0x0C */ Executive::ExecuteLEC,
            /* 0x0D */ nullptr,
            /* 0x0E */ nullptr,
            /* 0x0F */ nullptr,
            /* 0x10 */ Executive::ExecuteLD_LX_IMM8,
            /* 0x11 */ Executive::ExecuteLD_LX_pIMM32,
            /* 0x12 */ Executive::ExecuteLD_LX_pDY,
            /* 0x13 */ Executive::ExecuteLDQ_LX_pIMM16,
            /* 0x14 */ Executive::ExecuteLDQ_LX_pWY,
            /* 0x15 */ Executive::ExecuteLDP_LX_pIMM8,
            /* 0x16 */ Executive::ExecuteLDP_LX_pLY,
            /* 0x17 */ Executive::ExecuteST_pIMM32_LY,
            /* 0x18 */ Executive::ExecuteST_pDX_LY,
            /* 0x19 */ Executive::ExecuteSTQ_pIMM16_LY,
            /* 0x1A */ Executive::ExecuteSTQ_pWX_LY,
            /* 0x1B */ Executive::ExecuteSTP_pIMM8_LY,
            /* 0x1C */ Executive::ExecuteSTP_pLX_LY,
            /* 0x1D */ Executive::ExecuteMV_LX_LY,
            /* 0x1E */ Executive::ExecuteMV_HX_LY,
            /* 0x1F */ Executive::ExecuteMV_LX_HY,
            /* 0x20 */ Executive::ExecuteLD_WX_IMM16,
            /* 0x21 */ Executive::ExecuteLD_WX_pIMM32,
            /* 0x22 */ Executive::ExecuteLD_WX_pDY,
            /* 0x23 */ Executive::ExecuteLDQ_WX_pIMM16,
            /* 0x24 */ Executive::ExecuteLDQ_WX_pWY,
            /* 0x25 */ nullptr,
            /* 0x26 */ nullptr,
            /* 0x27 */ Executive::ExecuteST_pIMM32_WY,
            /* 0x28 */ Executive::ExecuteST_pDX_WY,
            /* 0x29 */ Executive::ExecuteSTQ_pIMM16_WY,
            /* 0x2A */ Executive::ExecuteSTQ_pWX_WY,
            /* 0x2B */ nullptr,
            /* 0x2C */ nullptr,
            /* 0x2D */ Executive::ExecuteMV_WX_WY,
            /* 0x2E */ Executive::ExecuteMWH_DX_WY,
            /* 0x2F */ Executive::ExecuteMWL_WX_DY,
            /* 0x30 */ Executive::ExecuteLD_DX_IMM32,
            /* 0x31 */ Executive::ExecuteLD_DX_pIMM32,
            /* 0x32 */ Executive::ExecuteLD_DX_pDY,
            /* 0x33 */ Executive::ExecuteLDQ_DX_pIMM16,
            /* 0x34 */ Executive::ExecuteLDQ_DX_pWY,
            /* 0x35 */ Executive::ExecuteLSP_IMM32,
            /* 0x36 */ Executive::ExecutePOP_DX,
            /* 0x37 */ Executive::ExecuteST_pIMM32_DY,
            /* 0x38 */ Executive::ExecuteST_pDX_DY,
            /* 0x39 */ Executive::ExecuteSTQ_pIMM16_DY,
            /* 0x3A */ Executive::ExecuteSTQ_pWX_DY,
            /* 0x3B */ Executive::ExecuteSSP_pIMM32,
            /* 0x3C */ Executive::ExecutePUSH_DY,
            /* 0x3D */ Executive::ExecuteMV_DX_DY,
            /* 0x3E */ Executive::ExecuteSPO_DX,
            /* 0x3F */ Executive::ExecuteSPI_DY,
            /* 0x40 */ Executive::ExecuteJMP_X_IMM32,
            /* 0x41 */ Executive::ExecuteJMP_X_DY,
            /* 0x42 */ Executive::ExecuteJPB_X_SIMM16,
            /* 0x43 */ Executive::ExecuteCALL_X_IMM32,
            /* 0x44 */ Executive::ExecuteINT_XY,
            /* 0x45 */ Executive::ExecuteRET_X,
            /* 0x46 */ Executive::ExecuteRETI,
            /* 0x47 */ nullptr,
            /* 0x48 */ nullptr,
            /* 0x49 */ nullptr,
            /* 0x4A */ nullptr,
            /* 0x4B */ nullptr,
            /* 0x4C */ nullptr,
            /* 0x4D */ nullptr,
            /* 0x4E */ Executive::ExecuteMFI_LY,
            /* 0x4F */ Executive::ExecuteMFO_LX,
            Executive::ExecuteADD_L0_IMM8,
            Executive::ExecuteADD_L0_LY,
            Executive::ExecuteADD_L0_pDY,
            Executive::ExecuteADC_L0_IMM8,
            Executive::ExecuteADC_L0_LY,
            Executive::ExecuteADC_L0_pDY,
            Executive::ExecuteSUB_L0_IMM8,
            Executive::ExecuteSUB_L0_LY,
            Executive::ExecuteSUB_L0_pDY,
            Executive::ExecuteSBC_L0_IMM8,
            Executive::ExecuteSBC_L0_LY,
            Executive::ExecuteSBC_L0_pDY,
            Executive::ExecuteINC_LX,
            Executive::ExecuteINC_pDX,
            Executive::ExecuteDEC_LX,
            Executive::ExecuteDEC_pDX,
            Executive::ExecuteADD_W0_IMM16,
            Executive::ExecuteADD_W0_WY,
            Executive::ExecuteADD_D0_IMM32,
            Executive::ExecuteADD_D0_DY,
            Executive::ExecuteSUB_W0_IMM16,
            Executive::ExecuteSUB_W0_WY,
            Executive::ExecuteSUB_D0_IMM32,
            Executive::ExecuteSUB_D0_DY,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            Executive::ExecuteINC_WX,
            Executive::ExecuteINC_DX,
            Executive::ExecuteDEC_WX,
            Executive::ExecuteDEC_DX,
            Executive::ExecuteAND_L0_IMM8,
            Executive::ExecuteAND_L0_LY,
            Executive::ExecuteAND_L0_pDY,
            Executive::ExecuteOR_L0_IMM8,
            Executive::ExecuteOR_L0_LY,
            Executive::ExecuteOR_L0_pDY,
            Executive::ExecuteXOR_L0_IMM8,
            Executive::ExecuteXOR_L0_LY,
            Executive::ExecuteXOR_L0_pDY,
            Executive::ExecuteNOT_LX,
            Executive::ExecuteNOT_pDX,
            nullptr,
            nullptr,
            Executive::ExecuteCMP_L0_IMM8,
            Executive::ExecuteCMP_L0_LY,
            Executive::ExecuteCMP_L0_pDY,
            Executive::ExecuteSLA_LX,
            Executive::ExecuteSLA_pDX,
            Executive::ExecuteSRA_LX,
            Executive::ExecuteSRA_pDX,
            Executive::ExecuteSRL_LX,
            Executive::ExecuteSRL_pDX,
            Executive::ExecuteSWAP_LX,
            Executive::ExecuteSWAP_pDX,
            Executive::ExecuteSWAP_WX,
            Executive::ExecuteSWAP_DX,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            Executive::ExecuteRLA,
            Executive::ExecuteRL_LX,
            Executive::ExecuteRL_pDX,
            Executive::ExecuteRLCA,
            Executive::ExecuteRLC_LX,
            Executive::ExecuteRLC_pDX,
            Executive::ExecuteRRA,
            Executive::ExecuteRR_LX,
            Executive::ExecuteRR_pDX,
            Executive::ExecuteRRCA,
            Executive::ExecuteRRC_LX,
            Executive::ExecuteRRC_pDX,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            Executive::ExecuteBIT_Y_LX,
            Executive::ExecuteBIT_Y_pDX,
            Executive::ExecuteSET_Y_LX,
            Executive::ExecuteSET_Y_pDX,
            Executive::ExecuteRES_Y_LX,
            Executive::ExecuteRES_Y_pDX,
            Executive::ExecuteTOG_Y_LX,
            Executive::ExecuteTOG_Y_pDX,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            Executive::ExecuteLDI_LX_pDY,
            Executive::ExecuteLDD_LX_pDY,
            Executive::ExecuteSTI_pDX_LY,
            Executive::ExecuteSTD_pDX_LY,
            Executive::ExecuteASP_SIMM8,
            Executive::ExecuteST_pDX_IMM8,
            Executive::ExecuteLASP_DX_SIMM8,
            Executive::ExecuteISP,
            Executive::ExecuteDSP,
            Executive::ExecuteASR_DX,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        };

        // Execute the instruction as normal.
        const Instruction& inst = pCore.mInstructionRegister;
        bool good = false;
        const auto executor = kExecutors[inst.mOpcode];

        if (executor != nullptr)
            { good = executor(pCore, inst); }
        else
            { return pCore.RaiseException(Exception::InvalidInstruction); }

        // If successful, observe the instruction's execution.
        if (good == true && pCore.mInstructionExecuteCallback != nullptr)
        {
            pCore.mInstructionExecuteCallback(pCore,
                pCore.mInstructionRegister);
        }

        return good;
    }
}

// Private Methods - Helpers ***************************************************

namespace G10::CPU
{
    auto Executive::CheckCondition (Core& pCore, std::uint8_t pCond) -> bool
    {
        switch (static_cast<Condition>(pCond))
        {
            case Condition::None:           return true;
            case Condition::ZeroSet:        return pCore.mFlagsRegister.mZero == true;
            case Condition::ZeroClear:      return pCore.mFlagsRegister.mZero == false;
            case Condition::CarrySet:       return pCore.mFlagsRegister.mCarry == true;
            case Condition::CarryClear:     return pCore.mFlagsRegister.mCarry == false;
            case Condition::OverflowSet:    return pCore.mFlagsRegister.mOverflow == true;
            case Condition::OverflowClear:  return pCore.mFlagsRegister.mOverflow == false;
            default:                        
                return pCore.RaiseException(Exception::InvalidArgument);
                return false;
        }
    }
}
