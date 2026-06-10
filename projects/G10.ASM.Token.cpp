/**
 * @file    G10.ASM.Token.cpp
 * @brief   Contains implementations for the G10 Assembler's token structure,
 *          and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Token.hpp>

// Static - Helper Functions ***************************************************

namespace G10::ASM
{
    static auto StringifySymbol (TokenType pType) -> std::optional<std::string_view>
    {
        switch (pType)
        {
            case TokenType::Plus: return "+";
            case TokenType::Minus: return "-";
            case TokenType::Times: return "*";
            case TokenType::Exponent: return "**";
            case TokenType::Divide: return "/";
            case TokenType::Modulo: return "%";
            case TokenType::BitwiseShiftLeft: return "<<";
            case TokenType::BitwiseShiftRight: return ">>";
            case TokenType::BitwiseAnd: return "&";
            case TokenType::BitwiseOr: return "|";
            case TokenType::BitwiseXor: return "^";
            case TokenType::BitwiseNot: return "~";
            case TokenType::AssignEqual: return "=";
            case TokenType::AssignPlus: return "+=";
            case TokenType::AssignMinus: return "-=";
            case TokenType::AssignTimes: return "*=";
            case TokenType::AssignExponent: return "**=";
            case TokenType::AssignDivide: return "/=";
            case TokenType::AssignModulo: return "%=";
            case TokenType::AssignShiftLeft: return "<<=";
            case TokenType::AssignShiftRight: return ">>=";
            case TokenType::AssignAnd: return "&=";
            case TokenType::AssignOr: return "|=";
            case TokenType::AssignXor: return "^=";
            case TokenType::LogicalAnd: return "&&";
            case TokenType::LogicalOr: return "||";
            case TokenType::LogicalNot: return "!";
            case TokenType::CompareEqual: return "==";
            case TokenType::CompareNotEqual: return "!=";
            case TokenType::CompareGreater: return ">";
            case TokenType::CompareGreaterEqual: return ">=";
            case TokenType::CompareLess: return "<";
            case TokenType::CompareLessEqual: return "<=";
            case TokenType::LeftParenthesis: return "(";
            case TokenType::RightParenthesis: return ")";
            case TokenType::LeftBracket: return "[";
            case TokenType::RightBracket: return "]";
            case TokenType::Comma: return ",";
            case TokenType::Colon: return ":";
            default: return std::nullopt;
        }
    }
}

// Public - Constructors & Destructor ******************************************

namespace G10::ASM
{
    Token::Token (TokenType pType, SourceLocation pLocation) :
        mLocation   { std::move(pLocation) },
        mType       { pType } {}

    Token::Token (TokenType pType, std::string pLiteral, SourceLocation pLocation) :
        mLocation   { std::move(pLocation) },
        mType       { pType },
        mContents   { std::move(pLiteral) } { ParseNumeric(); }

    Token::Token (TokenType pType, std::string_view pLiteral, SourceLocation pLocation) :
        Token { pType, std::string { pLiteral }, pLocation } {}

    Token::Token (std::uint64_t pCharCode, SourceLocation pLocation) :
        Token { TokenType::CharacterLiteral, std::string { std::to_string(pCharCode) }, pLocation } 
            { ParseNumeric(); }

    Token::Token (TokenType pType, std::uint64_t pInteger, SourceLocation pLocation) :
        Token { pType, std::to_string(pInteger), pLocation } 
            { ParseNumeric(); }

    Token::Token (const Keyword& pKeyword, SourceLocation pLocation) :
        mLocation   { std::move(pLocation) },
        mType       { TokenType::Keyword },
        mContents   { std::cref(pKeyword) } {}
}

// Public Methods **************************************************************

namespace G10::ASM
{
    auto Token::IsValidSecondPass () const -> bool
    {
        if (const auto kw = GetKeyword())
        {
            switch (kw->GetGroup())
            {
                case KeywordGroup::Register:
                case KeywordGroup::Condition:
                case KeywordGroup::InstructionType:
                case KeywordGroup::SectionName:
                case KeywordGroup::AssemblerDirective:
                case KeywordGroup::Hint:
                    return true;
                default:
                    return false;
            }
        }

        switch (mType)
        {
            case TokenType::EndOfFile:
            case TokenType::NewLine:
            case TokenType::Plus:
            case TokenType::Minus:
            case TokenType::LeftParenthesis:
            case TokenType::RightParenthesis:
            case TokenType::LeftBracket:
            case TokenType::RightBracket:
            case TokenType::Comma:
            case TokenType::Colon:
                return true;
            case TokenType::BinaryLiteral:
            case TokenType::OctalLiteral:
            case TokenType::DecimalLiteral:
            case TokenType::HexadecimalLiteral:
            case TokenType::CharacterLiteral:
            case TokenType::PixelLiteral:
                return
                    mInteger.has_value() == true &&
                    mInteger.value() <= 0xFFFFFFFF;
            case TokenType::StringLiteral: {
                if (const auto str = GetLiteral())
                    { return str->empty() == false; }
                else
                    { return false; }
            } break;
            case TokenType::Identifier: {
                if (const auto str = GetLiteral())
                { 
                    return 
                        str->empty() == false &&
                        str->contains('{') == false &&
                        str->contains('}') == false;
                }
                else
                    { return false; }
            } break;
            default:
                return false;
        }
    }

    auto Token::StringifyType () const -> std::string_view
    {
        switch (mType)
        {
            case TokenType::EndOfFile: return "EndOfFile";
            case TokenType::NewLine: return "NewLine";
            case TokenType::Plus: return "Plus";
            case TokenType::Minus: return "Minus";
            case TokenType::Times: return "Times";
            case TokenType::Exponent: return "Exponent";
            case TokenType::Divide: return "Divide";
            case TokenType::Modulo: return "Modulo";
            case TokenType::BitwiseShiftLeft: return "BitwiseShiftLeft";
            case TokenType::BitwiseShiftRight: return "BitwiseShiftRight";
            case TokenType::BitwiseAnd: return "BitwiseAnd";
            case TokenType::BitwiseOr: return "BitwiseOr";
            case TokenType::BitwiseXor: return "BitwiseXor";
            case TokenType::BitwiseNot: return "BitwiseNot";
            case TokenType::AssignEqual: return "AssignEqual";
            case TokenType::AssignPlus: return "AssignPlus";
            case TokenType::AssignMinus: return "AssignMinus";
            case TokenType::AssignTimes: return "AssignTimes";
            case TokenType::AssignExponent: return "AssignExponent";
            case TokenType::AssignDivide: return "AssignDivide";
            case TokenType::AssignModulo: return "AssignModulo";
            case TokenType::AssignShiftLeft: return "AssignShiftLeft";
            case TokenType::AssignShiftRight: return "AssignShiftRight";
            case TokenType::AssignAnd: return "AssignAnd";
            case TokenType::AssignOr: return "AssignOr";
            case TokenType::AssignXor: return "AssignXor";
            case TokenType::LogicalAnd: return "LogicalAnd";
            case TokenType::LogicalOr: return "LogicalOr";
            case TokenType::LogicalNot: return "LogicalNot";
            case TokenType::CompareEqual: return "CompareEqual";
            case TokenType::CompareNotEqual: return "CompareNotEqual";
            case TokenType::CompareGreater: return "CompareGreater";
            case TokenType::CompareGreaterEqual: return "CompareGreaterEqual";
            case TokenType::CompareLess: return "CompareLess";
            case TokenType::CompareLessEqual: return "CompareLessEqual";
            case TokenType::LeftParenthesis: return "LeftParenthesis";
            case TokenType::RightParenthesis: return "RightParenthesis";
            case TokenType::LeftBracket: return "LeftBracket";
            case TokenType::RightBracket: return "RightBracket";
            case TokenType::Comma: return "Comma";
            case TokenType::Colon: return "Colon";
            case TokenType::BinaryLiteral: return "BinaryLiteral";
            case TokenType::OctalLiteral: return "OctalLiteral";
            case TokenType::DecimalLiteral: return "DecimalLiteral";
            case TokenType::HexadecimalLiteral: return "HexadecimalLiteral";
            case TokenType::CharacterLiteral: return "CharacterLiteral";
            case TokenType::PixelLiteral: return "PixelLiteral";
            case TokenType::FloatingPointLiteral: return "FloatingPointLiteral";
            case TokenType::Identifier: return "Identifier";
            case TokenType::Parameter: return "Parameter";
            case TokenType::StringLiteral: return "StringLiteral";
            case TokenType::Keyword:
                if (const auto kw = GetKeyword())
                    { return kw->StringifyType(); }
                else
                    { return "Keyword"; }
            default: return "Unknown";
        }
    }

    auto Token::StringifyGroup () const -> std::string_view
    {
        switch (GetGroup())
        {
            case TokenGroup::ControlToken: return "ControlToken";
            case TokenGroup::ArithmeticOperator: return "ArithmeticOperator";
            case TokenGroup::AssignmentOperator: return "AssignmentOperator";
            case TokenGroup::LogicalOperator: return "LogicalOperator";
            case TokenGroup::ComparisonOperator: return "ComparisonOperator";
            case TokenGroup::GroupingOperator: return "GroupingOperator";
            case TokenGroup::Punctuation: return "Punctuation";
            case TokenGroup::Literal: return "Literal";
            case TokenGroup::Keyword:
                if (const auto kw = GetKeyword())
                    { return kw->StringifyGroup(); }
                else
                    { return "Keyword"; }
            default: return "Unknown";
        }
    }

    auto Token::Stringify () const -> std::optional<std::string>
    {
        if (const auto literal = GetLiteral())         
            { return *literal; }
        else if (const auto keyword = GetKeyword())    
            { return std::string { keyword->StringifyType() }; }
        else if (const auto symbol = StringifySymbol(mType))
            { return std::string { *symbol }; }
        else
            { return std::nullopt; }
    }
}

// Private Methods *************************************************************

namespace G10::ASM
{
    auto Token::ParseNumeric () -> void
    {
        try
        {
            if (std::holds_alternative<std::string>(mContents))
            {
                int base = 10;
                switch (mType)
                {
                    case TokenType::BinaryLiteral:          base = 2; break;
                    case TokenType::OctalLiteral:           base = 8; break;
                    case TokenType::HexadecimalLiteral:     base = 16; break;
                    case TokenType::DecimalLiteral:
                    case TokenType::CharacterLiteral:
                    case TokenType::PixelLiteral:
                    case TokenType::FloatingPointLiteral:   base = 10; break;
                    case TokenType::Parameter: {
                        const auto& lexeme = std::get<std::string>(mContents);
                        for (const char& ch : lexeme)
                        {
                            if (std::isdigit(ch) == 0)
                            {
                                mInteger = std::nullopt;
                                mSignedInteger = std::nullopt;
                                mFloat = std::nullopt;
                                return;
                            }
                        }

                        base = 10;
                    } break;
                    default:
                        mInteger = std::nullopt;
                        mSignedInteger = std::nullopt;
                        mFloat = std::nullopt;
                        return;
                }

                mInteger = std::stoull(std::get<std::string>(mContents), 
                    nullptr, base);
                mSignedInteger = std::stoll(std::get<std::string>(mContents),
                    nullptr, base);
                if (mType == TokenType::FloatingPointLiteral)
                    { mFloat = std::stod(std::get<std::string>(mContents)); }
                else
                    { mFloat = static_cast<double>(mSignedInteger.value_or(0)); }

                return;
            }
        } catch(...) {}

        mInteger = std::nullopt;
        mSignedInteger = std::nullopt;
        mFloat = std::nullopt;
    }

    auto Token::ParsePixel () -> void
    {
        // Count the number of leading backticks in the token's lexeme to
        // deduce the bit depth:
        // - `1` = `2BPP`
        const auto& lexeme = std::get<std::string>(mContents);
        auto firstNonBacktick = lexeme.find_first_not_of('`');
        auto substr = lexeme.substr(firstNonBacktick);
        switch (firstNonBacktick)
        {
            case 1: // 2BPP
            {
                if (substr.length() != 8)
                    { break; }

                std::uint8_t
                    highByte = 0x00, lowByte = 0x00,
                    highBit = 0b00, lowBit = 0b00;
                for (std::uint8_t i = 0; i < 8; ++i)
                {
                    std::uint8_t bit = 7 - i;
                    switch (substr[i])
                    {
                        case '0': highBit = 0; lowBit = 0; break;
                        case '1': highBit = 0; lowBit = 1; break;
                        case '2': highBit = 1; lowBit = 0; break;
                        case '3': highBit = 1; lowBit = 1; break;
                        default:
                            mInteger = std::nullopt;
                            mSignedInteger = std::nullopt;
                            mFloat = std::nullopt;
                            return;
                    }

                    lowByte  |= (lowBit  << bit);
                    highByte |= (highBit << bit);
                }

                mInteger = 
                    (static_cast<std::uint16_t>(highByte) << 8) |
                    static_cast<std::uint16_t>(lowByte);
                mSignedInteger = 
                    static_cast<std::int16_t>(mInteger.value_or(0));
                mFloat = static_cast<double>(mSignedInteger.value_or(0));
                return;
            } break;
            default: break;
        }

        mInteger = std::nullopt;
        mSignedInteger = std::nullopt;
        mFloat = std::nullopt;
    }
}
