/**
 * @file    G10.CPU.Executive.ALU.cpp
 * @brief   Contains implementations for the G10 CPU Executive class's
 *          arithmetic logic unit (ALU) methods, and related definitions.
 */

// Includes ********************************************************************

#include <G10.CPU.Executive.hpp>

// Private Methods - Arithmetic Logic Unit *************************************

namespace G10::CPU
{
    auto Executive::PerformADD8 (Core& pCore, bool pWithCarry, std::uint8_t pLeft, std::uint8_t pRight, std::uint8_t& pResultOut) -> bool
    {
        std::uint8_t carryIn = 0;
        if (pWithCarry == true)
        {
            bool carry = false;
            if (pCore.ReadFlag(Flag::Carry, carry) == false)
                { return false; }
            carryIn = carry ? 1 : 0;
        }

        std::uint16_t carrySum =
            static_cast<std::uint16_t>(pLeft) +
            static_cast<std::uint16_t>(pRight) +
            carryIn;
        std::uint8_t halfCarrySum =
            static_cast<std::uint8_t>((pLeft & 0x0F) + (pRight & 0x0F) + carryIn);
        std::uint8_t result = static_cast<std::uint8_t>(carrySum & 0xFF);
        bool overflowCheck = ((pLeft ^ result) & (pRight ^ result) & 0x80) != 0;
        pResultOut = result;

        // Batch flag updates: clear all affected flags, then set new values
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00000111;  // Clear Z, N, H, C, V bits
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        if (halfCarrySum > 0x0F) 
            { flags |= stx::under(Flag::HalfCarry); }
        if (carrySum > 0xFF) 
            { flags |= stx::under(Flag::Carry); }
        if (overflowCheck) 
            { flags |= stx::under(Flag::Overflow); }
        
        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformADD16 (Core& pCore, std::uint16_t pLeft, std::uint16_t pRight, std::uint16_t& pResultOut) -> bool
    {
        std::uint32_t carrySum =
            static_cast<std::uint32_t>(pLeft) +
            static_cast<std::uint32_t>(pRight);
        std::uint16_t halfCarrySum =
            static_cast<std::uint16_t>((pLeft & 0x0FFF) + (pRight & 0x0FFF));
        std::uint16_t result = static_cast<std::uint16_t>(carrySum & 0xFFFF);
        bool overflowCheck = ((pLeft ^ result) & (pRight ^ result) & 0x8000) != 0;
        pResultOut = result;

        // Batch flag updates: clear all affected flags, then set new values
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00000111;  // Clear Z, N, H, C, V bits
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        if (halfCarrySum > 0x0FFF) 
            { flags |= stx::under(Flag::HalfCarry); }
        if (carrySum > 0xFFFF) 
            { flags |= stx::under(Flag::Carry); }
        if (overflowCheck) 
            { flags |= stx::under(Flag::Overflow); }
        
        return pCore.WriteFlagsRegister(flags) && pCore.AddMachineCycles(1);
    }

    auto Executive::PerformADD32 (Core& pCore, std::uint32_t pLeft, std::uint32_t pRight, std::uint32_t& pResultOut) -> bool
    {
        std::uint64_t carrySum =
            static_cast<std::uint64_t>(pLeft) +
            static_cast<std::uint64_t>(pRight);
        std::uint32_t halfCarrySum =
            (pLeft & 0x0FFFFFFF) + (pRight & 0x0FFFFFFF);
        std::uint32_t result = static_cast<std::uint32_t>(carrySum & 0xFFFFFFFFull);
        bool overflowCheck = ((pLeft ^ result) & (pRight ^ result) & 0x80000000) != 0;
        pResultOut = result;

        // Batch flag updates: clear all affected flags, then set new values
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00000111;  // Clear Z, N, H, C, V bits
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        if (halfCarrySum > 0x0FFFFFFF) 
            { flags |= stx::under(Flag::HalfCarry); }
        if (carrySum > 0xFFFFFFFFull) 
            { flags |= stx::under(Flag::Carry); }
        if (overflowCheck) 
            { flags |= stx::under(Flag::Overflow); }
        
        return pCore.WriteFlagsRegister(flags) && pCore.AddMachineCycles(3);
    }

    auto Executive::PerformSUB8 (Core& pCore, bool pWithBorrow, std::uint8_t pLeft, std::uint8_t pRight, std::uint8_t* pOptResultOut) -> bool
    {
        std::uint8_t carryIn = 0;
        if (pWithBorrow == true)
        {
            bool carry = false;
            if (pCore.ReadFlag(Flag::Carry, carry) == false)
                { return false; }
            carryIn = carry ? 1 : 0;
        }

        std::int16_t borrowDiff =
            static_cast<std::int16_t>(pLeft) -
            static_cast<std::int16_t>(pRight) -
            static_cast<std::int16_t>(carryIn);
        std::int8_t halfBorrowDiff =
            static_cast<std::int8_t>(pLeft & 0x0F) -
            static_cast<std::int8_t>(pRight & 0x0F) -
            static_cast<std::int8_t>(carryIn);
        std::uint8_t result = static_cast<std::uint8_t>(borrowDiff & 0xFF);
        bool overflowCheck = ((pLeft ^ pRight) & (pLeft ^ result) & 0x80) != 0;

        if (pOptResultOut != nullptr)
            { *pOptResultOut = result; }

        // Batch flag updates: clear all affected flags, then set new values
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00000111;  // Clear Z, N, H, C, V bits
        flags |= stx::under(Flag::Subtract);  // Subtract flag always set for SUB
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        if (halfBorrowDiff < 0) 
            { flags |= stx::under(Flag::HalfCarry); }
        if (borrowDiff < 0) 
            { flags |= stx::under(Flag::Carry); }
        if (overflowCheck) 
            { flags |= stx::under(Flag::Overflow); }
        
        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformSUB16 (Core& pCore, std::uint16_t pLeft, std::uint16_t pRight, std::uint16_t& pResultOut) -> bool
    {
        std::int32_t borrowDiff =
            static_cast<std::int32_t>(pLeft) -
            static_cast<std::int32_t>(pRight);
        std::int16_t halfBorrowDiff =
            static_cast<std::int16_t>(pLeft & 0x0FFF) -
            static_cast<std::int16_t>(pRight & 0x0FFF);
        std::uint16_t result = static_cast<std::uint16_t>(borrowDiff & 0xFFFF);
        bool overflowCheck = ((pLeft ^ pRight) & (pLeft ^ result) & 0x8000) != 0;
        pResultOut = result;

        // Batch flag updates
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00000111;  // Clear Z, N, H, C, V bits
        flags |= stx::under(Flag::Subtract);
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        if (halfBorrowDiff < 0) 
            { flags |= stx::under(Flag::HalfCarry); }
        if (borrowDiff < 0) 
            { flags |= stx::under(Flag::Carry); }
        if (overflowCheck) 
            { flags |= stx::under(Flag::Overflow); }
        
        return pCore.WriteFlagsRegister(flags) && pCore.AddMachineCycles(1);
    }

    auto Executive::PerformSUB32 (Core& pCore, std::uint32_t pLeft, std::uint32_t pRight, std::uint32_t& pResultOut) -> bool
    {
        std::int64_t borrowDiff =
            static_cast<std::int64_t>(pLeft) -
            static_cast<std::int64_t>(pRight);
        std::int32_t halfBorrowDiff =
            static_cast<std::int32_t>(pLeft & 0x0FFFFFFF) -
            static_cast<std::int32_t>(pRight & 0x0FFFFFFF);
        std::uint32_t result = static_cast<std::uint32_t>(borrowDiff & 0xFFFFFFFFll);
        bool overflowCheck = ((pLeft ^ pRight) & (pLeft ^ result) & 0x80000000) != 0;
        pResultOut = result;

        // Batch flag updates
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00000111;  // Clear Z, N, H, C, V bits
        flags |= stx::under(Flag::Subtract);
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        if (halfBorrowDiff < 0) 
            { flags |= stx::under(Flag::HalfCarry); }
        if (borrowDiff < 0) 
            { flags |= stx::under(Flag::Carry); }
        if (overflowCheck) 
            { flags |= stx::under(Flag::Overflow); }
        
        return pCore.WriteFlagsRegister(flags) && pCore.AddMachineCycles(3);
    }

    auto Executive::PerformINC8 (Core& pCore, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool
    {
        std::uint8_t result = static_cast<std::uint8_t>(pValue + 1);
        bool halfCarryCheck = (result & 0x0F) == 0;
        bool overflowCheck = (pValue == 0x7F);
        pResultOut = result;

        // Batch flag updates
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00010111;  // Clear Z, N, H, V bits; Leave C unchanged.
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        if (halfCarryCheck) 
            { flags |= stx::under(Flag::HalfCarry); }
        if (overflowCheck) 
            { flags |= stx::under(Flag::Overflow); }
        
        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformINC16 (Core& pCore, std::uint16_t pValue, std::uint16_t& pResultOut) -> bool
    {
        std::uint16_t result = static_cast<std::uint16_t>(pValue + 1);
        bool halfCarryCheck = (result & 0x0FFF) == 0;
        pResultOut = result;

        // Batch flag updates
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00011111;  // Clear Z, N, H bits
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        if (halfCarryCheck) 
            { flags |= stx::under(Flag::HalfCarry); }
        
        return pCore.WriteFlagsRegister(flags) && pCore.AddMachineCycles(1);
    }

    auto Executive::PerformINC32 (Core& pCore, std::uint32_t pValue, std::uint32_t& pResultOut) -> bool
    {
        std::uint32_t result = pValue + 1;
        bool halfCarryCheck = (result & 0x0FFFFFFF) == 0;
        pResultOut = result;

        // Batch flag updates
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00011111;  // Clear Z, N, H bits
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        if (halfCarryCheck) 
            { flags |= stx::under(Flag::HalfCarry); }
        
        return pCore.WriteFlagsRegister(flags) && pCore.AddMachineCycles(3);
    }

    auto Executive::PerformDEC8 (Core& pCore, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool
    {
        std::uint8_t result = static_cast<std::uint8_t>(pValue - 1);
        bool halfCarryCheck = (pValue & 0x0F) == 0;
        bool overflowCheck = (pValue == 0x80);
        pResultOut = result;

        // Batch flag updates
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00010111;  // Clear Z, N, H, V bits; Leave C unchanged
        flags |= stx::under(Flag::Subtract);
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        if (halfCarryCheck) 
            { flags |= stx::under(Flag::HalfCarry); }
        if (overflowCheck) 
            { flags |= stx::under(Flag::Overflow); }
        
        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformDEC16 (Core& pCore, std::uint16_t pValue, std::uint16_t& pResultOut) -> bool
    {
        std::uint16_t result = static_cast<std::uint16_t>(pValue - 1);
        bool halfCarryCheck = (pValue & 0x0FFF) == 0;
        pResultOut = result;

        // Batch flag updates
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00011111;  // Clear Z, N, H bits
        flags |= stx::under(Flag::Subtract);
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        if (halfCarryCheck) 
            { flags |= stx::under(Flag::HalfCarry); }
        
        return pCore.WriteFlagsRegister(flags) && pCore.AddMachineCycles(1);
    }

    auto Executive::PerformDEC32 (Core& pCore, std::uint32_t pValue, std::uint32_t& pResultOut) -> bool
    {
        std::uint32_t result = pValue - 1;
        bool halfCarryCheck = (pValue & 0x0FFFFFFF) == 0;
        pResultOut = result;

        // Batch flag updates
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00011111;  // Clear Z, N, H bits
        flags |= stx::under(Flag::Subtract);
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        if (halfCarryCheck) 
            { flags |= stx::under(Flag::HalfCarry); }
        
        return pCore.WriteFlagsRegister(flags) && pCore.AddMachineCycles(3);
    }

    auto Executive::PerformAND8 (Core& pCore, std::uint8_t pLeft, std::uint8_t pRight, std::uint8_t& pResultOut) -> bool
    {
        std::uint8_t result = static_cast<std::uint8_t>(pLeft & pRight);
        pResultOut = result;

        // Batch flag updates: AND sets HalfCarry, clears others
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00100111;  // Clear Z, N, C, V bits
        flags |= stx::under(Flag::HalfCarry);
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        
        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformOR8 (Core& pCore, std::uint8_t pLeft, std::uint8_t pRight, std::uint8_t& pResultOut) -> bool
    {
        std::uint8_t result = static_cast<std::uint8_t>(pLeft | pRight);
        pResultOut = result;

        // Batch flag updates: OR clears all flags except Zero
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00000111;  // Clear Z, N, H, C, V bits
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        
        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformXOR8 (Core& pCore, std::uint8_t pLeft, std::uint8_t pRight, std::uint8_t& pResultOut) -> bool
    {
        std::uint8_t result = static_cast<std::uint8_t>(pLeft ^ pRight);
        pResultOut = result;

        // Batch flag updates: XOR clears all flags except Zero
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00000111;  // Clear Z, N, H, C, V bits
        if (result == 0) 
            { flags |= stx::under(Flag::Zero); }
        
        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformNOT8 (Core& pCore, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool
    {
        std::uint8_t result = static_cast<std::uint8_t>(~pValue);
        pResultOut = result;

        // Batch flag updates: NOT sets Subtract and HalfCarry, clears Overflow
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b10010111;
        flags |= stx::under(Flag::Subtract) |
                 stx::under(Flag::HalfCarry);
        
        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformSHL8 (Core& pCore, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool
    {
        std::uint8_t result = static_cast<std::uint8_t>((pValue << 1) & 0xFE);
        pResultOut = result;

        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00001111;  // Clear Z, N, H, C bits
        if (result == 0)
            { flags |= stx::under(Flag::Zero); }
        if ((pValue & 0x80) != 0)
            { flags |= stx::under(Flag::Carry); }

        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformSHR8 (Core& pCore, bool pUnsigned, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool
    {
        std::uint8_t result = (pUnsigned == false) ?
            static_cast<std::uint8_t>((pValue >> 1) | (pValue & 0x80)) :
            static_cast<std::uint8_t>((pValue >> 1) & 0x7F);
        pResultOut = result;

        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00001111;  // Clear Z, N, H, C bits
        if (result == 0)
            { flags |= stx::under(Flag::Zero); }
        if ((pValue & 0x01) != 0)
            { flags |= stx::under(Flag::Carry); }

        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformROL8 (Core& pCore, bool pThroughCarry, bool pClearZero, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool
    {
        bool carry = false;
        if (pCore.ReadFlag(Flag::Carry, carry) == false)
            { return false; }

        const bool carryOut = (pValue & 0x80) != 0;

        std::uint8_t result = 0;
        if (pThroughCarry == true)
            { result = static_cast<std::uint8_t>((pValue << 1) | (carry ? 1 : 0)); }
        else
            { result = static_cast<std::uint8_t>((pValue << 1) | ((pValue & 0x80) ? 1 : 0)); }

        pResultOut = result;

        bool newZero = (pClearZero == true) ? false : (result == 0);
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00001111;  // Clear Z, N, H, C bits
        if (newZero)
            { flags |= stx::under(Flag::Zero); }
        if (carryOut)
            { flags |= stx::under(Flag::Carry); }

        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformROR8 (Core& pCore, bool pThroughCarry, bool pClearZero, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool
    {
        bool carry = false;
        if (pCore.ReadFlag(Flag::Carry, carry) == false)
            { return false; }

        const bool carryOut = (pValue & 0x01) != 0;

        std::uint8_t result = 0;
        if (pThroughCarry == true)
            { result = static_cast<std::uint8_t>((pValue >> 1) | (carry ? 0x80 : 0)); }
        else
            { result = static_cast<std::uint8_t>((pValue >> 1) | ((pValue & 0x01) ? 0x80 : 0)); }

        pResultOut = result;

        bool newZero = (pClearZero == true) ? false : (result == 0);
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00001111;  // Clear Z, N, H, C bits
        if (newZero)
            { flags |= stx::under(Flag::Zero); }
        if (carryOut)
            { flags |= stx::under(Flag::Carry); }

        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformSWAP8 (Core& pCore, std::uint8_t pValue, std::uint8_t& pResultOut) -> bool
    {
        std::uint8_t result = static_cast<std::uint8_t>(((pValue & 0x0F) << 4) | ((pValue & 0xF0) >> 4));
        pResultOut = result;

        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00001111;  // Clear Z, N, H, C bits
        if (result == 0)
            { flags |= stx::under(Flag::Zero); }

        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformSWAP16 (Core& pCore, std::uint16_t pValue, std::uint16_t& pResultOut) -> bool
    {
        std::uint16_t result =
            static_cast<std::uint16_t>(((pValue & 0x00FF) << 8) | ((pValue & 0xFF00) >> 8));
        pResultOut = result;

        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00001111;  // Clear Z, N, H, C bits
        if (result == 0)
            { flags |= stx::under(Flag::Zero); }

        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformSWAP32 (Core& pCore, std::uint32_t pValue, std::uint32_t& pResultOut) -> bool
    {
        std::uint32_t result =
            ((pValue & 0x0000FFFF) << 16) | ((pValue & 0xFFFF0000) >> 16);
        pResultOut = result;

        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00001111;  // Clear Z, N, H, C bits
        if (result == 0)
            { flags |= stx::under(Flag::Zero); }

        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformBIT8 (Core& pCore, std::uint8_t pValue, std::uint8_t pBit) -> bool
    {
        if (pBit >= 8)
        {
            pCore.RaiseException(Exception::InvalidArgument);
            return false;
        }

        bool pSet = (pValue & (1 << pBit)) != 0;
        std::uint8_t flags = pCore.mFlagsRegister.mValue & 0b00011111;  // Clear Z, N, H bits
        if (pSet == false)
            { flags |= stx::under(Flag::Zero); }
        flags |= stx::under(Flag::HalfCarry);  // Set H flag

        return pCore.WriteFlagsRegister(flags);
    }

    auto Executive::PerformSET8 (Core& pCore, std::uint8_t pValue, std::uint8_t pBit, std::uint8_t& pResultOut) -> bool
    {
        if (pBit >= 8)
        {
            pCore.RaiseException(Exception::InvalidArgument);
            return false;
        }

        pResultOut = (pValue | (1 << pBit));
        return true;
    }

    auto Executive::PerformRES8 (Core& pCore, std::uint8_t pValue, std::uint8_t pBit, std::uint8_t& pResultOut) -> bool
    {
        if (pBit >= 8)
        {
            pCore.RaiseException(Exception::InvalidArgument);
            return false;
        }

        pResultOut = (pValue & (1 << pBit));
        return true;
    }

    auto Executive::PerformTOG8 (Core& pCore, std::uint8_t pValue, std::uint8_t pBit, std::uint8_t& pResultOut) -> bool
    {
        if (pBit >= 8)
        {
            pCore.RaiseException(Exception::InvalidArgument);
            return false;
        }

        pResultOut = (pValue ^ (1 << pBit));
        return true;
    }
}
