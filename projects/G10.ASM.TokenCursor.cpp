/**
 * @file    G10.ASM.TokenCursor.cpp
 * @brief   Contains implementations for the G10 Assembler's token cursor class,
 *          and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.TokenCursor.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::ASM
{
    TokenCursor::TokenCursor (TokenSlice pSlice) :
        mSlice  { pSlice }
    {}
}

// Public Methods **************************************************************

namespace G10::ASM
{
    auto TokenCursor::GetIndex () const -> std::size_t
        { return mIndex; }

    auto TokenCursor::SetIndex (const std::size_t pIndex) -> void
    {
        if (mSlice.empty() == true) { mIndex = 0; }
        else if (pIndex >= mSlice.size()) { mIndex = mSlice.size() - 1; }
        else { mIndex = pIndex; }
    }

    auto TokenCursor::ResetIndex () -> void
        { mIndex = 0; }

    auto TokenCursor::IsAtEnd () const -> bool
        { return (mIndex >= mSlice.size()); }

    auto TokenCursor::GetNextToken (bool pAdvanceAfter) -> const Token&
    {
        if (mIndex >= mSlice.size()) { return mSlice.back(); }
        else if (pAdvanceAfter == true) { return mSlice[mIndex++]; }
        else { return mSlice[mIndex]; }
    }

    auto TokenCursor::ExpectNextToken (TokenType pType, bool pAdvanceAfter)
        -> stx::optional_ref<const Token>
    {
        if (mIndex >= mSlice.size() || mSlice[mIndex].mType != pType)
            { return std::nullopt; }
        else if (pAdvanceAfter == true)
            { return mSlice[mIndex++]; }
        else
            { return mSlice[mIndex]; }
    }

    auto TokenCursor::ExpectNextToken (TokenGroup pGroup, bool pAdvanceAfter)
        -> stx::optional_ref<const Token>
    {
        if (mIndex >= mSlice.size() || mSlice[mIndex].GetGroup() != pGroup)
            { return std::nullopt; }
        else if (pAdvanceAfter == true)
            { return mSlice[mIndex++]; }
        else
            { return mSlice[mIndex]; }
    }

    auto TokenCursor::ExpectNextToken (KeywordGroup pGroup, bool pAdvanceAfter)
        -> stx::optional_ref<const Token>
    {
        if (mIndex >= mSlice.size())
            { return std::nullopt; }
        if (const auto kw = mSlice[mIndex].GetKeyword();
            kw && kw->GetGroup() == pGroup)
        {
            if (pAdvanceAfter == true) { return mSlice[mIndex++]; }
            else { return mSlice[mIndex]; }
        }

        return std::nullopt;
    }

    auto TokenCursor::PeekToken (std::size_t pOffset) const -> const Token&
    {
        if (mIndex + pOffset >= mSlice.size()) { return mSlice.back(); }
        else { return mSlice[mIndex + pOffset]; }
    }

    auto TokenCursor::Skip (std::size_t pCount) -> void
    {
        while (pCount > 0 && mIndex < mSlice.size())
            { ++mIndex; --pCount; }
    }

    auto TokenCursor::Unskip (std::size_t pCount) -> void
    {
        while (pCount > 0 && mIndex > 0)
            { --mIndex; --pCount; }
    }

    auto TokenCursor::SkipNewlines () -> void
    {
        while (
            mIndex < mSlice.size() && 
            mSlice[mIndex].mType == TokenType::NewLine
        ) { ++mIndex; }
    }

    auto TokenCursor::CollectLine () -> TokenSlice
    {
        if (mIndex >= mSlice.size())
            { return {}; }

        const std::size_t start = mIndex;
        while (
            mIndex < mSlice.size() &&
            mSlice[mIndex].mType != TokenType::EndOfFile &&
            mSlice[mIndex].mType != TokenType::NewLine
        ) { ++mIndex; }

        return mSlice.subspan(start, mIndex - start);
    }

    auto TokenCursor::CollectUntil (TokenType pType) -> TokenSlice
    {
        if (mIndex >= mSlice.size())
            { return {}; }

        const std::size_t start = mIndex;
        while (
            mIndex < mSlice.size() &&
            mSlice[mIndex].mType != pType
        ) { ++mIndex; }

        return mSlice.subspan(start, mIndex - start);
    }

    auto TokenCursor::CollectBlock (TokenType pStartType, TokenType pEndType) -> TokenSlice
    {
        if (mIndex >= mSlice.size())
            { return {}; }

        const std::size_t start = mIndex;
        std::size_t depth = 1;

        while (mIndex < mSlice.size() && depth > 0)
        {
            if (mSlice[mIndex].mType == pStartType)
                { ++depth; }
            else if (mSlice[mIndex].mType == pEndType)
                { if (--depth == 0) { break; } }

            ++mIndex;
        }

        return mSlice.subspan(start, mIndex - start);
    }

    auto TokenCursor::CollectExpression () -> TokenSlice
    {
        if (mIndex >= mSlice.size())
            { return {}; }

        const std::size_t start = mIndex;
        while (
            mIndex < mSlice.size() &&
            mSlice[mIndex].CanContinueExpression() == true
        ) { ++mIndex; }

        return mSlice.subspan(start, mIndex - start);
    }

    auto TokenCursor::CollectConditional () -> std::optional<TokenSlice>
    {
        if (mIndex >= mSlice.size())
            { return {}; }

        const std::size_t start = mIndex;
        std::size_t depth = 1;

        while (mIndex < mSlice.size() && depth > 0)
        {
            const auto& token = mSlice[mIndex];
            if (const auto kw = token.GetKeyword();
                kw && kw->Is<PreprocessorDirective>())
            {
                auto type = kw->GetType<PreprocessorDirective>();
                if (type == PreprocessorDirective::IF)
                    { depth++; }
                else if (type == PreprocessorDirective::ENDIF)
                {
                    if (--depth == 0)
                        { break; }
                }
                else if (type == PreprocessorDirective::ELSEIF ||
                         type == PreprocessorDirective::ELSE)
                {
                    if (depth == 1)
                        { return mSlice.subspan(start, mIndex - start); }
                }
            }

            ++mIndex;
        }

        if (depth > 0)
        {
            mIndex = start;
            return std::nullopt;
        }

        return mSlice.subspan(start, mIndex - start);
    }

    auto TokenCursor::CollectBody (PreprocessorDirective pType) 
        -> std::optional<TokenSlice>
    {
        PreprocessorDirective pEndType;
        switch (pType)
        {
            case PreprocessorDirective::REPEAT:
                pEndType = PreprocessorDirective::ENDREPEAT;
                break;
            case PreprocessorDirective::WHILE:
                pEndType = PreprocessorDirective::ENDWHILE;
                break;
            case PreprocessorDirective::FOR:
                pEndType = PreprocessorDirective::ENDFOR;
                break;
            case PreprocessorDirective::MACRO:
                pEndType = PreprocessorDirective::ENDMACRO;
                break;
            default: return std::nullopt;
        }

        if (mIndex >= mSlice.size())
            { return {}; }

        const std::size_t start = mIndex;
        std::size_t depth = 1;

        while (mIndex < mSlice.size() && depth > 0)
        {
            const auto& token = mSlice[mIndex];
            if (const auto kw = token.GetKeyword();
                kw && kw->Is<PreprocessorDirective>())
            {
                auto type = kw->GetType<PreprocessorDirective>();
                if (type == pType)
                    { depth++; }
                else if (type == pEndType)
                {
                    if (--depth == 0)
                        { break; }
                }
            }

            ++mIndex;
        }

        if (depth > 0)
        {
            mIndex = start;
            return std::nullopt;
        }

        return mSlice.subspan(start, mIndex - start);
    }
}
