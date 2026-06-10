/**
 * @file    G10.ASM.Lexer.hpp
 * @brief   Contains declarations for the G10 Assembler's Lexer class,
 *          and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.ASM.TokenCursor.hpp>
#include <G10.ASM.Diagnostic.hpp>

// Classes *********************************************************************

namespace G10::ASM
{
    class G10_API Lexer final
    {
    public: // Constructors & Destructor ***************************************

        explicit Lexer (Diagnostic& pDiag);

    public: // Methods - Source ************************************************

        auto LexString (const std::string& pSource, bool pSecondPass) -> bool;
        auto LexFile (const fs::path& pPath, bool pSecondPass) -> bool;

    public: // Methods - Tokens ************************************************

        inline auto GetTokens () const -> const std::vector<Token>&
            { return mTokens; }
        inline auto GetTokens () -> std::vector<Token>&
            { return mTokens; }

    private: // Methods - Tokenization *****************************************

        auto Tokenize () -> bool;
        auto Peek (std::size_t pOffset = 1) -> char;
        auto Next () -> char;
        auto SkipWhitespace () -> void;
        auto SkipComment () -> bool;
    
    private: // Methods - Token Collection *************************************

        auto CollectIdentifier () -> std::optional<Token>;
        auto CollectParameter () -> std::optional<Token>;
        auto CollectString () -> std::optional<Token>;
        auto CollectChar () -> std::optional<Token>;
        auto CollectPixel () -> std::optional<Token>;
        auto CollectBinary (bool pPercentPrefix) -> std::optional<Token>;
        auto CollectOctal (bool pAmpersandPrefix) -> std::optional<Token>;
        auto CollectHexadecimal (bool pDollarPrefix) -> std::optional<Token>;
        auto CollectDecimal () -> std::optional<Token>;
        auto CollectSymbol () -> std::optional<Token>;

    private: // Members ********************************************************

        Diagnostic&         mDiag;
        std::vector<Token>  mTokens {};
        std::string         mSource { "" };
        std::size_t         mSourceIndex { 0 };
        char                mSourceChar { '\0' };
        SourceLocation      mLocation {};
        bool                mSecondPass { false };
        bool                mHint { false };
        bool                mHintFile { false };
        bool                mHintLine { false };

    };
}
