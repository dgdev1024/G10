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
            if (doubleFaultCondition == true)
            {
                debug("Double Fault Condition: "
                    "Acknowledging exception {} while handling exception {}!",
                    stx::under(pCore.mExceptionPending),
                    stx::under(pCore.mException));
            }

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
                debug("Double Fault Condition: "
                    "Exception {} raised while jumping to exception handler!",
                    stx::under(pCore.mExceptionPending)
                );

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
                debug("Double Fault Condition: "
                    "Exception {} raised while jumping to interrupt vector {}!",
                    stx::under(pCore.mExceptionPending),
                    i
                );

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
            /* 0x4E */ Executive::ExecuteMFI_HY,
            /* 0x4F */ Executive::ExecuteMFO_HX,
            /* 0x50 */ Executive::ExecuteADD_LX_IMM8,
            /* 0x51 */ Executive::ExecuteADD_LX_LY,
            /* 0x52 */ Executive::ExecuteADD_LX_pDY,
            /* 0x53 */ Executive::ExecuteADC_LX_IMM8,
            /* 0x54 */ Executive::ExecuteADC_LX_LY,
            /* 0x55 */ Executive::ExecuteADC_LX_pDY,
            /* 0x56 */ Executive::ExecuteSUB_LX_IMM8,
            /* 0x57 */ Executive::ExecuteSUB_LX_LY,
            /* 0x58 */ Executive::ExecuteSUB_LX_pDY,
            /* 0x59 */ Executive::ExecuteSBC_LX_IMM8,
            /* 0x5A */ Executive::ExecuteSBC_LX_LY,
            /* 0x5B */ Executive::ExecuteSBC_LX_pDY,
            /* 0x5C */ Executive::ExecuteINC_LX,
            /* 0x5D */ Executive::ExecuteINC_pDX,
            /* 0x5E */ Executive::ExecuteDEC_LX,
            /* 0x5F */ Executive::ExecuteDEC_pDX,
            /* 0x60 */ Executive::ExecuteADD_WX_IMM16,
            /* 0x61 */ Executive::ExecuteADD_WX_WY,
            /* 0x62 */ Executive::ExecuteADD_DX_IMM32,
            /* 0x63 */ Executive::ExecuteADD_DX_DY,
            /* 0x64 */ Executive::ExecuteSUB_WX_IMM16,
            /* 0x65 */ Executive::ExecuteSUB_WX_WY,
            /* 0x66 */ Executive::ExecuteSUB_DX_IMM32,
            /* 0x67 */ Executive::ExecuteSUB_DX_DY,
            /* 0x68 */ nullptr,
            /* 0x69 */ nullptr,
            /* 0x6A */ nullptr,
            /* 0x6B */ nullptr,
            /* 0x6C */ Executive::ExecuteINC_WX,
            /* 0x6D */ Executive::ExecuteINC_DX,
            /* 0x6E */ Executive::ExecuteDEC_WX,
            /* 0x6F */ Executive::ExecuteDEC_DX,
            /* 0x70 */ Executive::ExecuteAND_LX_IMM8,
            /* 0x71 */ Executive::ExecuteAND_LX_LY,
            /* 0x72 */ Executive::ExecuteAND_LX_pDY,
            /* 0x73 */ Executive::ExecuteOR_LX_IMM8,
            /* 0x74 */ Executive::ExecuteOR_LX_LY,
            /* 0x75 */ Executive::ExecuteOR_LX_pDY,
            /* 0x76 */ Executive::ExecuteXOR_LX_IMM8,
            /* 0x77 */ Executive::ExecuteXOR_LX_LY,
            /* 0x78 */ Executive::ExecuteXOR_LX_pDY,
            /* 0x79 */ Executive::ExecuteNOT_LX,
            /* 0x7A */ Executive::ExecuteNOT_pDX,
            /* 0x7B */ nullptr,
            /* 0x7C */ nullptr,
            /* 0x7D */ Executive::ExecuteCMP_LX_IMM8,
            /* 0x7E */ Executive::ExecuteCMP_LX_LY,
            /* 0x7F */ Executive::ExecuteCMP_LX_pDY,
            /* 0x80 */ Executive::ExecuteSLA_LX,
            /* 0x81 */ Executive::ExecuteSLA_HX,
            /* 0x82 */ Executive::ExecuteSLA_pDX,
            /* 0x83 */ Executive::ExecuteSRA_LX,
            /* 0x84 */ Executive::ExecuteSRA_HX,
            /* 0x85 */ Executive::ExecuteSRA_pDX,
            /* 0x86 */ Executive::ExecuteSRL_LX,
            /* 0x87 */ Executive::ExecuteSRL_HX,
            /* 0x88 */ Executive::ExecuteSRL_pDX,
            /* 0x89 */ Executive::ExecuteSWAP_LX,
            /* 0x8A */ Executive::ExecuteSWAP_HX,
            /* 0x8B */ Executive::ExecuteSWAP_pDX,
            /* 0x8C */ Executive::ExecuteSWAP_WX,
            /* 0x8D */ Executive::ExecuteSWAP_DX,
            /* 0x8E */ nullptr,
            /* 0x8F */ nullptr,
            /* 0x90 */ Executive::ExecuteRLA,
            /* 0x91 */ Executive::ExecuteRL_LX,
            /* 0x92 */ Executive::ExecuteRL_HX,
            /* 0x93 */ Executive::ExecuteRL_pDX,
            /* 0x94 */ Executive::ExecuteRLCA,
            /* 0x95 */ Executive::ExecuteRLC_LX,
            /* 0x96 */ Executive::ExecuteRLC_HX,
            /* 0x97 */ Executive::ExecuteRLC_pDX,
            /* 0x98 */ Executive::ExecuteRRA,
            /* 0x99 */ Executive::ExecuteRR_LX,
            /* 0x9A */ Executive::ExecuteRR_HX,
            /* 0x9B */ Executive::ExecuteRR_pDX,
            /* 0x9C */ Executive::ExecuteRRCA,
            /* 0x9D */ Executive::ExecuteRRC_LX,
            /* 0x9E */ Executive::ExecuteRRC_HX,
            /* 0x9F */ Executive::ExecuteRRC_pDX,
            /* 0xA0 */ Executive::ExecuteBIT_Y_LX,
            /* 0xA1 */ Executive::ExecuteBIT_Y_HX,
            /* 0xA2 */ Executive::ExecuteBIT_Y_pDX,
            /* 0xA3 */ Executive::ExecuteSET_Y_LX,
            /* 0xA4 */ Executive::ExecuteSET_Y_HX,
            /* 0xA5 */ Executive::ExecuteSET_Y_pDX,
            /* 0xA6 */ Executive::ExecuteRES_Y_LX,
            /* 0xA7 */ Executive::ExecuteRES_Y_HX,
            /* 0xA8 */ Executive::ExecuteRES_Y_pDX,
            /* 0xA9 */ Executive::ExecuteTOG_Y_LX,
            /* 0xAA */ Executive::ExecuteTOG_Y_HX,
            /* 0xAB */ Executive::ExecuteTOG_Y_pDX,
            /* 0xAC */ nullptr,
            /* 0xAD */ nullptr,
            /* 0xAE */ nullptr,
            /* 0xAF */ nullptr,
            /* 0xB0 */ Executive::ExecuteLDI_LX_pDY,
            /* 0xB1 */ Executive::ExecuteLDI_HX_pDY,
            /* 0xB2 */ Executive::ExecuteLDD_LX_pDY,
            /* 0xB3 */ Executive::ExecuteLDD_HX_pDY,
            /* 0xB4 */ Executive::ExecuteSTI_pDX_LY,
            /* 0xB5 */ Executive::ExecuteSTI_pDX_HY,
            /* 0xB6 */ Executive::ExecuteSTD_pDX_LY,
            /* 0xB7 */ Executive::ExecuteSTD_pDX_HY,
            /* 0xB8 */ Executive::ExecuteASP_SIMM8,
            /* 0xB9 */ Executive::ExecuteST_pDX_IMM8,
            /* 0xBA */ Executive::ExecuteLASP_DX_SIMM8,
            /* 0xBB */ Executive::ExecuteISP,
            /* 0xBC */ Executive::ExecuteDSP,
            /* 0xBD */ Executive::ExecuteASR_DX,
            /* 0xBE */ nullptr,
            /* 0xBF */ nullptr,
            /* 0xC0 */ Executive::ExecuteLD_HX_IMM8,
            /* 0xC1 */ Executive::ExecuteLD_HX_pIMM32,
            /* 0xC2 */ Executive::ExecuteLD_HX_pDY,
            /* 0xC3 */ Executive::ExecuteLDQ_HX_pIMM16,
            /* 0xC4 */ Executive::ExecuteLDQ_HX_pWY,
            /* 0xC5 */ Executive::ExecuteLDP_HX_pIMM8,
            /* 0xC6 */ Executive::ExecuteLDP_HX_pLY,
            /* 0xC7 */ Executive::ExecuteST_pIMM32_HY,
            /* 0xC8 */ Executive::ExecuteST_pDX_HY,
            /* 0xC9 */ Executive::ExecuteSTQ_pIMM16_HY,
            /* 0xCA */ Executive::ExecuteSTQ_pWX_HY,
            /* 0xCB */ Executive::ExecuteSTP_pIMM8_HY,
            /* 0xCC */ Executive::ExecuteSTP_pLX_HY,
            /* 0xCD */ Executive::ExecuteMV_HX_HY,
            /* 0xCE */ nullptr,
            /* 0xCF */ nullptr,
            /* 0xD0 */ Executive::ExecuteADD_LX_HY,
            /* 0xD1 */ Executive::ExecuteADC_LX_HY,
            /* 0xD2 */ Executive::ExecuteSUB_LX_HY,
            /* 0xD3 */ Executive::ExecuteSBC_LX_HY,
            /* 0xD4 */ Executive::ExecuteINC_HX,
            /* 0xD5 */ Executive::ExecuteDEC_HX,
            /* 0xD6 */ Executive::ExecuteAND_LX_HY,
            /* 0xD7 */ Executive::ExecuteOR_LX_HY,
            /* 0xD8 */ Executive::ExecuteXOR_LX_HY,
            /* 0xD9 */ Executive::ExecuteNOT_HX,
            /* 0xDA */ Executive::ExecuteCMP_LX_HY,
            /* 0xDB */ nullptr,
            /* 0xDC */ nullptr,
            /* 0xDD */ nullptr,
            /* 0xDE */ nullptr,
            /* 0xDF */ nullptr,
            /* 0xE0 */ nullptr,
            /* 0xE1 */ nullptr,
            /* 0xE2 */ nullptr,
            /* 0xE3 */ nullptr,
            /* 0xE4 */ nullptr,
            /* 0xE5 */ nullptr,
            /* 0xE6 */ nullptr,
            /* 0xE7 */ nullptr,
            /* 0xE8 */ nullptr,
            /* 0xE9 */ nullptr,
            /* 0xEA */ nullptr,
            /* 0xEB */ nullptr,
            /* 0xEC */ nullptr,
            /* 0xED */ nullptr,
            /* 0xEE */ nullptr,
            /* 0xEF */ nullptr,
            /* 0xF0 */ nullptr,
            /* 0xF1 */ nullptr,
            /* 0xF2 */ nullptr,
            /* 0xF3 */ nullptr,
            /* 0xF4 */ nullptr,
            /* 0xF5 */ nullptr,
            /* 0xF6 */ nullptr,
            /* 0xF7 */ nullptr,
            /* 0xF8 */ nullptr,
            /* 0xF9 */ nullptr,
            /* 0xFA */ nullptr,
            /* 0xFB */ nullptr,
            /* 0xFC */ nullptr,
            /* 0xFD */ nullptr,
            /* 0xFE */ nullptr,
            /* 0xFF */ nullptr
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
