/**
 * @file    G10.ASM.Token.hpp
 * @brief   Contains declarations for the G10 Assembler's token structure,
 *          and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.ASM.Keyword.hpp>

// Types ***********************************************************************

namespace G10::ASM
{
    using TokenContents = std::variant<
        std::monostate,                 // Operators and Punctuation
        std::string,                    // Literals and Identifiers
        Keyword::Ref                    // Keywords
    >;
}

// Constants & Enumerations ****************************************************

namespace G10::ASM
{
    enum class TokenGroup : std::uint32_t
    {
        Unknown                 = 0x00,
        ControlToken            = 0x100,
        ArithmeticOperator      = 0x200,
        AssignmentOperator      = 0x400,
        LogicalOperator         = 0x800,
        ComparisonOperator      = 0x1000,
        GroupingOperator        = 0x2000,
        Punctuation             = 0x4000,
        Literal                 = 0x8000,
        Keyword                 = 0x10000,
    };

    enum class TokenType : std::uint32_t
    {
        Unknown                 = 0x00,
        EndOfFile               = 0x100,
        NewLine,
        Plus                    = 0x201,    // `+`
        Minus,                              // `-`
        Times,                              // `*`
        Exponent,                           // `**`
        Divide,                             // `/`
        Modulo,                             // `%`
        BitwiseShiftLeft,                   // `<<`
        BitwiseShiftRight,                  // `>>`
        BitwiseAnd,                         // `&`
        BitwiseOr,                          // `|`
        BitwiseXor,                         // `^`
        BitwiseNot,                         // `~`
        AssignEqual             = 0x400,    // `=`
        AssignPlus,                         // `+=`
        AssignMinus,                        // `-=`
        AssignTimes,                        // `*=`
        AssignExponent,                     // `**=`
        AssignDivide,                       // `/=`
        AssignModulo,                       // `%=`
        AssignShiftLeft,                    // `<<=`
        AssignShiftRight,                   // `>>=`
        AssignAnd,                          // `&=`
        AssignOr,                           // `|=`
        AssignXor,                          // `^=`
        LogicalAnd              = 0x800,    // `&&`
        LogicalOr,                          // `||`
        LogicalNot,                         // `!`
        CompareEqual            = 0x1000,   // `==`
        CompareNotEqual,                    // `!=`
        CompareGreater,                     // `>`
        CompareGreaterEqual,                // `>=`
        CompareLess,                        // `<`
        CompareLessEqual,                   // `<=`
        LeftParenthesis         = 0x2000,   // `(`
        RightParenthesis,                   // `)`
        LeftBracket,                        // `[`
        RightBracket,                       // `]`
        Comma                   = 0x4000,   // `,`
        Colon,                              // `:`
        BinaryLiteral           = 0x8000,   // e.g. `%10101101`, `0b11001100`
        OctalLiteral,                       // e.g. `&377`, `0o123`
        DecimalLiteral,                     // e.g. `123`
        HexadecimalLiteral,                 // e.g. `$1A3F`, `0x4B`
        CharacterLiteral,                   // e.g. `'a'`, `'\n'`
        PixelLiteral,                       // e.g. ` ``01230123 `
        FloatingPointLiteral,               // e.g. `123.456`, `0.789`
        Identifier              = 0x8010,   // e.g. `myVariable`, `.my_label`, `some_{interpolated}_symbol`
        Parameter,                          // e.g. `@1`, `@myMacroParam`, `@?`, `@@`
        StringLiteral,                      // e.g. `"Hello, World!"`, "The answer is {interpolated}."
        Keyword                 = 0x10000   
    };
}

// Structures ******************************************************************

namespace G10::ASM
{
    struct G10_API Token final
    {
        SourceLocation                  mLocation {};
        TokenType                       mType {};
        TokenContents                   mContents {};
        std::optional<std::uint64_t>    mInteger {};
        std::optional<std::int64_t>     mSignedInteger {};
        std::optional<double>           mFloat {};

    public: // Constructors & Destructor ***************************************

        Token () = default;
        explicit Token (TokenType pType, SourceLocation pLocation = {});
        explicit Token (TokenType pType, std::string pLiteral, SourceLocation pLocation = {});
        explicit Token (TokenType pType, std::string_view pLiteral, SourceLocation pLocation = {});
        explicit Token (std::uint64_t pCharCode, SourceLocation pLocation = {});
        explicit Token (TokenType pType, std::uint64_t pInteger, SourceLocation pLocation = {});
        explicit Token (const Keyword& pKeyword, SourceLocation pLocation = {});

    public: // Methods *********************************************************

        inline constexpr auto GetGroup () const -> TokenGroup
            { return static_cast<TokenGroup>(std::to_underlying(mType) & 0xFFFFFF00); }

        inline constexpr auto GetLiteral () const -> const std::string*
            { return std::get_if<std::string>(&mContents); }

        inline constexpr auto GetInteger () const -> std::optional<std::uint64_t>
            { return mInteger; }

        inline constexpr auto GetFloat () const -> std::optional<double>
            { return mFloat; }

        inline constexpr auto GetKeyword () const -> const Keyword*
        {
            if (auto ref = std::get_if<Keyword::Ref>(&mContents))
                { return &ref->get(); }
            else { return nullptr; }
        }

        inline constexpr auto IsInteger () const -> bool
        {
            return
                mType == TokenType::BinaryLiteral ||
                mType == TokenType::OctalLiteral ||
                mType == TokenType::DecimalLiteral ||
                mType == TokenType::HexadecimalLiteral ||
                mType == TokenType::CharacterLiteral ||
                mType == TokenType::PixelLiteral ||
                mType == TokenType::FloatingPointLiteral;
        }

        inline constexpr auto HasInteger () const -> bool
        {
            return
                mInteger.has_value() == true &&
                mSignedInteger.has_value() == true &&
                mFloat.has_value() == true;
        }

        inline constexpr auto CanStartExpression () const -> bool
        {
            if (const auto kw = GetKeyword())
            {
                return
                    kw->GetGroup() == KeywordGroup::PreprocessorFunction ||
                    kw->GetGroup() == KeywordGroup::PreprocessorBuiltinSymbol;
            }

            return
                GetGroup() == TokenGroup::Literal ||
                mType == TokenType::Parameter ||
                mType == TokenType::Plus ||
                mType == TokenType::Minus ||
                mType == TokenType::BitwiseNot ||
                mType == TokenType::LogicalNot ||
                mType == TokenType::LeftParenthesis;
        }

        inline constexpr auto CanContinueExpression () const -> bool
        {
            return
                CanStartExpression() == true ||
                GetGroup() == TokenGroup::ArithmeticOperator ||
                GetGroup() == TokenGroup::LogicalOperator ||
                GetGroup() == TokenGroup::ComparisonOperator ||
                mType == TokenType::RightParenthesis;
        }

        auto IsValidSecondPass () const -> bool;
        auto StringifyType () const -> std::string_view;
        auto StringifyGroup () const -> std::string_view;
        auto Stringify () const -> std::optional<std::string>;

    private: // Methods ********************************************************

        auto ParseNumeric () -> void;
        auto ParsePixel () -> void;

    };
}
