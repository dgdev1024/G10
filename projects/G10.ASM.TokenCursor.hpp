/**
 * @file    G10.ASM.TokenCursor.hpp
 * @brief   Contains declarations for the G10 Assembler's token cursor class,
 *          and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.ASM.Token.hpp>

// Types ***********************************************************************

namespace G10::ASM
{
    using TokenSlice = std::span<Token>;
}

// Classes *********************************************************************

namespace G10::ASM
{
    class G10_API TokenCursor final
    {
    public: // Constructors and Destructor *************************************

        TokenCursor () = default;
        explicit TokenCursor (TokenSlice pSlice);

    public: // Methods *********************************************************

        auto GetIndex () const -> std::size_t;
        auto SetIndex (const std::size_t pIndex) -> void;
        auto ResetIndex () -> void;
        auto IsAtEnd () const -> bool;
        auto GetNextToken (bool pAdvanceAfter = false) -> const Token&;
        auto ExpectNextToken (TokenType pType, bool pAdvanceAfter = true)
            -> stx::optional_ref<const Token>;
        auto ExpectNextToken (TokenGroup pGroup, bool pAdvanceAfter = true)
            -> stx::optional_ref<const Token>;
        auto ExpectNextToken (KeywordGroup pGroup, bool pAdvanceAfter = true)
            -> stx::optional_ref<const Token>;
        auto PeekToken (std::size_t pOffset) const -> const Token&;
        auto Skip (std::size_t pCount = 1) -> void;
        auto Unskip (std::size_t pCount = 1) -> void;
        auto SkipNewlines () -> void;
        auto CollectLine () -> TokenSlice;
        auto CollectUntil (TokenType pType) -> TokenSlice;
        auto CollectBlock (TokenType pStartType, TokenType pEndType) -> TokenSlice;
        auto CollectExpression () -> TokenSlice;
        auto CollectConditional () -> std::optional<TokenSlice>;
        auto CollectBody (PreprocessorDirective pType) -> std::optional<TokenSlice>;

    private: // Members ********************************************************

        TokenSlice mSlice {};
        std::size_t mIndex { 0 };

    };
}
