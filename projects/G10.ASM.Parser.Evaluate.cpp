/**
 * @file    G10.ASM.Parser.Evaluate.cpp
 * @brief   Contains implementations for the G10 Assembler Parser's evaluation
 *          methods, and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Parser.hpp>

// Private Methods - Evaluate **************************************************

namespace G10::ASM
{
    auto Parser::EvaluateIntegerExpression (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> std::shared_ptr<IntegerExpressionNode>
    {
        const auto resetIndex = pCursor.GetIndex();
        if (const auto& token = pCursor.GetNextToken(true);
            token.IsInteger() == true)
        {
            auto node = std::make_shared<IntegerExpressionNode>();
            node->mLocation = pLocation;
            node->mValue = static_cast<std::uint32_t>(*token.mInteger);
            return node;
        }

        pCursor.SetIndex(resetIndex);
        return nullptr;
    }

    auto Parser::EvaluateStringExpression (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> std::shared_ptr<StringExpressionNode>
    {
        const auto resetIndex = pCursor.GetIndex();
        if (const auto str = pCursor.ExpectNextToken(TokenType::StringLiteral))
        {
            auto node = std::make_shared<StringExpressionNode>();
            node->mLocation = pLocation;
            node->mValue = *str->Stringify();
            return node;
        }

        pCursor.SetIndex(resetIndex);
        return nullptr;
    }

    auto Parser::EvaluateRegisterExpression (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> std::shared_ptr<RegisterExpressionNode>
    {
        const auto resetIndex = pCursor.GetIndex();
        const auto& token = pCursor.GetNextToken(true);
        if (const auto kw = token.GetKeyword();
            kw && kw->Is<CPU::Register>())
        {
            auto node = std::make_shared<RegisterExpressionNode>();
            node->mLocation = pLocation;
            node->mRegister = kw->GetType<CPU::Register>();
            switch (node->mRegister)
            {
                case CPU::Register::D0:     node->mName = "D0"; break;
                case CPU::Register::D1:     node->mName = "D1"; break;
                case CPU::Register::D2:     node->mName = "D2"; break;
                case CPU::Register::D3:     node->mName = "D3"; break;
                case CPU::Register::D4:     node->mName = "D4"; break;
                case CPU::Register::D5:     node->mName = "D5"; break;
                case CPU::Register::D6:     node->mName = "D6"; break;
                case CPU::Register::D7:     node->mName = "D7"; break;
                case CPU::Register::D8:     node->mName = "D8"; break;
                case CPU::Register::D9:     node->mName = "D9"; break;
                case CPU::Register::D10:    node->mName = "D10"; break;
                case CPU::Register::D11:    node->mName = "D11"; break;
                case CPU::Register::D12:    node->mName = "D12"; break;
                case CPU::Register::D13:    node->mName = "D13"; break;
                case CPU::Register::D14:    node->mName = "D14"; break;
                case CPU::Register::D15:    node->mName = "D15"; break;
                case CPU::Register::W0:     node->mName = "W0"; break;
                case CPU::Register::W1:     node->mName = "W1"; break;
                case CPU::Register::W2:     node->mName = "W2"; break;
                case CPU::Register::W3:     node->mName = "W3"; break;
                case CPU::Register::W4:     node->mName = "W4"; break;
                case CPU::Register::W5:     node->mName = "W5"; break;
                case CPU::Register::W6:     node->mName = "W6"; break;
                case CPU::Register::W7:     node->mName = "W7"; break;
                case CPU::Register::W8:     node->mName = "W8"; break;
                case CPU::Register::W9:     node->mName = "W9"; break;
                case CPU::Register::W10:    node->mName = "W10"; break;
                case CPU::Register::W11:    node->mName = "W11"; break;
                case CPU::Register::W12:    node->mName = "W12"; break;
                case CPU::Register::W13:    node->mName = "W13"; break;
                case CPU::Register::W14:    node->mName = "W14"; break;
                case CPU::Register::W15:    node->mName = "W15"; break;
                case CPU::Register::H0:     node->mName = "H0"; break;
                case CPU::Register::H1:     node->mName = "H1"; break;
                case CPU::Register::H2:     node->mName = "H2"; break;
                case CPU::Register::H3:     node->mName = "H3"; break;
                case CPU::Register::H4:     node->mName = "H4"; break;
                case CPU::Register::H5:     node->mName = "H5"; break;
                case CPU::Register::H6:     node->mName = "H6"; break;
                case CPU::Register::H7:     node->mName = "H7"; break;
                case CPU::Register::H8:     node->mName = "H8"; break;
                case CPU::Register::H9:     node->mName = "H9"; break;
                case CPU::Register::H10:    node->mName = "H10"; break;
                case CPU::Register::H11:    node->mName = "H11"; break;
                case CPU::Register::H12:    node->mName = "H12"; break;
                case CPU::Register::H13:    node->mName = "H13"; break;
                case CPU::Register::H14:    node->mName = "H14"; break;
                case CPU::Register::H15:    node->mName = "H15"; break;
                case CPU::Register::L0:     node->mName = "L0"; break;
                case CPU::Register::L1:     node->mName = "L1"; break;
                case CPU::Register::L2:     node->mName = "L2"; break;
                case CPU::Register::L3:     node->mName = "L3"; break;
                case CPU::Register::L4:     node->mName = "L4"; break;
                case CPU::Register::L5:     node->mName = "L5"; break;
                case CPU::Register::L6:     node->mName = "L6"; break;
                case CPU::Register::L7:     node->mName = "L7"; break;
                case CPU::Register::L8:     node->mName = "L8"; break;
                case CPU::Register::L9:     node->mName = "L9"; break;
                case CPU::Register::L10:    node->mName = "L10"; break;
                case CPU::Register::L11:    node->mName = "L11"; break;
                case CPU::Register::L12:    node->mName = "L12"; break;
                case CPU::Register::L13:    node->mName = "L13"; break;
                case CPU::Register::L14:    node->mName = "L14"; break;
                case CPU::Register::L15:    node->mName = "L15"; break;
            }

            return node;
        }

        pCursor.SetIndex(resetIndex);
        return nullptr;
    }

    auto Parser::EvaluateConditionExpression (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> std::shared_ptr<ConditionExpressionNode>
    {
        const auto resetIndex = pCursor.GetIndex();
        const auto& token = pCursor.GetNextToken(true);
        if (const auto kw = token.GetKeyword();
            kw && kw->Is<CPU::Condition>())
        {
            auto node = std::make_shared<ConditionExpressionNode>();
            node->mLocation = pLocation;
            node->mCondition = kw->GetType<CPU::Condition>();
            switch (node->mCondition)
            {
                case CPU::Condition::NC: node->mName = "NC"; break;
                case CPU::Condition::ZS: node->mName = "ZS"; break;
                case CPU::Condition::ZC: node->mName = "ZC"; break;
                case CPU::Condition::CS: node->mName = "CS"; break;
                case CPU::Condition::CC: node->mName = "CC"; break;
                case CPU::Condition::VS: node->mName = "VS"; break;
                case CPU::Condition::VC: node->mName = "VC"; break;
            }

            return node;
        }

        pCursor.SetIndex(resetIndex);
        return nullptr;
    }

    auto Parser::EvaluateLabelExpression (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> std::shared_ptr<LabelExpressionNode>
    {
        const auto resetIndex = pCursor.GetIndex();
        if (const auto id = pCursor.ExpectNextToken(TokenType::Identifier))
        {
            auto node = std::make_shared<LabelExpressionNode>();
            node->mLocation = pLocation;
            node->mSymbol = *id->Stringify();
            return node;
        }

        pCursor.SetIndex(resetIndex);
        return nullptr;
    }

    auto Parser::EvaluateSectionNameExpression (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> std::shared_ptr<SectionNameExpressionNode>
    {
        const auto resetIndex = pCursor.GetIndex();
        const auto& token = pCursor.GetNextToken(true);
        if (const auto kw = token.GetKeyword();
            kw && kw->Is<SectionName>())
        {
            auto node = std::make_shared<SectionNameExpressionNode>();
            node->mLocation = pLocation;
            node->mSectionName = kw->GetType<SectionName>();
            switch (node->mSectionName)
            {
                case SectionName::METADATA: node->mName = "METADATA"; break;
                case SectionName::INT0: node->mName = "INT0"; break;
                case SectionName::INT1: node->mName = "INT1"; break;
                case SectionName::INT2: node->mName = "INT2"; break;
                case SectionName::INT3: node->mName = "INT3"; break;
                case SectionName::INT4: node->mName = "INT4"; break;
                case SectionName::INT5: node->mName = "INT5"; break;
                case SectionName::INT6: node->mName = "INT6"; break;
                case SectionName::INT7: node->mName = "INT7"; break;
                case SectionName::INT8: node->mName = "INT8"; break;
                case SectionName::INT9: node->mName = "INT9"; break;
                case SectionName::INT10: node->mName = "INT10"; break;
                case SectionName::INT11: node->mName = "INT11"; break;
                case SectionName::INT12: node->mName = "INT12"; break;
                case SectionName::INT13: node->mName = "INT13"; break;
                case SectionName::INT14: node->mName = "INT14"; break;
                case SectionName::INT15: node->mName = "INT15"; break;
                case SectionName::INT16: node->mName = "INT16"; break;
                case SectionName::INT17: node->mName = "INT17"; break;
                case SectionName::INT18: node->mName = "INT18"; break;
                case SectionName::INT19: node->mName = "INT19"; break;
                case SectionName::INT20: node->mName = "INT20"; break;
                case SectionName::INT21: node->mName = "INT21"; break;
                case SectionName::INT22: node->mName = "INT22"; break;
                case SectionName::INT23: node->mName = "INT23"; break;
                case SectionName::INT24: node->mName = "INT24"; break;
                case SectionName::INT25: node->mName = "INT25"; break;
                case SectionName::INT26: node->mName = "INT26"; break;
                case SectionName::INT27: node->mName = "INT27"; break;
                case SectionName::INT28: node->mName = "INT28"; break;
                case SectionName::INT29: node->mName = "INT29"; break;
                case SectionName::INT30: node->mName = "INT30"; break;
                case SectionName::INT31: node->mName = "INT31"; break;
                case SectionName::CODE: node->mName = "CODE"; break;
                case SectionName::DATA: node->mName = "DATA"; break;
                case SectionName::BSS: node->mName = "BSS"; break;
            }
            
            return node;
        }

        pCursor.SetIndex(resetIndex);
        return nullptr;
    }

    auto Parser::EvaluateBinaryExpression (const SourceLocation& pLocation, 
        TokenCursor& pCursor) -> std::shared_ptr<ExpressionNode>
    {
        const auto resetIndex = pCursor.GetIndex();

        std::shared_ptr<ExpressionNode> left {};
        if (pCursor.ExpectNextToken(TokenType::LeftParenthesis))
        {
            left = EvaluateBinaryExpression(pLocation, pCursor);
            if (left == nullptr ||
                !pCursor.ExpectNextToken(TokenType::RightParenthesis))
            {
                pCursor.SetIndex(resetIndex);
                return nullptr;
            }
        }
        else
        {
            left = EvaluateIntegerExpression(pLocation, pCursor);
            if (left == nullptr)
            {
                pCursor.SetIndex(resetIndex);
                left = EvaluateLabelExpression(pLocation, pCursor);
                if (left == nullptr)
                {
                    pCursor.SetIndex(resetIndex);
                    return nullptr;
                }
            }
        }

        const auto& op = pCursor.GetNextToken();
        if (op.mType != TokenType::Plus && op.mType != TokenType::Minus)
        {
            left->mLocation = pLocation;
            return left;
        }
        pCursor.Skip();

        auto right = EvaluateBinaryExpression(pLocation, pCursor);
        if (right == nullptr)
        {
            pCursor.SetIndex(resetIndex);
            return nullptr;
        }

        auto binary = std::make_shared<BinaryExpressionNode>();
        binary->mLeft = left;
        binary->mRight = right;
        binary->mIsSubtraction = (op.mType == TokenType::Minus);
        binary->mLocation = pLocation;
        return binary;
    }

    auto Parser::EvaluatePointerExpression (const SourceLocation& pLocation, 
        TokenCursor& pCursor) -> std::shared_ptr<PointerExpressionNode>
    {
        const auto resetIndex = pCursor.GetIndex();
        if (!pCursor.ExpectNextToken(TokenType::LeftBracket))
        {
            pCursor.SetIndex(resetIndex);
            return nullptr;
        }

        std::shared_ptr<ExpressionNode> expr = 
            EvaluateRegisterExpression(pLocation, pCursor);
        if (expr == nullptr)
        {
            expr = EvaluateBinaryExpression(pLocation, pCursor);
            if (expr == nullptr)
            {
                pCursor.SetIndex(resetIndex);
                return nullptr;
            }
        }

        if (!pCursor.ExpectNextToken(TokenType::RightBracket))
        {
            pCursor.SetIndex(resetIndex);
            return nullptr;
        }

        auto pointer = std::make_shared<PointerExpressionNode>();
        pointer->mExpression = expr;
        pointer->mLocation = pLocation;
        return pointer;
    }

    auto Parser::EvaluateOperandExpression (const SourceLocation& pLocation, 
        TokenCursor& pCursor) -> std::shared_ptr<ExpressionNode>
    {
        const auto resetIndex = pCursor.GetIndex();
        std::shared_ptr<ExpressionNode> operand {};

        if (operand = EvaluateBinaryExpression(pLocation, pCursor))
            { return operand; }
        else if (operand = EvaluateStringExpression(pLocation, pCursor))
            { return operand; }
        else if (operand = EvaluateRegisterExpression(pLocation, pCursor))
            { return operand; }
        else if (operand = EvaluateConditionExpression(pLocation, pCursor))
            { return operand; }
        else if (operand = EvaluatePointerExpression(pLocation, pCursor))
            { return operand; }

        pCursor.SetIndex(resetIndex);
        return nullptr;
    }
}
