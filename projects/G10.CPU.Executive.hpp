/**
 * @file    G10.CPU.Executive.hpp
 * @brief   Contains declarations for the G10 CPU Executive class, and 
 *          related definitions.
 */

// Includes ********************************************************************

#include <G10.CPU.Core.hpp>

// Classes *********************************************************************

namespace G10::CPU
{
    class G10_API Executive final
    {
    public: // Methods *********************************************************

        static auto StepCore (Core& pCore) -> bool;

    private: // Methods - Lifecycle ********************************************

        static auto ServiceInterrupt (Core& pCore, std::uint8_t& pServicedLine) -> bool;
        static auto DecodeNextInstruction (Core& pCore) -> bool;

    private: // Methods - Helpers **********************************************

        static auto CheckCondition (Core& pCore, std::uint8_t pCond) -> bool;

    private: // Methods - Arithmetic Logic Unit ********************************

        static auto PerformADD8 (Core& pCore, bool pWithCarry, std::uint8_t pLeft, std::uint8_t pRight, std::uint8_t& pResultOut) -> bool;
        static auto PerformADD16 (Core& pCore, std::uint16_t pLeft, std::uint16_t pRight, std::uint16_t& pResultOut) -> bool;
        static auto PerformADD32 (Core& pCore, std::uint32_t pLeft, std::uint32_t pRight, std::uint32_t& pResultOut) -> bool;
        static auto PerformSUB8 (Core& pCore, bool pWithBorrow, std::uint8_t pLeft, std::uint8_t pRight, std::uint8_t* pOptResultOut) -> bool;
        static auto PerformSUB16 (Core& pCore, std::uint16_t pLeft, std::uint16_t pRight, std::uint16_t& pResultOut) -> bool;
        static auto PerformSUB32 (Core& pCore, std::uint32_t pLeft, std::uint32_t pRight, std::uint32_t& pResultOut) -> bool;
        static auto PerformINC8 (Core& pCore, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool;
        static auto PerformINC16 (Core& pCore, std::uint16_t pValue, std::uint16_t& pResultOut) -> bool;
        static auto PerformINC32 (Core& pCore, std::uint32_t pValue, std::uint32_t& pResultOut) -> bool;
        static auto PerformDEC8 (Core& pCore, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool;
        static auto PerformDEC16 (Core& pCore, std::uint16_t pValue, std::uint16_t& pResultOut) -> bool;
        static auto PerformDEC32 (Core& pCore, std::uint32_t pValue, std::uint32_t& pResultOut) -> bool;
        static auto PerformAND8 (Core& pCore, std::uint8_t pLeft, std::uint8_t pRight, std::uint8_t& pResultOut) -> bool;
        static auto PerformOR8 (Core& pCore, std::uint8_t pLeft, std::uint8_t pRight, std::uint8_t& pResultOut) -> bool;
        static auto PerformXOR8 (Core& pCore, std::uint8_t pLeft, std::uint8_t pRight, std::uint8_t& pResultOut) -> bool;
        static auto PerformNOT8 (Core& pCore, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool;
        static auto PerformSHL8 (Core& pCore, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool;
        static auto PerformSHR8 (Core& pCore, bool pUnsigned, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool;
        static auto PerformROL8 (Core& pCore, bool pThroughCarry, bool pClearZero, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool;
        static auto PerformROR8 (Core& pCore, bool pThroughCarry, bool pClearZero, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool;
        static auto PerformSWAP8 (Core& pCore, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool;
        static auto PerformSWAP16 (Core& pCore, std::uint16_t pValue, std::uint16_t& pResultOut) -> bool;
        static auto PerformSWAP32 (Core& pCore, std::uint32_t pValue, std::uint32_t& pResultOut) -> bool;
        static auto PerformBIT8 (Core& pCore, std::uint8_t pValue, std::uint8_t pBit) -> bool;
        static auto PerformSET8 (Core& pCore, std::uint8_t pValue, std::uint8_t pBit, std::uint8_t& pResultOut) -> bool;
        static auto PerformRES8 (Core& pCore, std::uint8_t pValue, std::uint8_t pBit, std::uint8_t& pResultOut) -> bool;
        static auto PerformTOG8 (Core& pCore, std::uint8_t pValue, std::uint8_t pBit, std::uint8_t& pResultOut) -> bool;

    private: // Methods - Instruction Execution ********************************

        static auto ExecuteNOP (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSTOP (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteHALT (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteDI (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteEI (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteEII (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteDAA (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSCF (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteCCF (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteCLV (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSEV (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteREX_XY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLEC (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLD_LX_IMM8 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLD_LX_pIMM32 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLD_LX_pDY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLDQ_LX_pIMM16 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLDQ_LX_pWY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLDP_LX_pIMM8 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLDP_LX_pLY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteST_pIMM32_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteST_pDX_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSTQ_pIMM16_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSTQ_pWX_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSTP_pIMM8_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSTP_pLX_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteMV_LX_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteMV_HX_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteMV_LX_HY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLD_WX_IMM16 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLD_WX_pIMM32 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLD_WX_pDY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLDQ_WX_pIMM16 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLDQ_WX_pWY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteST_pIMM32_WY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteST_pDX_WY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSTQ_pIMM16_WY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSTQ_pWX_WY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteMV_WX_WY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteMWH_DX_WY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteMWL_WX_DY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLD_DX_IMM32 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLD_DX_pIMM32 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLD_DX_pDY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLDQ_DX_pIMM16 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLDQ_DX_pWY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLSP_IMM32 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecutePOP_DX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteST_pIMM32_DY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteST_pDX_DY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSTQ_pIMM16_DY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSTQ_pWX_DY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSSP_pIMM32 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecutePUSH_DY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteMV_DX_DY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSPO_DX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSPI_DY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteJMP_X_IMM32 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteJMP_X_DY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteJPB_X_SIMM16 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteCALL_X_IMM32 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteINT_XY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRET_X (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRETI (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteMFI_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteMFO_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteADD_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteADD_L0_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteADD_L0_pDY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteADC_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteADC_L0_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteADC_L0_pDY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSUB_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSUB_L0_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSUB_L0_pDY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSBC_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSBC_L0_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSBC_L0_pDY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteINC_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteINC_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteDEC_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteDEC_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteADD_W0_IMM16 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteADD_W0_WY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteADD_D0_IMM32 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteADD_D0_DY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSUB_W0_IMM16 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSUB_W0_WY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSUB_D0_IMM32 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSUB_D0_DY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteINC_WX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteINC_DX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteDEC_WX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteDEC_DX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteAND_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteAND_L0_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteAND_L0_pDY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteOR_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteOR_L0_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteOR_L0_pDY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteXOR_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteXOR_L0_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteXOR_L0_pDY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteNOT_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteNOT_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteCMP_L0_IMM8 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteCMP_L0_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteCMP_L0_pDY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSLA_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSLA_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSRA_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSRA_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSRL_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSRL_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSWAP_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSWAP_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSWAP_WX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSWAP_DX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRLA (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRL_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRL_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRLCA (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRLC_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRLC_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRRA (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRR_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRR_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRRCA (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRRC_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRRC_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteBIT_Y_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteBIT_Y_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSET_Y_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSET_Y_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRES_Y_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteRES_Y_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteTOG_Y_LX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteTOG_Y_pDX (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLDI_LX_pDY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLDD_LX_pDY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSTI_pDX_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteSTD_pDX_LY (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteASP_SIMM8 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteST_pDX_IMM8 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteLASP_DX_SIMM8 (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteISP (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteDSP (Core& pCore, const Instruction& pInst) -> bool;
        static auto ExecuteASR_DX (Core& pCore, const Instruction& pInst) -> bool;

    };
}
