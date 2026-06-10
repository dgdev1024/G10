/**
 * @file    G10.ASM.Codegen.Evaluate.cpp
 * @brief   Contains implementations for the G10 Assembler Parser's
 *          evaluation methods, and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Codegen.hpp>

// Private Methods - Evaluate **************************************************

namespace G10::ASM
{
    auto Codegen::EvaluateIntegerExpression (const std::shared_ptr<ExpressionNode>& pNode)
        -> std::optional<std::uint32_t>
    {
        if (pNode == nullptr)
            { return std::nullopt; }

        if (const auto i = stx::to<IntegerExpressionNode>(pNode))
            { return i->mValue; }
        // else if (const auto b = stx::to<BinaryExpressionNode>(pNode))
        //     { return EvaluateBinaryExpression(b); }
        // else if (const auto l = stx::to<LabelExpressionNode>(pNode))
        // {
        //     if (auto maybe = EvaluateLabelExpression(l))
        //         { return maybe->second; }
        // }

        return std::nullopt;
    }

    auto Codegen::EvaluateStringExpression (const std::shared_ptr<ExpressionNode>& pNode)
        -> std::optional<std::string>
    {
        if (pNode == nullptr)
            { return std::nullopt; }

        if (const auto s = stx::to<StringExpressionNode>(pNode))
            { return s->mValue; }

        return std::nullopt;
    }

    auto Codegen::EvaluateRegisterExpression (const std::shared_ptr<ExpressionNode>& pNode)
        -> stx::optional_pair<std::string, CPU::Register>
    {
        if (pNode == nullptr)
            { return std::nullopt; }

        if (const auto r = stx::to<RegisterExpressionNode>(pNode))
            { return std::make_pair(r->mName, r->mRegister); }

        return std::nullopt;
    }

    auto Codegen::EvaluateConditionExpression (const std::shared_ptr<ExpressionNode>& pNode)
        -> stx::optional_pair<std::string, CPU::Condition>
    {
        if (pNode == nullptr)
            { return std::nullopt; }

        if (const auto c = stx::to<ConditionExpressionNode>(pNode))
            { return std::make_pair(c->mName, c->mCondition); }

        return std::nullopt;
    }

    auto Codegen::EvaluateLabelExpression (const std::shared_ptr<ExpressionNode>& pNode)
        -> stx::optional_pair<std::string, std::uint32_t>
    {
        if (pNode == nullptr)
            { return std::nullopt; }

        if (const auto l = stx::to<LabelExpressionNode>(pNode))
        {
            // Find the symbol's index. Ensure the index is in range.
            const auto& name = l->mSymbol;
            const auto findIt = mSymbolNameIndices.find(name);
            if (findIt == mSymbolNameIndices.end())
            {
                // Index Not Found = Unknown Symbol -> Create Relocation.
                return std::nullopt;
            }
            else if (findIt->second >= mSymbols.size())
            {
                // Index Out of Range -> Invalid Symbol.
                return std::nullopt;
            }

            // Ensure the symbol is defined, and belongs to a valid section.
            const auto& symbol = mSymbols[findIt->second];
            if (symbol.mSectionIndex == stx::npos32)
            {
                // Imported or Unresolved Symbol
                return std::nullopt;
            }
            else if (symbol.mSectionIndex < mSections.size())
            {
                // Valid Section -> Resolve It.
                const auto& section = mSections[symbol.mSectionIndex];
                if (section.mHeader.mTargetAddress != stx::npos32)
                {
                    // Fixed Target Address -> Resolve to Absolute.
                    return std::make_pair(name,
                        section.mHeader.mTargetAddress + symbol.mAddressOffset);
                }
                else // if (symbol.mSectionIndex == mActiveSectionIndex)
                {
                    // Symbol in Same Active Section -> Return Relative Address.
                    return std::make_pair(name, symbol.mAddressOffset);
                }

                // Symbol cannot be resolved to a numeric constant.
            }
        }

        return std::nullopt;
    }

    auto Codegen::EvaluateSectionNameExpression (const std::shared_ptr<ExpressionNode>& pNode)
        -> std::optional<SectionName>
    {
        if (pNode == nullptr)
            { return std::nullopt; }

        if (const auto s = stx::to<SectionNameExpressionNode>(pNode))
            { return s->mSectionName; }

        return std::nullopt;
    }

    auto Codegen::EvaluateBinaryExpression (const std::shared_ptr<ExpressionNode>& pNode)
        -> std::optional<std::uint32_t>
    {
        if (pNode == nullptr)
            { return std::nullopt; }

        if (const auto b = stx::to<BinaryExpressionNode>(pNode))
        {
            std::optional<std::uint32_t> left {}, right {};
            if (const auto l = EvaluateIntegerExpression(b->mLeft))
                { left = *l; }
            else if (const auto l = EvaluateBinaryExpression(b->mLeft))
                { left = *l; }
            else if (const auto l = EvaluateLabelExpression(b->mLeft))
                { left = l->second; }
            else
                { return std::nullopt; }

            if (const auto r = EvaluateIntegerExpression(b->mRight))
                { right = *r; }
            else if (const auto r = EvaluateBinaryExpression(b->mRight))
                { right = *r; } 
            else if (const auto r = EvaluateLabelExpression(b->mRight))
                { right = r->second; }
            else
                { return std::nullopt; }

            return (b->mIsSubtraction == true) ?
                static_cast<std::uint32_t>((*left) - (*right)) :
                static_cast<std::uint32_t>((*left) + (*right));
        }

        return std::nullopt;
    }

    auto Codegen::EvaluatePointerExpression (const std::shared_ptr<ExpressionNode>& pNode)
        -> stx::optional_var<std::uint32_t, 
            std::pair<std::string, CPU::Register>, std::string>
    {
        if (pNode == nullptr)
            { return std::nullopt; }

        if (const auto ptr = stx::to<PointerExpressionNode>(pNode))
        {
            if (const auto r = EvaluateRegisterExpression(ptr->mExpression))
                { return std::make_pair(r->first, r->second); }
            else if (const auto b = EvaluateBinaryExpression(ptr->mExpression))
                { return *b; }
            else if (const auto i = EvaluateIntegerExpression(ptr->mExpression))
                { return *i; }
            else if (const auto l = EvaluateLabelExpression(ptr->mExpression))
                { return l->first; }
            else if (const auto l = stx::to<LabelExpressionNode>(ptr->mExpression))
                { return l->mSymbol; }
        }

        return std::nullopt;
    }
}
