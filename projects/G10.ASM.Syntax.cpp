/**
 * @file    G10.ASM.Syntax.cpp
 * @brief   Contains implementations for the G10 Assembler's syntax node
 *          structures, and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Syntax.hpp>

// Public Methods **************************************************************

namespace G10::ASM
{
    // Expression Nodes ********************************************************

    auto IntegerExpressionNode::Stringify (std::size_t pIndent) const -> std::string
    {
        return std::format("{0}IntegerExpression: {1}\n", 
            std::string(pIndent * 2, ' '), mValue);
    }

    auto StringExpressionNode::Stringify (std::size_t pIndent) const -> std::string
    {
        return std::format("{0}StringExpression: \"{1}\"\n", 
            std::string(pIndent * 2, ' '), mValue);
    }

    auto RegisterExpressionNode::Stringify (std::size_t pIndent) const -> std::string
    {
        return std::format("{0}RegisterExpression: {1}\n", 
            std::string(pIndent * 2, ' '), mName);
    }

    auto ConditionExpressionNode::Stringify (std::size_t pIndent) const -> std::string
    {
        return std::format("{0}ConditionExpression: {1}\n", 
            std::string(pIndent * 2, ' '), mName);
    }

    auto LabelExpressionNode::Stringify (std::size_t pIndent) const -> std::string
    {
        return std::format("{0}LabelExpression: {1}\n", 
            std::string(pIndent * 2, ' '), mSymbol);
    }

    auto SectionNameExpressionNode::Stringify (std::size_t pIndent) const -> std::string
    {
        return std::format("{0}SectionNameExpression: {1}\n", 
            std::string(pIndent * 2, ' '), mName);
    }

    auto BinaryExpressionNode::Stringify (std::size_t pIndent) const -> std::string
    {
        return std::format(
            "{0}BinaryExpression:\n"
            "  Left:\n"
            "{1}"
            "  Right:\n"
            "{2}"
            "  Operator: {3}\n",
            std::string(pIndent * 2, ' '),
            mLeft ? mLeft->Stringify(pIndent + 2) : "",
            mRight ? mRight->Stringify(pIndent + 2) : "",
            mIsSubtraction ? '-' : '+'
        );
    }

    auto PointerExpressionNode::Stringify (std::size_t pIndent) const -> std::string
    {
        return std::format(
            "{0}PointerExpression:\n"
            "{1}",
            std::string(pIndent * 2, ' '),
            mExpression ? mExpression->Stringify(pIndent + 1) : ""
        );
    }

    // Operand Nodes ***********************************************************

    

    // Directive Nodes *********************************************************

    auto ByteDirectiveNode::Stringify (std::size_t pIndent) const -> std::string
    {
        std::string str = std::format(
            "{0}ByteDirective:\n",
            std::string(pIndent * 2, ' ')
        );

        for (const auto& operand : mOperands)
        {
            str += operand->Stringify(pIndent + 1);
        }

        return str;
    }

    auto WordDirectiveNode::Stringify (std::size_t pIndent) const -> std::string
    {
        std::string str = std::format(
            "{0}WordDirective:\n",
            std::string(pIndent * 2, ' ')
        );

        for (const auto& operand : mOperands)
        {
            str += operand->Stringify(pIndent + 1);
        }

        return str;
    }

    auto DoubleWordDirectiveNode::Stringify (std::size_t pIndent) const -> std::string
    {
        std::string str = std::format(
            "{0}DoubleWordDirective:\n",
            std::string(pIndent * 2, ' ')
        );

        for (const auto& operand : mOperands)
        {
            str += operand->Stringify(pIndent + 1);
        }

        return str;
    }

    auto StringDirectiveNode::Stringify (std::size_t pIndent) const -> std::string
    {
        std::string str = std::format(
            "{0}StringDirective:\n",
            std::string(pIndent * 2, ' ')
        );

        for (const auto& operand : mOperands)
        {
            str += operand->Stringify(pIndent + 1);
        }

        return str;
    }

    auto SpaceDirectiveNode::Stringify (std::size_t pIndent) const -> std::string
    {
        std::string str = std::format(
            "{0}SpaceDirective:\n",
            std::string(pIndent * 2, ' ')
        );

        for (const auto& operand : mOperands)
        {
            str += operand->Stringify(pIndent + 1);
        }

        return str;
    }

    auto IncbinDirectiveNode::Stringify (std::size_t pIndent) const -> std::string
    {
        return std::format(
            "{0}IncbinDirective:\n"
            "{0}  Filename:\n"
            "{1}"
            "{0}  Offset:\n"
            "{2}"
            "{0}  Size:\n"
            "{3}",
            std::string(pIndent * 2, ' '),
            mFilename ? mFilename->Stringify(pIndent + 2) : "",
            mOffset ? mOffset->Stringify(pIndent + 2) : "",
            mSize ? mSize->Stringify(pIndent + 2) : ""
        );
    }

    auto ExportDirectiveNode::Stringify (std::size_t pIndent) const -> std::string
    {
        std::string str = std::format(
            "{0}ExportDirective:\n",
            std::string(pIndent * 2, ' ')
        );

        for (const auto& label : mLabels)
        {
            str += label->Stringify(pIndent + 1);
        }

        return str;
    }

    auto ImportDirectiveNode::Stringify (std::size_t pIndent) const -> std::string
    {
        std::string str = std::format(
            "{0}ImportDirective:\n",
            std::string(pIndent * 2, ' ')
        );

        for (const auto& label : mLabels)
        {
            str += label->Stringify(pIndent + 1);
        }

        return str;
    }

    auto SectionDirectiveNode::Stringify (std::size_t pIndent) const -> std::string
    {
        return std::format(
            "{0}SectionDirective:\n"
            "{0}  DisplayName:\n"
            "{1}"
            "{0}  SectionName:\n"
            "{2}"
            "{0}  FixedAddress:\n"
            "{3}",
            std::string(pIndent * 2, ' '),
            mDisplayName ? mDisplayName->Stringify(pIndent + 2) : "",
            mSectionName ? mSectionName->Stringify(pIndent + 2) : "",
            mFixedAddress ? mFixedAddress->Stringify(pIndent + 2) : ""
        );
    }

    auto OrgDirectiveNode::Stringify (std::size_t pIndent) const -> std::string
    {
        return std::format(
            "{0}OrgDirective:\n"
            "{1}",
            std::string(pIndent * 2, ' '),
            mAddress ? mAddress->Stringify(pIndent + 1) : ""
        );
    }

    auto AlignDirectiveNode::Stringify (std::size_t pIndent) const -> std::string
    {
        return std::format(
            "{0}AlignDirective:\n"
            "{1}",
            std::string(pIndent * 2, ' '),
            mBoundary ? mBoundary->Stringify(pIndent + 1) : ""
        );
    }

    // Statement Nodes *********************************************************

    auto LabelStatementNode::Stringify (std::size_t pIndent) const -> std::string
    {
        return std::format(
            "{0}LabelStatement:\n"
            "{1}",
            std::string(pIndent * 2, ' '),
            mLabelExpr.Stringify(pIndent + 1)
        );
    }

    auto InstructionStatementNode::Stringify (std::size_t pIndent) const -> std::string
    {
        std::string name {};
        switch (mType)
        {
            case CPU::InstructionType::NOP: name = "NOP"; break;
            case CPU::InstructionType::STOP: name = "STOP"; break;
            case CPU::InstructionType::HALT: name = "HALT"; break;
            case CPU::InstructionType::DI: name = "DI"; break;
            case CPU::InstructionType::EI: name = "EI"; break;
            case CPU::InstructionType::EII: name = "EII"; break;
            case CPU::InstructionType::DAA: name = "DAA"; break;
            case CPU::InstructionType::SCF: name = "SCF"; break;
            case CPU::InstructionType::CCF: name = "CCF"; break;
            case CPU::InstructionType::CLV: name = "CLV"; break;
            case CPU::InstructionType::SEV: name = "SEV"; break;
            case CPU::InstructionType::REX: name = "REX"; break;
            case CPU::InstructionType::LEC: name = "LEC"; break;
            case CPU::InstructionType::LD: name = "LD"; break;
            case CPU::InstructionType::LDQ: name = "LDQ"; break;
            case CPU::InstructionType::LDP: name = "LDP"; break;
            case CPU::InstructionType::ST: name = "ST"; break;
            case CPU::InstructionType::STQ: name = "STQ"; break;
            case CPU::InstructionType::STP: name = "STP"; break;
            case CPU::InstructionType::MV: name = "MV"; break;
            case CPU::InstructionType::MWH: name = "MWH"; break;
            case CPU::InstructionType::MWL: name = "MWL"; break;
            case CPU::InstructionType::LSP: name = "LSP"; break;
            case CPU::InstructionType::POP: name = "POP"; break;
            case CPU::InstructionType::SSP: name = "SSP"; break;
            case CPU::InstructionType::PUSH: name = "PUSH"; break;
            case CPU::InstructionType::SPO: name = "SPO"; break;
            case CPU::InstructionType::SPI: name = "SPI"; break;
            case CPU::InstructionType::JMP: name = "JMP"; break;
            case CPU::InstructionType::JPB: name = "JPB"; break;
            case CPU::InstructionType::CALL: name = "CALL"; break;
            case CPU::InstructionType::INT: name = "INT"; break;
            case CPU::InstructionType::RET: name = "RET"; break;
            case CPU::InstructionType::RETI: name = "RETI"; break;
            case CPU::InstructionType::MFI: name = "MFI"; break;
            case CPU::InstructionType::MFO: name = "MFO"; break;
            case CPU::InstructionType::ADD: name = "ADD"; break;
            case CPU::InstructionType::ADC: name = "ADC"; break;
            case CPU::InstructionType::SUB: name = "SUB"; break;
            case CPU::InstructionType::SBC: name = "SBC"; break;
            case CPU::InstructionType::INC: name = "INC"; break;
            case CPU::InstructionType::DEC: name = "DEC"; break;
            case CPU::InstructionType::AND: name = "AND"; break;
            case CPU::InstructionType::OR: name = "OR"; break;
            case CPU::InstructionType::XOR: name = "XOR"; break;
            case CPU::InstructionType::NOT: name = "NOT"; break;
            case CPU::InstructionType::CMP: name = "CMP"; break;
            case CPU::InstructionType::SLA: name = "SLA"; break;
            case CPU::InstructionType::SRA: name = "SRA"; break;
            case CPU::InstructionType::SRL: name = "SRL"; break;
            case CPU::InstructionType::SWAP: name = "SWAP"; break;
            case CPU::InstructionType::RLA: name = "RLA"; break;
            case CPU::InstructionType::RL: name = "RL"; break;
            case CPU::InstructionType::RLCA: name = "RLCA"; break;
            case CPU::InstructionType::RLC: name = "RLC"; break;
            case CPU::InstructionType::RRA: name = "RRA"; break;
            case CPU::InstructionType::RR: name = "RR"; break;
            case CPU::InstructionType::RRCA: name = "RRCA"; break;
            case CPU::InstructionType::RRC: name = "RRC"; break;
            case CPU::InstructionType::BIT: name = "BIT"; break;
            case CPU::InstructionType::SET: name = "SET"; break;
            case CPU::InstructionType::RES: name = "RES"; break;
            case CPU::InstructionType::TOG: name = "TOG"; break;
            case CPU::InstructionType::LDI: name = "LDI"; break;
            case CPU::InstructionType::LDD: name = "LDD"; break;
            case CPU::InstructionType::STI: name = "STI"; break;
            case CPU::InstructionType::STD: name = "STD"; break;
            case CPU::InstructionType::ASP: name = "ASP"; break;
            case CPU::InstructionType::LASP: name = "LASP"; break;
            case CPU::InstructionType::ISP: name = "ISP"; break;
            case CPU::InstructionType::DSP: name = "DSP"; break;
            case CPU::InstructionType::ASR: name = "ASR"; break;
        }

        return std::format(
            "{0}InstructionStatement:\n"
            "{0}  Type: {1}\n"
            "{0}  Operands:\n"
            "{2}",
            std::string(pIndent * 2, ' '),
            name,
            [this, pIndent] () {
                std::string str;
                for (const auto& operand : mOperands) {
                    str += operand->Stringify(pIndent + 2);
                }
                return str;
            }()
        );
    }

    // Syntax Module ***********************************************************

    auto SyntaxModule::Stringify (std::size_t pIndent) const -> std::string
    {
        std::string result = std::format(
            "{0}SyntaxModule:\n",
            std::string(pIndent * 2, ' ')
        );

        for (const auto& node : mNodes)
        {
            result += node->Stringify(pIndent + 1);
        }

        return result;
    }
}
