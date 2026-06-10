/**
 * @file    G10.ASM.Codegen.Instructions.cpp
 * @brief   Contains implementations for the G10 Assembler Code Generator's
 *          instruction dispatch methods, and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Codegen.hpp>

// Private Methods - Instruction Dispatch **************************************

namespace G10::ASM
{
    auto Codegen::DispatchNOP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x0000 NOP`
        // - Does nothing.
        return EmitOpcode(0x0000);
    }

    auto Codegen::DispatchSTOP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x0100 STOP`
        // - Does one of the following:
        // - Places the G10 CPU into the `STOP` state.
        // - If the CPU's `SPD.0` bit is set (speed switch is armed), then
        //   initiates a speed switch instead.
        return EmitOpcode(0x0100);
    }

    auto Codegen::DispatchHALT (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x0200 HALT`
        // - Places the G10 CPU into the `HALT` state.
        return EmitOpcode(0x0200);
    }

    auto Codegen::DispatchDI (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x0300 DI`
        // - Disables CPU interrupts.
        return EmitOpcode(0x0300);
    }

    auto Codegen::DispatchEI (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x0400 EI`
        // - Enables CPU interrupts after a delay of one instruction.
        return EmitOpcode(0x0400);
    }

    auto Codegen::DispatchEII (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x0500 EII`
        // - Enables CPU interrupts immediately.
        return EmitOpcode(0x0500);
    }

    auto Codegen::DispatchDAA (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x0600 DAA`
        // - Decimal-adjusts the low byte accumulator register.
        return EmitOpcode(0x0600);
    }

    auto Codegen::DispatchSCF (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x0700 SCF`
        // - Sets the carry flag.
        return EmitOpcode(0x0700);
    }

    auto Codegen::DispatchCCF (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x0800 CCF`
        // - Compliments (inverts) the carry flag.
        return EmitOpcode(0x0800);
    }

    auto Codegen::DispatchCLV (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x0900 CLV`
        // - Clears the overflow flag.
        return EmitOpcode(0x0900);
    }

    auto Codegen::DispatchSEV (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x0A00 SEV`
        // - Sets the overflow flag.
        return EmitOpcode(0x0A00);
    }

    auto Codegen::DispatchREX (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x0BXY REX XY`
        // - Raises Exception with Code `XY`
        // - 1 Operand
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.REX' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Integer Expression (Byte)
        if (const auto int1 = EvaluateIntegerExpression(op1))
        {
            return EmitOpcode(0x0B00 | (*int1 & 0xFF));
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.REX' Instruction: "
                "Expected integer expression.");
            return false;
        }
    }

    auto Codegen::DispatchLEC (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x0CX0 LEC LX`
        // - Load Exception Code into Byte Register `LX`.
        // - 1 Operand
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.LEC' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression (Low Byte Register)
        if (const auto& reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            return EmitOpcode(0x0C, regIndex1, 0);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.LEC' Instruction: "
                "Expected register expression.");
            return false;
        }
    }

    auto Codegen::DispatchLD (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x10X0 LD LX, IMM8`
        // `0x11X0 LD LX, [IMM32]`
        // `0x12XY LD LX, [DY]`   
        // `0x20X0 LD WX, IMM16`
        // `0x21X0 LD WX, [IMM32]`
        // `0x22XY LD WX, [DY]`
        // `0x30X0 LD DX, IMM32`
        // `0x31X0 LD DX, [IMM32]`
        // `0x32XY LD DX, [DY]`
        // - Loads Immediate or Memory Value into Register
        // - 2 Operands
        // - Can also act as an alias for the `MV` and `ST` instructions.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.LD' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Store Base Opcode and Parameters
        std::uint8_t    base = 0x00;
        std::uint8_t    param1 = 0x00;
        std::uint8_t    param2 = 0x00;

        // Operand 1:
        // - Pointer Expression (redirect to `ST` instruction).
        // - Register Expression
        CPU::RegisterAccess destRegClass {};
        if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            return DispatchST(pNode, pCtx);
        }
        else if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);

            switch (regClass1)
            {
                case CPU::RegisterAccess::LowByte:      base = 0x10; break;
                case CPU::RegisterAccess::Word:         base = 0x20; break;
                case CPU::RegisterAccess::DoubleWord:   base = 0x30; break;
                default:
                    mDiag.ReportError(pNode.mLocation, "'.LD' Instruction: "
                        "Cannot use high-byte register class.");
                    return false;
            }

            destRegClass = regClass1;
            param1 = regIndex1;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.LD' Instruction: "
                "Expected pointer or register expression.");
            return false;
        }

        // Operand 2 (one of the following):
        // - Register Expression (redirect to `MV` instruction).
        // - Binary Expression
        // - Label Expression (Resolved or Not)
        // - Integer Expression
        // - Pointer Expression (Address or Register)
        if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            return DispatchMV(pNode, pCtx);
        }
        else if (const auto bin2 = EvaluateBinaryExpression(op2))
        {
            EmitOpcode(base, param1, param2);
            switch (destRegClass)
            {
                case CPU::RegisterAccess::LowByte:      return EmitByte(*bin2 & 0xFF);
                case CPU::RegisterAccess::Word:         return EmitWord(*bin2 & 0xFFFF);
                case CPU::RegisterAccess::DoubleWord:   return EmitDoubleWord(*bin2);
            }
        }
        else if (const auto lbl2 = stx::to<LabelExpressionNode>(op2))
        {
            EmitOpcode(base, param1, param2);
            switch (destRegClass)
            {
                case CPU::RegisterAccess::LowByte:
                    return AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 1);
                case CPU::RegisterAccess::Word:
                    return AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 2);
                case CPU::RegisterAccess::DoubleWord:
                    return AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 4);
            }
        }
        else if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            EmitOpcode(base, param1, param2);
            switch (destRegClass)
            {
                case CPU::RegisterAccess::LowByte:      return EmitByte(*int2 & 0xFF);
                case CPU::RegisterAccess::Word:         return EmitWord(*int2 & 0xFFFF);
                case CPU::RegisterAccess::DoubleWord:   return EmitDoubleWord(*int2);
            }
        }
        else if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (const auto addr2 = std::get_if<std::uint32_t>(&ptr2.value()))
            {
                return
                    EmitOpcode(base + 1, param1, param2) &&
                    EmitDoubleWord(*addr2);
            }
            else if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.LD' Instruction: "
                        "Invalid register class for pointer operand.");
                    return false;
                }

                param2 = regIndex2;
                return EmitOpcode(base + 2, param1, param2);
            }
            else if (const auto lbl2 = std::get_if<std::string>(&ptr2.value()))
            {
                return
                    EmitOpcode(base + 1, param1, param2) &&
                    AddRelocation(*lbl2, ObjectRelocationType::Absolute, 4);
            }
        }

        return true;
    }

    auto Codegen::DispatchLDQ (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x13X0 LDQ LX, [IMM16]`
        // `0x14XY LDQ LX, [WY]`
        // `0x23X0 LDQ WX, [IMM16]`
        // `0x24XY LDQ WX, [WY]`
        // `0x33X0 LDQ DX, [IMM16]`
        // `0x34XY LDQ DX, [WY]`
        // - Load value from 16-bit relative address.
        // - Address is relative to absolute address `$FFFF0000`.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.LDQ' Instruction: "
                "Expected two operands");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Base and Params
        std::uint8_t base = 0, param1 = 0, param2 = 0;

        // Operand 1:
        // - Register Expression
        CPU::RegisterAccess destRegClass {};
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);

            destRegClass = regClass1;
            param1 = regIndex1;
            switch (destRegClass)
            {
                case CPU::RegisterAccess::LowByte:      base = 0x13; break;
                case CPU::RegisterAccess::Word:         base = 0x23; break;
                case CPU::RegisterAccess::DoubleWord:   base = 0x33; break;
                default:
                    mDiag.ReportError(pNode.mLocation, "'.LDQ' Instruction: "
                        "Invalid register class for destination operand.");
                    return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.LDQ' Instruction: "
                "Expected a register expression");
            return false;
        }

        // Operand 2:
        // - Pointer Expression
        if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (const auto addr2 = std::get_if<std::uint32_t>(&ptr2.value()))
            {
                return
                    EmitOpcode(base, param1, param2) &&
                    EmitWord(*addr2 & 0xFFFF);
            }
            else if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::Word)
                {
                    mDiag.ReportError(pNode.mLocation, "'.LDQ' Instruction: "
                        "Invalid register class for pointer operand.");
                    return false;
                }

                param2 = regIndex2;
                return EmitOpcode(base + 1, param1, param2);                
            }
            else if (const auto lbl2 = std::get_if<std::string>(&ptr2.value()))
            {
                return
                    EmitOpcode(base, param1, param2) &&
                    AddRelocation(*lbl2, ObjectRelocationType::Absolute, 2);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.LDQ' Instruction: "
                    "Invalid pointer operand.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.LDQ' Instruction: "
                "Expected a pointer expression");
            return false;
        }
    }

    auto Codegen::DispatchLDP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x15X0 LDP LX, [IMM8]`
        // `0x16XY LDP LX, [LY]`
        // - Load value from relative 8-bit address.
        // - Address is relative to absolute address `$FFFFFF00`.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.LDP' Instruction: "
                "Expected two operands");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Base and Params
        std::uint8_t base = 0, param1 = 0, param2 = 0;

        // Operand 1:
        // - Register Expression
        CPU::RegisterAccess destRegClass {};
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);

            destRegClass = regClass1;
            param1 = regIndex1;
            switch (destRegClass)
            {
                case CPU::RegisterAccess::LowByte:      base = 0x15; break;
                default:
                    mDiag.ReportError(pNode.mLocation, "'.LDP' Instruction: "
                        "Invalid register class for destination operand.");
                    return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.LDP' Instruction: "
                "Expected a register expression");
            return false;
        }

        // Operand 2:
        // - Pointer Expression
        if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (const auto addr2 = std::get_if<std::uint32_t>(&ptr2.value()))
            {
                return
                    EmitOpcode(base, param1, param2) &&
                    EmitByte(*addr2 & 0xFF);
            }
            else if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::LowByte)
                {
                    mDiag.ReportError(pNode.mLocation, "'.LDP' Instruction: "
                        "Invalid register class for pointer operand.");
                    return false;
                }

                param2 = regIndex2;
                return EmitOpcode(base + 1, param1, param2);                
            }
            else if (const auto lbl2 = std::get_if<std::string>(&ptr2.value()))
            {
                return
                    EmitOpcode(base, param1, param2) &&
                    AddRelocation(*lbl2, ObjectRelocationType::Absolute, 1);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.LDP' Instruction: "
                    "Invalid pointer operand.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.LDP' Instruction: "
                "Expected a pointer expression");
            return false;
        }
    }

    auto Codegen::DispatchST (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x170Y ST [IMM32], LY`
        // `0x18XY ST [DX], LY`
        // `0x270Y ST [IMM32], WY`
        // `0x28XY ST [DX], WY`
        // `0x370Y ST [IMM32], DY`
        // `0x380Y ST [DX], DY`
        // `0xB5X0 ST [DX], IMM8`
        // - Stores value in memory at pointed address
        // - 2 operands
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.ST' Instruction: "
                "Expected two operands");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Opcode and Params
        std::uint8_t    base = 0x00,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 2:
        // - Register Expression
        // - Integer Expression (Byte)
        std::optional<CPU::RegisterAccess> srcMaybeReg {};
        std::optional<std::uint8_t> srcMaybeByte {};
        std::optional<std::string> srcMaybeLabel {};
        if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            srcMaybeReg = regClass2;
            param2 = regIndex2;

            switch (regClass2)
            {
                case CPU::RegisterAccess::LowByte:      base = 0x17; break;
                case CPU::RegisterAccess::Word:         base = 0x27; break;
                case CPU::RegisterAccess::DoubleWord:   base = 0x37; break;
                default:
                    mDiag.ReportError(pNode.mLocation, "'.ST' Instruction: "
                        "Invalid register class for operand 2");
                    return false;
            }
        }
        else if (const auto bin2 = EvaluateBinaryExpression(op2))
        {
            base = 0xB5;
            srcMaybeByte = (*bin2 & 0xFF);
        }
        else if (const auto lbl2 = stx::to<LabelExpressionNode>(op2))
        {
            base = 0xB5;
            srcMaybeLabel = lbl2->mSymbol;
        }
        else if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            base = 0xB5;
            srcMaybeByte = (*int2 & 0xFF);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.ST' Instruction: "
                "Invalid operand 2");
            return false;
        }

        // Operand 1:
        // - Pointer Expression (Address or Register)
        if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto int1 = std::get_if<std::uint32_t>(&ptr1.value()))
            {
                if (base == 0xB5)
                {
                    mDiag.ReportError(pNode.mLocation, "'.ST' Instruction: "
                        "Invalid operand 1");
                    return false;
                }

                return
                    EmitOpcode(base, param1, param2) &&
                    EmitDoubleWord(*int1);
            }
            else if (const auto lbl1 = std::get_if<std::string>(&ptr1.value()))
            {
                if (base == 0xB5)
                {
                    mDiag.ReportError(pNode.mLocation, "'.ST' Instruction: "
                        "Invalid operand 1");
                    return false;
                }

                return
                    EmitOpcode(base, param1, param2) &&
                    AddRelocation(*lbl1, ObjectRelocationType::Absolute, 4);
            }
            else if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.ST' Instruction: "
                        "Invalid register class for operand 1");
                    return false;
                }

                param1 = regIndex1;
                if (srcMaybeReg.has_value() == true)
                {
                    return EmitOpcode(base + 1, param1, param2);
                }
                else if (srcMaybeByte.has_value() == true)
                {
                    return 
                        EmitOpcode(base, param1, param2) &&
                        EmitByte(*srcMaybeByte);
                }
                else if (srcMaybeLabel.has_value() == true)
                {
                    return 
                        EmitOpcode(base, param1, param2) &&
                        AddRelocation(*srcMaybeLabel, ObjectRelocationType::Absolute, 1);
                }
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.ST' Instruction: "
                "Expected a pointer expression for operand 1");
            return false;
        }

        return true;
    }

    auto Codegen::DispatchSTQ (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x190Y STQ [IMM16], LY`
        // `0x1AXY STQ [WX], LY`
        // `0x290Y STQ [IMM16], WY`
        // `0x2AXY STQ [WX], WY`
        // `0x390Y STQ [IMM16], DY`
        // `0x3AXY STQ [WX], DY`
        // - Stores a value from a register to memory at relative 16-bit address.
        // - Address is relative to absolute address `$FFFF0000`.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.STQ' Instruction: "
                "Expected two operands");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Opcode and Params
        std::uint8_t    base = 0x00,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 2:
        // - Register Expression
        CPU::RegisterAccess srcReg {};
        if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            srcReg = regClass2;
            param2 = regIndex2;

            switch (regClass2)
            {
                case CPU::RegisterAccess::LowByte:      base = 0x19; break;
                case CPU::RegisterAccess::Word:         base = 0x29; break;
                case CPU::RegisterAccess::DoubleWord:   base = 0x39; break;
                default:
                    mDiag.ReportError(pNode.mLocation, "'.STQ' Instruction: "
                        "Invalid register class for operand 2");
                    return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.STQ' Instruction: "
                "Expected a register expression for operand 2");
            return false;
        }

        // Operand 1:
        // - Pointer Expression (Address or Register)
        if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto int1 = std::get_if<std::uint32_t>(&ptr1.value()))
            {
                return
                    EmitOpcode(base, param1, param2) &&
                    EmitWord(*int1 & 0xFFFF);
            }
            else if (const auto lbl1 = std::get_if<std::string>(&ptr1.value()))
            {
                return
                    EmitOpcode(base, param1, param2) &&
                    AddRelocation(*lbl1, ObjectRelocationType::Absolute, 2);
            }
            else if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::Word)
                {
                    mDiag.ReportError(pNode.mLocation, "'.STQ' Instruction: "
                        "Invalid register class for operand 1");
                    return false;
                }

                param1 = regIndex1;
                return EmitOpcode(base + 1, param1, param2);
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.STQ' Instruction: "
                "Expected a pointer expression for operand 1");
            return false;
        }

        return true;
    }

    auto Codegen::DispatchSTP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x1B0Y STP [IMM8], LY`
        // `0x1CXY STP [LX], LY`
        // - Stores a value in memory at a relative 8-bit address.
        // - Address is relative to absolute address `$FFFFFF00`.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.STP' Instruction: "
                "Expected two operands");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Opcode and Params
        std::uint8_t    base = 0x00,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 2:
        // - Register Expression
        CPU::RegisterAccess srcReg {};
        if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.STP' Instruction: "
                    "Invalid register class for operand 2");
                return false;
            }

            base = 0x1B;
            srcReg = regClass2;
            param2 = regIndex2;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.STP' Instruction: "
                "Expected a register expression for operand 2");
            return false;
        }

        // Operand 1:
        // - Pointer Expression (Address or Register)
        if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto int1 = std::get_if<std::uint32_t>(&ptr1.value()))
            {
                return
                    EmitOpcode(base, param1, param2) &&
                    EmitByte(*int1 & 0xFF);
            }
            else if (const auto lbl1 = std::get_if<std::string>(&ptr1.value()))
            {
                return
                    EmitOpcode(base, param1, param2) &&
                    AddRelocation(*lbl1, ObjectRelocationType::Absolute, 1);
            }
            else if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::LowByte)
                {
                    mDiag.ReportError(pNode.mLocation, "'.STP' Instruction: "
                        "Invalid register class for operand 1");
                    return false;
                }

                param1 = regIndex1;
                return EmitOpcode(base + 1, param1, param2);
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.STP' Instruction: "
                "Expected a pointer expression for operand 1");
            return false;
        }

        return true;
    }

    auto Codegen::DispatchMV (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x1DXY MV LX, LY`
        // `0x1EXY MV HX, LY`
        // `0x1FXY MV LX, HY`
        // `0x2DXY MV WX, WY`
        // `0x3DXY MV DX, DY`
        // - Copies a value from one register to another.
        // - 2 operands
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.MV' Instruction: "
                "Expected two operands");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Base and Params
        std::uint8_t base = 0;
        std::uint8_t param1 = 0;
        std::uint8_t param2 = 0;

        // Operand 1:
        // - Register Expression
        CPU::RegisterAccess destRegClass {};
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            destRegClass = regClass1;
            param1 = regIndex1;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.MV' Instruction: "
                "Expected a register expression for operand 1");
            return false;
        }

        // Operand 2:
        // - Register Expression
        CPU::RegisterAccess srcRegClass {};
        if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            srcRegClass = regClass2;
            param2 = regIndex2;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.MV' Instruction: "
                "Expected a register expression for operand 2");
            return false;
        }

        if (destRegClass == CPU::RegisterAccess::LowByte &&
            srcRegClass == CPU::RegisterAccess::LowByte)
            { base = 0x1D; }
        else if (destRegClass == CPU::RegisterAccess::HighByte &&
            srcRegClass == CPU::RegisterAccess::LowByte)
            { base = 0x1E; }
        else if (destRegClass == CPU::RegisterAccess::LowByte &&
            srcRegClass == CPU::RegisterAccess::HighByte)
            { base = 0x1F; }
        else if (destRegClass == CPU::RegisterAccess::Word &&
            srcRegClass == CPU::RegisterAccess::Word)
            { base = 0x2D; }
        else if (destRegClass == CPU::RegisterAccess::DoubleWord &&
            srcRegClass == CPU::RegisterAccess::DoubleWord)
            { base = 0x3D; }
        else if (destRegClass == CPU::RegisterAccess::DoubleWord &&
            srcRegClass == CPU::RegisterAccess::Word)
            { base = 0x2E; }
        else if (destRegClass == CPU::RegisterAccess::Word &&
            srcRegClass == CPU::RegisterAccess::DoubleWord)
            { base = 0x2F; }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.MV' Instruction: "
                "Incompatible register classes");
            return false;
        }

        return EmitOpcode(base, param1, param2);
    }

    auto Codegen::DispatchMWH (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x2EXY MWH DX, WY`
        // - Moves `WY` into the upper 16 bits of `DX`
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.MWH' Instruction: "
                "Expected two operands");
            return false;
        }

        // Opcode and Params
        std::uint8_t    base = 0x00,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 1:
        // - Register Expression (Double Word)
        if (const auto reg1 = EvaluateRegisterExpression(pNode.mOperands[0]))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::DoubleWord)
            {
                mDiag.ReportError(pNode.mLocation, "'.MWH' Instruction: "
                    "Expected a double-word register expression for operand 1");
                return false;
            }
            param1 = regIndex1;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.MWH' Instruction: "
                "Expected a register expression for operand 1");
            return false;
        }

        // Operand 2:
        // - Register Expression (Word)
        if (const auto reg2 = EvaluateRegisterExpression(pNode.mOperands[1]))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::Word)
            {
                mDiag.ReportError(pNode.mLocation, "'.MWH' Instruction: "
                    "Expected a word register expression for operand 2");
                return false;
            }
            param2 = regIndex2;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.MWH' Instruction: "
                "Expected a register expression for operand 2");
            return false;
        }

        base = 0x2E;
        return EmitOpcode(base, param1, param2);
    }

    auto Codegen::DispatchMWL (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x2FXY MWL WX, DY`
        // - Moves the upper 16 bits of `DY` into `WX`.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.MWL' Instruction: "
                "Expected two operands");
            return false;
        }

        // Opcode and Params
        std::uint8_t    base = 0x00,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 1:
        // - Register Expression (Word)
        if (const auto reg1 = EvaluateRegisterExpression(pNode.mOperands[0]))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::Word)
            {
                mDiag.ReportError(pNode.mLocation, "'.MWL' Instruction: "
                    "Expected a word register expression for operand 1");
                return false;
            }
            param1 = regIndex1;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.MWL' Instruction: "
                "Expected a register expression for operand 1");
            return false;
        }

        // Operand 2:
        // - Register Expression (Double Word)
        if (const auto reg2 = EvaluateRegisterExpression(pNode.mOperands[1]))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::DoubleWord)
            {
                mDiag.ReportError(pNode.mLocation, "'.MWL' Instruction: "
                    "Expected a double-word register expression for operand 2");
                return false;
            }
            param2 = regIndex2;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.MWL' Instruction: "
                "Expected a register expression for operand 2");
            return false;
        }

        base = 0x2F;
        return EmitOpcode(base, param1, param2);
    }

    auto Codegen::DispatchLSP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x3500 LSP IMM32`
        // - Loads an immediate 32-bit value into the stack pointer.
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.LSP' Instruction: "
                "Expected one operand");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Integer Expression
        if (const auto bin1 = EvaluateBinaryExpression(op1))
        {
            return EmitOpcode(0x3500) && EmitDoubleWord(*bin1);
        }
        else if (const auto int1 = EvaluateIntegerExpression(op1))
        {
            return EmitOpcode(0x3500) && EmitDoubleWord(*int1);
        }
        else if (const auto lbl1 = stx::to<LabelExpressionNode>(op1))
        {
            return
                EmitOpcode(0x3500) &&
                AddRelocation(lbl1->mSymbol, ObjectRelocationType::Absolute, 4);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.LSP' Instruction: "
                "Expected an integer expression");
            return false;
        }
    }

    auto Codegen::DispatchSSP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x3B00 SSP [IMM32]`
        // - Stores the stack pointer's value in memory.
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.SSP' Instruction: "
                "Expected one operand");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Integer Expression
        if (const auto bin1 = EvaluateBinaryExpression(op1))
        {
            return EmitOpcode(0x3B00) && EmitDoubleWord(*bin1);
        }
        else if (const auto int1 = EvaluateIntegerExpression(op1))
        {
            return EmitOpcode(0x3B00) && EmitDoubleWord(*int1);
        }
        else if (const auto lbl1 = stx::to<LabelExpressionNode>(op1))
        {
            return
                EmitOpcode(0x3B00) &&
                AddRelocation(lbl1->mSymbol, ObjectRelocationType::Absolute, 4);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.SSP' Instruction: "
                "Expected an integer expression");
            return false;
        }
    }

    auto Codegen::DispatchPUSH (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x3C0Y PUSH DY`
        // - Pushes value of register `DY` onto the stack.
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.PUSH' Instruction: "
                "Expected one operand");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression (Double Word only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::DoubleWord)
            {
                mDiag.ReportError(pNode.mLocation, "'.PUSH' Instruction: "
                    "Expected a double-word register expression");
                return false;
            }

            return EmitOpcode(0x3C, 0, regIndex1);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.PUSH' Instruction: "
                "Expected a register expression");
            return false;
        }
    }

    auto Codegen::DispatchPOP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x36X0 POP DX`
        // - Pops value from stack and moves it into register `DX`.
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.POP' Instruction: "
                "Expected one operand");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression (Double Word only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::DoubleWord)
            {
                mDiag.ReportError(pNode.mLocation, "'.POP' Instruction: "
                    "Expected a double-word register expression");
                return false;
            }

            return EmitOpcode(0x36, regIndex1, 0);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.POP' Instruction: "
                "Expected a register expression");
            return false;
        }
    }

    auto Codegen::DispatchSPO (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x3EX0 SPO DX`
        // - Moves the stack pointer into register `DX`
        // - 1 operand
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.SPO' Instruction: "
                "Expected one operand");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression (Double Word only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::DoubleWord)
            {
                mDiag.ReportError(pNode.mLocation, "'.SPO' Instruction: "
                    "Expected a double-word register expression");
                return false;
            }

            return EmitOpcode(0x3E, regIndex1, 0);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.SPO' Instruction: "
                "Expected a register expression");
            return false;
        }
    }

    auto Codegen::DispatchSPI (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x3F0Y SPI DY`
        // - Moves register `DY` into the stack pointer.
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.SPI' Instruction: "
                "Expected one operand");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression (Double Word only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::DoubleWord)
            {
                mDiag.ReportError(pNode.mLocation, "'.SPI' Instruction: "
                    "Expected a double-word register expression");
                return false;
            }

            return EmitOpcode(0x3F, regIndex1, 0);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.SPI' Instruction: "
                "Expected a register expression");
            return false;
        }
    }

    auto Codegen::DispatchJMP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x40X0 JMP X, IMM32`
        // `0x41XY JMP X, DY`
        // - Jumps to a specified address, given condition `X`.
        // - 1 or 2 operands.
        if (pNode.mOperands.size() < 1 || pNode.mOperands.size() > 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.JMP' Instruction: "
                "Expected 1 or 2 operands");
            return false;
        }

        // Opcode and Params
        std::uint8_t    opcode = 0x40,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 1 (optional):
        // - Condition Expression
        // - If omitted, assume condition `NC`.
        CPU::Condition cond = CPU::Condition::NC;
        std::uint8_t addrOperIndex = 0;
        if (pNode.mOperands.size() == 2)
        {
            addrOperIndex = 1;
            if (const auto cond1 = EvaluateConditionExpression(pNode.mOperands[0]))
            {
                cond = cond1->second;
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.JMP' Instruction: "
                    "Expected a condition expression");
                return false;
            }
        }
        param1 = std::to_underlying(cond);

        // Operand 2:
        // - Integer Expression
        // - Register Expression
        // - Unresolved Label
        const auto& op2 = pNode.mOperands[addrOperIndex];
        if (const auto bin2 = EvaluateBinaryExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitDoubleWord(*bin2);
        }
        else if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitDoubleWord(*int2);
        }
        else if (const auto lbl2 = stx::to<LabelExpressionNode>(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 4);
        }
        else if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::DoubleWord)
            {
                mDiag.ReportError(pNode.mLocation, "'.JMP' Instruction: "
                    "Expected a double-word register expression");
                return false;
            }

            param2 = regIndex2;
            return EmitOpcode(opcode, param1, param2);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.JMP' Instruction: "
                "Expected an integer, register, or label expression");
            return false;
        }
    }

    auto Codegen::DispatchJPB (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x42X0 JPB X, SIMM16`
        // - Jump by a specified offset, given condition `X`
        // - 1 or 2 operands.
        if (pNode.mOperands.size() < 1 || pNode.mOperands.size() > 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.JPB' Instruction: "
                "Expected 1 or 2 operands");
            return false;
        }

        // Opcode and Params
        std::uint8_t    opcode = 0x42,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 1 (optional):
        // - Condition Expression
        // - If omitted, assume condition `NC`.
        CPU::Condition cond = CPU::Condition::NC;
        std::uint8_t addrOperIndex = 0;
        if (pNode.mOperands.size() == 2)
        {
            addrOperIndex = 1;
            if (const auto cond1 = EvaluateConditionExpression(pNode.mOperands[0]))
            {
                cond = cond1->second;
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.JPB' Instruction: "
                    "Expected a condition expression");
                return false;
            }
        }
        param1 = std::to_underlying(cond);

        // Operand 2:
        // - Label Expression
        // - Integer Expression
        // - Unresolved Label
        const auto& op2 = pNode.mOperands[addrOperIndex];
        if (const auto lbl2 = EvaluateLabelExpression(op2))
        {
            // In the case of a resolved label, the offset is calculated based
            // on the difference between the label's resolved address, and the
            // position of the active section's location counter.
            std::int64_t offset = static_cast<std::int64_t>(lbl2->second) - 
                (pCtx.mLocationCounter + 4);
            if (pCtx.mHeader.mTargetAddress < 0xFFFFFFFF)
            {
                offset -= pCtx.mHeader.mTargetAddress;
            }

            if (offset < -32768 || offset > 32767)
            {
                mDiag.ReportError(pNode.mLocation, "'.JPB' Instruction: "
                    "Label offset is out of range ({})", offset);
                return false;
            }

            return
                EmitOpcode(opcode, param1, param2) &&
                EmitWord(static_cast<std::uint16_t>(offset));
        }
        else if (const auto bin2 = EvaluateBinaryExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitWord(*bin2 & 0xFFFF);
        }
        else if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitWord(*int2 & 0xFFFF);
        }
        else if (const auto lbl2 = stx::to<LabelExpressionNode>(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                AddRelocation(lbl2->mSymbol, ObjectRelocationType::Relative, 2);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.JPB' Instruction: "
                "Expected a label or integer expression");
            return false;
        }
    }

    auto Codegen::DispatchCALL (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x43X0 CALL X, IMM32`
        // - Calls a subroutine at a specified address, given condition `X`.
        // - 1 or 2 operands.
        if (pNode.mOperands.size() < 1 || pNode.mOperands.size() > 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.CALL' Instruction: "
                "Expected 1 or 2 operands");
            return false;
        }

        // Opcode and Params
        std::uint8_t    opcode = 0x43,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 1 (optional):
        // - Condition Expression
        // - If omitted, assume condition `NC`.
        CPU::Condition cond = CPU::Condition::NC;
        std::uint8_t addrOperIndex = 0;
        if (pNode.mOperands.size() == 2)
        {
            addrOperIndex = 1;
            if (const auto cond1 = EvaluateConditionExpression(pNode.mOperands[0]))
            {
                cond = cond1->second;
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.CALL' Instruction: "
                    "Expected a condition expression");
                return false;
            }
        }
        param1 = std::to_underlying(cond);

        // Operand 2:
        // - Integer Expression
        // - Unresolved Label
        const auto& op2 = pNode.mOperands[addrOperIndex];
        if (const auto bin2 = EvaluateBinaryExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitDoubleWord(*bin2);
        }
        else if (const auto lbl2 = stx::to<LabelExpressionNode>(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 4);
        }
        else if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitDoubleWord(*int2);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.CALL' Instruction: "
                "Expected an integer or label expression");
            return false;
        }
    }

    auto Codegen::DispatchINT (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x44XY INT XY`
        // - Calls the interrupt service routine at line index `XY`
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.INT' Instruction: "
                "Expected one operand");
            return false;
        }

        // Operand 1:
        // - Integer Expression (between 0 and 31).
        const auto& op1 = pNode.mOperands[0];
        if (const auto int1 = EvaluateIntegerExpression(op1))
        {
            if (*int1 < 0 || *int1 > 31)
            {
                mDiag.ReportError(pNode.mLocation, "'.INT' Instruction: "
                    "Expected an integer expression between 0 and 31");
                return false;
            }

            return EmitOpcode(0x4400 | static_cast<std::uint8_t>(*int1));
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.INT' Instruction: "
                "Expected an integer expression");
            return false;
        }
    }

    auto Codegen::DispatchRET (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x45X0 RET X`
        // - Returns from a subroutine, given condition `X`
        // - 0 or 1 operands.
        if (pNode.mOperands.size() > 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.RET' Instruction: "
                "Expected 0 or 1 operands");
            return false;
        }

        // Operand 1 (optional):
        // - Condition Expression
        // - If omitted, assume condition `NC`.
        CPU::Condition cond = CPU::Condition::NC;
        if (pNode.mOperands.size() == 1)
        {
            if (const auto cond1 = EvaluateConditionExpression(pNode.mOperands[0]))
            {
                cond = cond1->second;
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.RET' Instruction: "
                    "Expected a condition expression");
                return false;
            }
        }

        return EmitOpcode(0x45, std::to_underlying(cond), 0);
    }

    auto Codegen::DispatchRETI (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x4600 RETI`
        // - Returns from subroutine, clears exception code and enables interrupts.
        return EmitOpcode(0x4600);
    }

    auto Codegen::DispatchMFI (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x4E0Y MFI LY`
        // - Moves byte register `LY` into the Flags Register.
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.MFI' Instruction: "
                "Expected one operand");
            return false;
        }

        const auto& op1 = pNode.mOperands[0];
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.MFI' Instruction: "
                    "Expected a low byte register.");
                return false;
            }
            
            return EmitOpcode(0x4E, 0, regIndex1);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.MFI' Instruction: "
                "Expected a register expression");
            return false;
        }
    }

    auto Codegen::DispatchMFO (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x4FX0 MFO LX`
        // - Moves the Flags Register's value into register `LX`
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.MFO' Instruction: "
                "Expected one operand");
            return false;
        }

        const auto& op1 = pNode.mOperands[0];
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);

            if (regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.MFO' Instruction: "
                    "Expected a low byte register.");
                return false;
            }

            return EmitOpcode(0x4F, regIndex1, 0);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.MFO' Instruction: "
                "Expected a register expression");
            return false;
        }
    }

    auto Codegen::DispatchADD (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x5000 ADD L0, IMM8`
        // `0x510Y ADD L0, LY`
        // `0x520Y ADD L0, [DY]`
        // `0x6000 ADD W0, IMM16`
        // `0x610Y ADD W0, WY`
        // `0x6200 ADD D0, IMM32`
        // `0x630Y ADD D0, DY`
        // - Adds value to accumulator, storing result in accumulator.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.ADD' Instruction: "
                "Expected two operands");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Base Opcode and Parameters
        std::uint8_t    base = 0x00,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 1:
        // - Register Expression
        CPU::RegisterAccess destRegClass {};
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);

            if (regIndex1 > 0 || regClass1 == CPU::RegisterAccess::HighByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.ADD' Instruction: "
                    "Expected an accumulator register.");
                return false;
            }

            switch (regClass1)
            {
                case CPU::RegisterAccess::LowByte:      base = 0x50; break;
                case CPU::RegisterAccess::Word:         base = 0x60; break;
                case CPU::RegisterAccess::DoubleWord:   base = 0x62; break;
            }

            destRegClass = regClass1;          
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.ADD' Instruction: "
                "Expected register expression.");
            return false;
        }

        // Operand 2 (one of the following):
        // - Integer Expression
        // - Register Expression
        // - Pointer Expression (Register; Byte Accumulator Only)
        if (const auto bin2 = EvaluateBinaryExpression(op2))
        {
            EmitOpcode(base, param1, param2);
            switch (destRegClass)
            {
                case CPU::RegisterAccess::LowByte:      return EmitByte(*bin2 & 0xFF);
                case CPU::RegisterAccess::Word:         return EmitWord(*bin2 & 0xFFFF);
                case CPU::RegisterAccess::DoubleWord:   return EmitDoubleWord(*bin2);
            }
        }
        else if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            EmitOpcode(base, param1, param2);
            switch (destRegClass)
            {
                case CPU::RegisterAccess::LowByte:      return EmitByte(*int2 & 0xFF);
                case CPU::RegisterAccess::Word:         return EmitWord(*int2 & 0xFFFF);
                case CPU::RegisterAccess::DoubleWord:   return EmitDoubleWord(*int2);
            }
        }
        else if (const auto lbl2 = stx::to<LabelExpressionNode>(op2))
        {
            EmitOpcode(base, param1, param2);
            switch (destRegClass)
            {
                case CPU::RegisterAccess::LowByte:
                    return AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 1);
                case CPU::RegisterAccess::Word:
                    return AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 2);
                case CPU::RegisterAccess::DoubleWord:
                    return AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 4);
            }
        }
        else if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != destRegClass)
            {
                mDiag.ReportError(pNode.mLocation, "'.ADD' Instruction: "
                    "Register classes do not match.");
                return false;
            }

            return EmitOpcode(base + 1, param1, regIndex2);
        }
        else if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (destRegClass != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.ADD' Instruction: "
                    "Expected byte accumulator for pointer expression.");
                return false;
            }

            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.ADD' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(base + 2, param1, regIndex2);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.ADD' Instruction: "
                    "Expected register in pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.ADD' Instruction: "
                "Expected integer, register, or pointer expression.");
            return false;
        }

        return true;
    }

    auto Codegen::DispatchADC (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x5300 ADC L0, IMM8`
        // `0x540Y ADC L0, LY`
        // `0x550Y ADC L0, [DY]`
        // - Adds value and carry flag to accumulator, storing result in
        //   accumulator.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.ADC' Instruction: "
                "Expected two operands");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Base Opcode and Parameters
        std::uint8_t    base = 0x00,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 1:
        // - Register Expression
        CPU::RegisterAccess destRegClass {};
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);

            if (regIndex1 > 0 || regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.ADC' Instruction: "
                    "Expected the low byte accumulator register.");
                return false;
            }

            base = 0x53;
            destRegClass = regClass1;          
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.ADC' Instruction: "
                "Expected register expression.");
            return false;
        }

        // Operand 2 (one of the following):
        // - Integer Expression
        // - Register Expression
        // - Pointer Expression (Register; Byte Accumulator Only)
        if (const auto bin2 = EvaluateBinaryExpression(op2))
        {
            return
                EmitOpcode(base, param1, param2) &&
                EmitByte(*bin2 & 0xFF);
        }
        else if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            return
                EmitOpcode(base, param1, param2) &&
                EmitByte(*int2 & 0xFF);
        }
        else if (const auto lbl2 = stx::to<LabelExpressionNode>(op2))
        {
            return 
                EmitOpcode(base, param1, param2) &&
                AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 1);
        }
        else if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != destRegClass)
            {
                mDiag.ReportError(pNode.mLocation, "'.ADC' Instruction: "
                    "Register classes do not match.");
                return false;
            }

            return EmitOpcode(base + 1, param1, regIndex2);
        }
        else if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (destRegClass != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.ADC' Instruction: "
                    "Expected byte accumulator for pointer expression.");
                return false;
            }

            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.ADC' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(base + 2, param1, regIndex2);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.ADC' Instruction: "
                    "Expected register in pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.ADC' Instruction: "
                "Expected integer, register, or pointer expression.");
            return false;
        }

        return true;
    }

    auto Codegen::DispatchSUB (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x5600 SUB L0, IMM8`
        // `0x570Y SUB L0, LY`
        // `0x580Y SUB L0, [DY]`
        // `0x6400 SUB W0, IMM16`
        // `0x650Y SUB W0, WY`
        // `0x6600 SUB D0, IMM32`
        // `0x6700 SUB D0, DY`
        // - Subtracts value from accumulator, storing result in accumulator.
        // - 2 operands
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.SUB' Instruction: "
                "Expected two operands");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Base Opcode and Parameters
        std::uint8_t    base = 0x00,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 1:
        // - Register Expression
        CPU::RegisterAccess destRegClass {};
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);

            if (regIndex1 > 0 || regClass1 == CPU::RegisterAccess::HighByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.SUB' Instruction: "
                    "Expected an accumulator register.");
                return false;
            }

            switch (regClass1)
            {
                case CPU::RegisterAccess::LowByte:      base = 0x56; break;
                case CPU::RegisterAccess::Word:         base = 0x64; break;
                case CPU::RegisterAccess::DoubleWord:   base = 0x66; break;
            }

            destRegClass = regClass1;          
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.SUB' Instruction: "
                "Expected register expression.");
            return false;
        }

        // Operand 2 (one of the following):
        // - Integer Expression
        // - Register Expression
        // - Pointer Expression (Register; Byte Accumulator Only)
        if (const auto bin2 = EvaluateBinaryExpression(op2))
        {
            EmitOpcode(base, param1, param2);
            switch (destRegClass)
            {
                case CPU::RegisterAccess::LowByte:      return EmitByte(*bin2 & 0xFF);
                case CPU::RegisterAccess::Word:         return EmitWord(*bin2 & 0xFFFF);
                case CPU::RegisterAccess::DoubleWord:   return EmitDoubleWord(*bin2);
            }
        }
        else if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            EmitOpcode(base, param1, param2);
            switch (destRegClass)
            {
                case CPU::RegisterAccess::LowByte:      return EmitByte(*int2 & 0xFF);
                case CPU::RegisterAccess::Word:         return EmitWord(*int2 & 0xFFFF);
                case CPU::RegisterAccess::DoubleWord:   return EmitDoubleWord(*int2);
            }
        }
        else if (const auto lbl2 = stx::to<LabelExpressionNode>(op2))
        {
            EmitOpcode(base, param1, param2);
            switch (destRegClass)
            {
                case CPU::RegisterAccess::LowByte:
                    return AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 1);
                case CPU::RegisterAccess::Word:
                    return AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 2);
                case CPU::RegisterAccess::DoubleWord:
                    return AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 4);
            }
        }
        else if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != destRegClass)
            {
                mDiag.ReportError(pNode.mLocation, "'.SUB' Instruction: "
                    "Register classes do not match.");
                return false;
            }

            return EmitOpcode(base + 1, param1, regIndex2);
        }
        else if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (destRegClass != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.SUB' Instruction: "
                    "Expected byte accumulator for pointer expression.");
                return false;
            }

            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.SUB' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(base + 2, param1, regIndex2);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.SUB' Instruction: "
                    "Expected register in pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.SUB' Instruction: "
                "Expected integer, register, or pointer expression.");
            return false;
        }

        return true;
    }

    auto Codegen::DispatchSBC (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x5900 SBC L0, IMM8`
        // `0x5A0Y SBC L0, LY`
        // `0x5B0Y SBC L0, [DY]`
        // - Subtracts value and carry from accumulator, storing result in
        //   accumulator.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.SBC' Instruction: "
                "Expected two operands");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Base Opcode and Parameters
        std::uint8_t    base = 0x00,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 1:
        // - Register Expression
        CPU::RegisterAccess destRegClass {};
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);

            if (regIndex1 > 0 || regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.SBC' Instruction: "
                    "Expected the low byte accumulator register.");
                return false;
            }

            base = 0x59;
            destRegClass = regClass1;          
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.SBC' Instruction: "
                "Expected register expression.");
            return false;
        }

        // Operand 2 (one of the following):
        // - Integer Expression
        // - Register Expression
        // - Pointer Expression (Register; Byte Accumulator Only)
        if (const auto bin2 = EvaluateBinaryExpression(op2))
        {
            return
                EmitOpcode(base, param1, param2) &&
                EmitByte(*bin2 & 0xFF);
        }
        else if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            return
                EmitOpcode(base, param1, param2) &&
                EmitByte(*int2 & 0xFF);
        }
        else if (const auto lbl2 = stx::to<LabelExpressionNode>(op2))
        {
            return
                EmitOpcode(base, param1, param2) &&
                AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 1);
        }
        else if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != destRegClass)
            {
                mDiag.ReportError(pNode.mLocation, "'.SBC' Instruction: "
                    "Register classes do not match.");
                return false;
            }

            return EmitOpcode(base + 1, param1, regIndex2);
        }
        else if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (destRegClass != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.SBC' Instruction: "
                    "Expected byte accumulator for pointer expression.");
                return false;
            }

            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.SBC' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(base + 2, param1, regIndex2);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.SBC' Instruction: "
                    "Expected register in pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.SBC' Instruction: "
                "Expected integer, register, or pointer expression.");
            return false;
        }

        return true;
    }

    auto Codegen::DispatchINC (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x5CX0 INC LX`
        // `0x5DX0 INC [DX]`
        // `0x6CX0 INC WX`
        // `0x6DX0 INC DX`
        // - Increments a value.
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.INC' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression
        // - Pointer Expression (Double Word Register)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            switch (regClass1)
            {
                case CPU::RegisterAccess::LowByte:      return EmitOpcode(0x5C, regIndex1, 0);
                case CPU::RegisterAccess::Word:         return EmitOpcode(0x6C, regIndex1, 0);
                case CPU::RegisterAccess::DoubleWord:   return EmitOpcode(0x6D, regIndex1, 0);
                default:
                    mDiag.ReportError(pNode.mLocation, "'.INC' Instruction: "
                        "Unexpected register class.");
                    return false;
            }
        }
        else if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.INC' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(0x5D, regIndex1, 0);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.INC' Instruction: "
                    "Expected register in pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.INC' Instruction: "
                "Expected register or pointer expression.");
            return false;
        }

        return true;
    }

    auto Codegen::DispatchDEC (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x5EX0 DEC LX`
        // `0x5FX0 DEC [DX]`
        // `0x6EX0 DEC WX`
        // `0x6FX0 DEC DX`
        // - Decrements a value.
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.DEC' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression
        // - Pointer Expression (Double Word Register)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            switch (regClass1)
            {
                case CPU::RegisterAccess::LowByte:      return EmitOpcode(0x5E, regIndex1, 0);
                case CPU::RegisterAccess::Word:         return EmitOpcode(0x6E, regIndex1, 0);
                case CPU::RegisterAccess::DoubleWord:   return EmitOpcode(0x6F, regIndex1, 0);
                default:
                    mDiag.ReportError(pNode.mLocation, "'.DEC' Instruction: "
                        "Unexpected register class.");
                    return false;
            }
        }
        else if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.DEC' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(0x5F, regIndex1, 0);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.DEC' Instruction: "
                    "Expected register in pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.DEC' Instruction: "
                "Expected register or pointer expression.");
            return false;
        }

        return true;
    }

    auto Codegen::DispatchAND (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x7000 AND L0, IMM8`
        // `0x710Y AND L0, LY`
        // `0x720Y AND L0, [DY]`
        // - Bitwise ANDs accumulator and value, storing result in accumulator.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.AND' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Opcode and params
        std::uint8_t    opcode = 0x70,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 1:
        // - Register Expression (Low Byte only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regIndex1 > 0 || regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.AND' Instruction: "
                    "Expected low byte accumulator register for register expression.");
                return false;
            }

            param1 = regIndex1;
        }

        // Operand 2:
        // - Integer Expression
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto bin2 = EvaluateBinaryExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitByte(*bin2 & 0xFF);
        }
        else if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitByte(*int2 & 0xFF);
        }
        else if (const auto lbl2 = stx::to<LabelExpressionNode>(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 1);
        }
        else if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.AND' Instruction: "
                    "Expected low byte register for register expression.");
                return false;
            }

            param2 = regIndex2;
            return EmitOpcode(opcode + 1, param1, param2);
        }
        else if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.AND' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                param2 = regIndex2;
                return EmitOpcode(opcode + 2, param1, param2);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.AND' Instruction: "
                    "Expected register for pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.AND' Instruction: "
                "Expected immediate value, register, or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchOR (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x7300 OR L0, IMM8`
        // `0x740Y OR L0, LY`
        // `0x750Y OR L0, [DY]`
        // - Bitwise ORs value and accumulator, storing result in accumulator.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.OR' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Opcode and params
        std::uint8_t    opcode = 0x73,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 1:
        // - Register Expression (Low Byte only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regIndex1 > 0 || regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.OR' Instruction: "
                    "Expected low byte accumulator register for register expression.");
                return false;
            }

            param1 = regIndex1;
        }

        // Operand 2:
        // - Integer Expression
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto bin2 = EvaluateBinaryExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitByte(*bin2 & 0xFF);
        }
        else if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitByte(*int2 & 0xFF);
        }
        else if (const auto lbl2 = stx::to<LabelExpressionNode>(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 1);
        }
        else if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.OR' Instruction: "
                    "Expected low byte register for register expression.");
                return false;
            }

            param2 = regIndex2;
            return EmitOpcode(opcode + 1, param1, param2);
        }
        else if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.OR' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                param2 = regIndex2;
                return EmitOpcode(opcode + 2, param1, param2);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.OR' Instruction: "
                    "Expected register for pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.OR' Instruction: "
                "Expected immediate value, register, or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchXOR (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x7600 XOR L0, IMM8`
        // `0x770Y XOR L0, LY`
        // `0x780Y XOR L0, [DY]`
        // - Bitwise XORs value and accumulator, storing result in accumulator.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.XOR' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Opcode and params
        std::uint8_t    opcode = 0x76,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 1:
        // - Register Expression (Low Byte only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regIndex1 > 0 || regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.XOR' Instruction: "
                    "Expected low byte accumulator register for register expression.");
                return false;
            }

            param1 = regIndex1;
        }

        // Operand 2:
        // - Integer Expression
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto bin2 = EvaluateBinaryExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitByte(*bin2 & 0xFF);
        }
        else if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitByte(*int2 & 0xFF);
        }
        else if (const auto lbl2 = stx::to<LabelExpressionNode>(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 1);
        }
        else if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.XOR' Instruction: "
                    "Expected low byte register for register expression.");
                return false;
            }

            param2 = regIndex2;
            return EmitOpcode(opcode + 1, param1, param2);
        }
        else if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.XOR' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                param2 = regIndex2;
                return EmitOpcode(opcode + 2, param1, param2);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.XOR' Instruction: "
                    "Expected register for pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.XOR' Instruction: "
                "Expected immediate value, register, or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchNOT (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x79X0 NOT LX`
        // `0x7AX0 NOT [DX]`
        // - Compliments (bitwise NOTs) a value.
        // - 0 or 1 operands.
        if (pNode.mOperands.size() > 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.NOT' Instruction: "
                "Expected zero or one operand.");
            return false;
        }
        else if (pNode.mOperands.size() == 0)
        {
            return EmitOpcode(0x7900);  // Treat zero operands as `NOT L0`.
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression
        // - Pointer Expression (Double Word Register)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            switch (regClass1)
            {
                case CPU::RegisterAccess::LowByte:      return EmitOpcode(0x79, regIndex1, 0);
                default:
                    mDiag.ReportError(pNode.mLocation, "'.NOT' Instruction: "
                        "Unexpected register class.");
                    return false;
            }
        }
        else if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.NOT' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(0x7A, regIndex1, 0);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.NOT' Instruction: "
                    "Expected register in pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.NOT' Instruction: "
                "Expected register or pointer expression.");
            return false;
        }

        return true;
    }

    auto Codegen::DispatchCMP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {        
        // `0x7D00 CMP L0, IMM8`
        // `0x7E0Y CMP L0, LY`
        // `0x7F0Y CMP L0, [DY]`
        // - Subtracts value from accumulator, without storing result.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.CMP' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Opcode and params
        std::uint8_t    opcode = 0x7D,
                        param1 = 0x00,
                        param2 = 0x00;

        // Operand 1:
        // - Register Expression (Low Byte only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regIndex1 > 0 || regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.CMP' Instruction: "
                    "Expected low byte accumulator register for register expression.");
                return false;
            }

            param1 = regIndex1;
        }

        // Operand 2:
        // - Integer Expression
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto bin2 = EvaluateBinaryExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitByte(*bin2 & 0xFF);
        }
        else if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                EmitByte(*int2 & 0xFF);
        }
        else if (const auto lbl2 = stx::to<LabelExpressionNode>(op2))
        {
            return
                EmitOpcode(opcode, param1, param2) &&
                AddRelocation(lbl2->mSymbol, ObjectRelocationType::Absolute, 1);
        }
        else if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.CMP' Instruction: "
                    "Expected low byte register for register expression.");
                return false;
            }

            param2 = regIndex2;
            return EmitOpcode(opcode + 1, param1, param2);
        }
        else if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.CMP' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                param2 = regIndex2;
                return EmitOpcode(opcode + 2, param1, param2);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.CMP' Instruction: "
                    "Expected register for pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.CMP' Instruction: "
                "Expected immediate value, register, or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchSLA (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x80X0 SLA LX`
        // `0x81X0 SLA [DX]`
        // - Shifts value left by one bit (arithmetic).
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.SLA' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.SLA' Instruction: "
                    "Expected low byte register for register expression.");
                return false;
            }

            return EmitOpcode(0x80, regIndex1, 0);
        }
        else if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.SLA' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(0x81, regIndex1, 0);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.SLA' Instruction: "
                    "Expected register for pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.SLA' Instruction: "
                "Expected register or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchSRA (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x82X0 SRA LX`
        // `0x83X0 SRA [DX]`
        // - Shifts value right by one bit, preserving sign bit (arithmetic).
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.SRA' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.SRA' Instruction: "
                    "Expected low byte register for register expression.");
                return false;
            }

            return EmitOpcode(0x82, regIndex1, 0);
        }
        else if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.SRA' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(0x83, regIndex1, 0);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.SRA' Instruction: "
                    "Expected register for pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.SRA' Instruction: "
                "Expected register or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchSRL (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x84X0 SRL LX`
        // `0x85X0 SRL [DX]`
        // - Shifts value right by one bit (logical).
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.SRL' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.SRL' Instruction: "
                    "Expected low byte register for register expression.");
                return false;
            }

            return EmitOpcode(0x84, regIndex1, 0);
        }
        else if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.SRL' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(0x85, regIndex1, 0);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.SRL' Instruction: "
                    "Expected register for pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.SRL' Instruction: "
                "Expected register or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchSWAP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x86X0 SWAP LX`
        // `0x87X0 SWAP [DX]`
        // `0x88X0 SWAP WX`
        // `0x89X0 SWAP DX`
        // - Swaps the halves of a value (nibbles of a byte, bytes of a word, etc.)
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.SWAP' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression
        // - Pointer Expression (Double Word Register only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            switch (regClass1)
            {
                case CPU::RegisterAccess::LowByte:
                    return EmitOpcode(0x86, regIndex1, 0);
                case CPU::RegisterAccess::Word:
                    return EmitOpcode(0x88, regIndex1, 0);
                case CPU::RegisterAccess::DoubleWord:
                    return EmitOpcode(0x89, regIndex1, 0);
                default:
                    mDiag.ReportError(pNode.mLocation, "'.SWAP' Instruction: "
                        "Invalid register class for register expression.");
                    return false;
            }
        }
        else if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.SWAP' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(0x87, regIndex1, 0);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.SWAP' Instruction: "
                    "Expected register for pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.SWAP' Instruction: "
                "Expected register or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchRLA (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x9000 RLA`
        // - Rotates byte accumulator left by one bit, through the carry flag.
        return EmitOpcode(0x9000);
    }

    auto Codegen::DispatchRL (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x9100 RL LX`
        // `0x9200 RL [DX]`
        // - Rotates value left by one bit, through the carry flag.
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.RL' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.RL' Instruction: "
                    "Expected low byte register for register expression.");
                return false;
            }

            return EmitOpcode(0x91, regIndex1, 0);
        }
        else if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.RL' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(0x92, regIndex1, 0);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.RL' Instruction: "
                    "Expected register for pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.RL' Instruction: "
                "Expected register or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchRLCA (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x9300 RLCA`
        // - Rotates byte accumulator left by one bit.
        return EmitOpcode(0x9300);
    }

    auto Codegen::DispatchRLC (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x94X0 RLC LX`
        // `0x95X0 RLC [DX]`
        // - Rotates value left by one bit.
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.RLC' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.RLC' Instruction: "
                    "Expected low byte register for register expression.");
                return false;
            }

            return EmitOpcode(0x94, regIndex1, 0);
        }
        else if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.RLC' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(0x95, regIndex1, 0);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.RLC' Instruction: "
                    "Expected register for pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.RLC' Instruction: "
                "Expected register or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchRRA (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x9600 RRA`
        // - Rotates byte accumulator right by one bit, through the carry flag.
        return EmitOpcode(0x9600);
    }

    auto Codegen::DispatchRR (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x97X0 RR LX`
        // `0x98X0 RR [DX]`
        // - Rotates value right by one bit, through the carry flag.
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.RR' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.RR' Instruction: "
                    "Expected low byte register for register expression.");
                return false;
            }

            return EmitOpcode(0x97, regIndex1, 0);
        }
        else if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.RR' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(0x98, regIndex1, 0);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.RR' Instruction: "
                    "Expected register for pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.RR' Instruction: "
                "Expected register or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchRRCA (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x9900 RRCA`
        // - Rotates byte accumulator right by one bit.
        return EmitOpcode(0x9900);
    }

    auto Codegen::DispatchRRC (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0x9AX0 RRC LX`
        // `0x9BX0 RRC [DX]`
        // - Rotates value right by one bit.
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.RRC' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.RRC' Instruction: "
                    "Expected low byte register for register expression.");
                return false;
            }

            return EmitOpcode(0x9A, regIndex1, 0);
        }
        else if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.RRC' Instruction: "
                        "Expected double-word register for pointer expression.");
                    return false;
                }

                return EmitOpcode(0x9B, regIndex1, 0);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.RRC' Instruction: "
                    "Expected register for pointer expression.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.RRC' Instruction: "
                "Expected register or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchBIT (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0xA0XY BIT Y, LX`
        // `0xA1XY BIT Y, [DX]`
        // - Tests one bit in a value.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.BIT' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Operand 1:
        // - Integer Expression (between 0 and 7)
        std::uint8_t bit = 0xFF;
        if (const auto int1 = EvaluateIntegerExpression(op1))
        {
            if (*int1 < 0 || *int1 > 7)
            {
                mDiag.ReportError(pNode.mLocation, "'.BIT' Instruction: "
                    "Bit index must be between 0 and 7.");
                return false;
            }

            bit = static_cast<std::uint8_t>(*int1);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.BIT' Instruction: "
                "Expected an integer expression for bit index.");
            return false;
        }

        // Operand 2:
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.BIT' Instruction: "
                    "Expected a low byte register.");
                return false;
            }

            return EmitOpcode(0xA0, regIndex2, bit);
        }
        else if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.BIT' Instruction: "
                        "Expected a double-word register for pointer operand.");
                    return false;
                }

                return EmitOpcode(0xA1, regIndex2, bit);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.BIT' Instruction: "
                    "Expected register pointer operand.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.BIT' Instruction: "
                "Expected a register or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchSET (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0xA2XY SET Y, LX`
        // `0xA3XY SET Y, [DX]`
        // - Sets one bit in a value
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.SET' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Operand 1:
        // - Integer Expression (between 0 and 7)
        std::uint8_t bit = 0xFF;
        if (const auto int1 = EvaluateIntegerExpression(op1))
        {
            if (*int1 < 0 || *int1 > 7)
            {
                mDiag.ReportError(pNode.mLocation, "'.SET' Instruction: "
                    "Bit index must be between 0 and 7.");
                return false;
            }

            bit = static_cast<std::uint8_t>(*int1);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.SET' Instruction: "
                "Expected an integer expression for bit index.");
            return false;
        }

        // Operand 2:
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.SET' Instruction: "
                    "Expected a low byte register.");
                return false;
            }

            return EmitOpcode(0xA2, regIndex2, bit);
        }
        else if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.SET' Instruction: "
                        "Expected a double-word register for pointer operand.");
                    return false;
                }

                return EmitOpcode(0xA3, regIndex2, bit);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.SET' Instruction: "
                    "Expected register pointer operand.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.SET' Instruction: "
                "Expected a register or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchRES (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0xA4XY RES Y, LX`
        // `0xA5XY RES Y, [DX]`
        // - Resets one bit in a value.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.RES' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Operand 1:
        // - Integer Expression (between 0 and 7)
        std::uint8_t bit = 0xFF;
        if (const auto int1 = EvaluateIntegerExpression(op1))
        {
            if (*int1 < 0 || *int1 > 7)
            {
                mDiag.ReportError(pNode.mLocation, "'.RES' Instruction: "
                    "Bit index must be between 0 and 7.");
                return false;
            }

            bit = static_cast<std::uint8_t>(*int1);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.RES' Instruction: "
                "Expected an integer expression for bit index.");
            return false;
        }

        // Operand 2:
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.RES' Instruction: "
                    "Expected a low byte register.");
                return false;
            }

            return EmitOpcode(0xA4, regIndex2, bit);
        }
        else if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.RES' Instruction: "
                        "Expected a double-word register for pointer operand.");
                    return false;
                }

                return EmitOpcode(0xA5, regIndex2, bit);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.RES' Instruction: "
                    "Expected register pointer operand.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.RES' Instruction: "
                "Expected a register or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchTOG (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0xA6XY TOG Y, LX`
        // `0xA7XY TOG Y, [DX]`
        // - Toggles a bit in a byte value.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.TOG' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Operand 1:
        // - Integer Expression (between 0 and 7)
        std::uint8_t bit = 0xFF;
        if (const auto int1 = EvaluateIntegerExpression(op1))
        {
            if (*int1 < 0 || *int1 > 7)
            {
                mDiag.ReportError(pNode.mLocation, "'.TOG' Instruction: "
                    "Bit index must be between 0 and 7.");
                return false;
            }

            bit = static_cast<std::uint8_t>(*int1);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.TOG' Instruction: "
                "Expected an integer expression for bit index.");
            return false;
        }

        // Operand 2:
        // - Register Expression (Low Byte only)
        // - Pointer Expression (Double Word Register only)
        if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.TOG' Instruction: "
                    "Expected a low byte register.");
                return false;
            }

            return EmitOpcode(0xA6, regIndex2, bit);
        }
        else if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.TOG' Instruction: "
                        "Expected a double-word register for pointer operand.");
                    return false;
                }

                return EmitOpcode(0xA7, regIndex2, bit);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.TOG' Instruction: "
                    "Expected register pointer operand.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.TOG' Instruction: "
                "Expected a register or pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchLDI (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0xB0XY LDI LX, [DY]`
        // - Loads data from register-pointed memory, then increments register.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.LDI' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Operand 1:
        // - Register Expression (Low Byte only)
        std::uint8_t destRegIndex { 0 };
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.LDI' Instruction: "
                    "Expected a low byte register.");
                return false;
            }

            destRegIndex = regIndex1;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.LDI' Instruction: "
                "Expected a register expression.");
            return false;
        }

        // Operand 2:
        // - Pointer Expression (Double Word Register only)
        if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.LDI' Instruction: "
                        "Expected a double-word register for pointer operand.");
                    return false;
                }

                return EmitOpcode(0xB0, destRegIndex, regIndex2);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.LDI' Instruction: "
                    "Expected register pointer operand.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.LDI' Instruction: "
                "Expected a pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchLDD (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0xB1XY LDD LX, [DY]`
        // - Loads data from register-pointed memory, then decrements register.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.LDD' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Operand 1:
        // - Register Expression (Low Byte only)
        std::uint8_t destRegIndex { 0 };
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.LDD' Instruction: "
                    "Expected a low byte register.");
                return false;
            }

            destRegIndex = regIndex1;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.LDD' Instruction: "
                "Expected a register expression.");
            return false;
        }

        // Operand 2:
        // - Pointer Expression (Double Word Register only)
        if (const auto ptr2 = EvaluatePointerExpression(op2))
        {
            if (const auto reg2 = std::get_if<StringAndRegister>(&ptr2.value()))
            {
                const auto regUnder2 = std::to_underlying(reg2->second);
                const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
                const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
                if (regClass2 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.LDD' Instruction: "
                        "Expected a double-word register for pointer operand.");
                    return false;
                }

                return EmitOpcode(0xB1, destRegIndex, regIndex2);
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.LDD' Instruction: "
                    "Expected register pointer operand.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.LDD' Instruction: "
                "Expected a pointer expression.");
            return false;
        }
    }

    auto Codegen::DispatchSTI (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0xB2XY STI [DX], LY`
        // - Stores data from register to register-pointed memory, then
        //   increments pointer register.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.STI' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Operand 1:
        // - Pointer Expression (Double Word Register only)
        std::uint8_t ptrRegIndex { 0 };
        if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.STI' Instruction: "
                        "Expected a double-word register for pointer operand.");
                    return false;
                }

                ptrRegIndex = regIndex1;
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.STI' Instruction: "
                    "Expected register pointer operand.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.STI' Instruction: "
                "Expected a pointer expression.");
            return false;
        }

        // Operand 2:
        // - Register Expression (Low Byte only)
        std::uint8_t srcRegIndex { 0 };
        if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.STI' Instruction: "
                    "Expected a low byte register.");
                return false;
            }

            srcRegIndex = regIndex2;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.STI' Instruction: "
                "Expected a register expression.");
            return false;
        }

        return EmitOpcode(0xB2, ptrRegIndex, srcRegIndex);
    }

    auto Codegen::DispatchSTD (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0xB3XY STD [DX], LY`
        // - Stores data from register to register-pointed memory, then
        //   decrements pointer register.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.STD' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Operand 1:
        // - Pointer Expression (Double Word Register only)
        std::uint8_t ptrRegIndex { 0 };
        if (const auto ptr1 = EvaluatePointerExpression(op1))
        {
            if (const auto reg1 = std::get_if<StringAndRegister>(&ptr1.value()))
            {
                const auto regUnder1 = std::to_underlying(reg1->second);
                const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
                const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
                if (regClass1 != CPU::RegisterAccess::DoubleWord)
                {
                    mDiag.ReportError(pNode.mLocation, "'.STD' Instruction: "
                        "Expected a double-word register for pointer operand.");
                    return false;
                }

                ptrRegIndex = regIndex1;
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.STD' Instruction: "
                    "Expected register pointer operand.");
                return false;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.STD' Instruction: "
                "Expected a pointer expression.");
            return false;
        }

        // Operand 2:
        // - Register Expression (Low Byte only)
        std::uint8_t srcRegIndex { 0 };
        if (const auto reg2 = EvaluateRegisterExpression(op2))
        {
            const auto regUnder2 = std::to_underlying(reg2->second);
            const auto regClass2 = static_cast<CPU::RegisterAccess>(regUnder2 & 0xF0);
            const auto regIndex2 = static_cast<std::uint8_t>(regUnder2 & 0x0F);
            if (regClass2 != CPU::RegisterAccess::LowByte)
            {
                mDiag.ReportError(pNode.mLocation, "'.STD' Instruction: "
                    "Expected a low byte register.");
                return false;
            }

            srcRegIndex = regIndex2;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.STD' Instruction: "
                "Expected a register expression.");
            return false;
        }

        return EmitOpcode(0xB3, ptrRegIndex, srcRegIndex);
    }

    auto Codegen::DispatchASP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0xB400 ASP SIMM8`
        // - Adjusts the stack pointer by the given offset.
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.ASP' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Integer Expression
        if (const auto int1 = EvaluateIntegerExpression(op1))
        {
            return 
                EmitOpcode(0xB400) &&
                EmitByte(*int1 & 0xFF);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.ASP' Instruction: "
                "Expected an integer expression.");
            return false;
        }
    }

    auto Codegen::DispatchLASP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0xB6X0 LASP DX, SIMM8`
        // - Loads stack pointer, adjusted by offset, into register `DX`.
        // - 2 operands.
        if (pNode.mOperands.size() != 2)
        {
            mDiag.ReportError(pNode.mLocation, "'.LASP' Instruction: "
                "Expected two operands.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];
        const auto& op2 = pNode.mOperands[1];

        // Operand 1:
        // - Register Expression (Double Word only)
        std::uint8_t destRegIndex { 0 };
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::DoubleWord)
            {
                mDiag.ReportError(pNode.mLocation, "'.LASP' Instruction: "
                    "Expected a double-word register.");
                return false;
            }

            destRegIndex = regIndex1;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.LASP' Instruction: "
                "Expected a register expression.");
            return false;
        }

        // Operand 2:
        // - Integer Expression
        if (const auto int2 = EvaluateIntegerExpression(op2))
        {
            return
                EmitOpcode(0xB6, destRegIndex, 0) &&
                EmitByte(*int2 & 0xFF);
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.LASP' Instruction: "
                "Expected an integer expression.");
            return false;
        }
    }

    auto Codegen::DispatchISP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0xB700 ISP`
        // - Increments stack pointer.
        return EmitOpcode(0xB700);
    }

    auto Codegen::DispatchDSP (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0xB800 DSP`
        // - Decrements stack pointer.
        return EmitOpcode(0xB800);
    }

    auto Codegen::DispatchASR (const InstructionStatementNode& pNode,
        ObjectSectionContext& pCtx) -> bool
    {
        // `0xB9X0 ASR DX`
        // - Adds the stack pointer to register `DX`
        // - 1 operand.
        if (pNode.mOperands.size() != 1)
        {
            mDiag.ReportError(pNode.mLocation, "'.ASR' Instruction: "
                "Expected one operand.");
            return false;
        }
        const auto& op1 = pNode.mOperands[0];

        // Operand 1:
        // - Register Expression (Double Word only)
        std::uint8_t destRegIndex { 0 };
        if (const auto reg1 = EvaluateRegisterExpression(op1))
        {
            const auto regUnder1 = std::to_underlying(reg1->second);
            const auto regClass1 = static_cast<CPU::RegisterAccess>(regUnder1 & 0xF0);
            const auto regIndex1 = static_cast<std::uint8_t>(regUnder1 & 0x0F);
            if (regClass1 != CPU::RegisterAccess::DoubleWord)
            {
                mDiag.ReportError(pNode.mLocation, "'.ASR' Instruction: "
                    "Expected a double-word register.");
                return false;
            }

            destRegIndex = regIndex1;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, "'.ASR' Instruction: "
                "Expected a register expression.");
            return false;
        }

        return EmitOpcode(0xB9, destRegIndex, 0);
    }
}
