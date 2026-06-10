/**
 * @file    G10.ASM.Lexer.cpp
 * @brief   Contains implementations for the G10 Assembler's lexer class,
 *          and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Lexer.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::ASM
{
    Lexer::Lexer (Diagnostic& pDiag) :
        mDiag { pDiag } {}
}

// Public Methods - Source *****************************************************

namespace G10::ASM
{
    auto Lexer::LexString (const std::string& pSource, bool pSecondPass) -> bool
    {
        mSource = pSource;
        mSecondPass = pSecondPass;
        mSourceIndex = 0;
        mSourceChar = (pSource.empty() == true) ? '\0' : mSource[0];
        mLocation = SourceLocation { "<string>", 1, 1 };
        mTokens.clear();
        
        return Tokenize();
    }

    auto Lexer::LexFile (const fs::path& pPath, bool pSecondPass) -> bool
    {
        if (fs::exists(pPath) == false)
        {
            mDiag.ReportError("Source file '{}' not found.", pPath.string());
            return false;
        }

        auto fileSize = fs::file_size(pPath);
        std::fstream file { pPath, std::ios::in | std::ios::binary };
        if (file.is_open() == false)
        {
            mDiag.ReportError("Source file '{}' could not be opened for reading.",
                pPath.string());
            return false;
        }

        mSource.resize(fileSize);
        file.read(mSource.data(), fileSize);
        file.close();

        mSecondPass = pSecondPass;
        mSourceIndex = 0;
        mSourceChar = (mSource.empty() == true) ? '\0' : mSource[0];
        mLocation = SourceLocation { pPath.string(), 1, 1 };
        mTokens.clear();

        return Tokenize();
    }
}

// Private Methods - Tokenization **********************************************

namespace G10::ASM
{
    auto Lexer::Tokenize () -> bool
    {
        while (mSourceIndex < mSource.size())
        {
            SkipWhitespace();
            if (SkipComment() == true)
                { continue; }
            if (mSourceChar == '\0')
                { break; }

            std::optional<Token> token { std::nullopt };
            if (
                std::isalpha(mSourceChar) ||
                mSourceChar == '_' ||
                mSourceChar == '.' ||
                mSourceChar == '{'
            ) { token = CollectIdentifier(); }
            else if (
                mSourceChar == '@'
            ) { token = CollectParameter(); }
            else if (
                mSourceChar == '"' 
            ) { token = CollectString(); }
            else if (
                mSourceChar == '\''
            ) { token = CollectChar(); }
            else if (
                mSourceChar == '`'
            ) { token = CollectPixel(); }
            else if (
                mSourceChar == '%'
            ) { token = CollectBinary(true); }
            else if (
                mSourceChar == '&'
            ) { token = CollectOctal(true); }
            else if (
                mSourceChar == '$'
            ) { token = CollectHexadecimal(true); }
            else if (
                std::isdigit(mSourceChar)
            ) { token = CollectDecimal(); }
            else { token = CollectSymbol(); }

            if (token.has_value() == true)
            {
                if (mSecondPass == true && token->IsValidSecondPass() == false)
                {
                    mDiag.ReportError(mLocation,
                        "'{}' token '{}' is invalid during second-pass lexing.",
                        token->StringifyGroup(), token->StringifyType());
                    return false;
                }

                mTokens.push_back(*token);
            }
            else 
            { 
                if (mHintFile == true)
                    { mHintFile = mHint; }
                else if (mHintLine == true)
                    { mHintLine = mHint; }
                else
                    { return false; } 
            }
        }

        return true;
    }

    auto Lexer::Peek (std::size_t pOffset) -> char
    {
        if (mSourceIndex + pOffset < mSource.size())
            { return mSource[mSourceIndex + pOffset]; }
        else { return '\0'; }
    }

    auto Lexer::Next () -> char
    {
        if (mSourceIndex + 1 >= mSource.size())
            { mSourceChar = '\0'; }
        else
        {
            mSourceChar = mSource[++mSourceIndex];
            if (mSourceChar == '\n')
                { mLocation.mLine++; mLocation.mColumn = 1; }
            else
                { mLocation.mColumn++; }
        }

        return mSourceChar;
    }

    auto Lexer::SkipWhitespace () -> void
    {
        while (std::isspace(mSourceChar))
        {
            if (mSourceChar == '\n')
                { mTokens.emplace_back(TokenType::NewLine, mLocation); }

            Next();
        }
    }

    auto Lexer::SkipComment () -> bool
    {
        if (mSourceChar == ';')
        {
            while (mSourceChar != '\0' && mSourceChar != '\n')
                { Next(); }

            return true;
        }

        return false;
    }
}

// Private Methods - Token Collection ******************************************

namespace G10::ASM
{
    auto Lexer::CollectIdentifier () -> std::optional<Token>
    {
        std::string lexeme {};
        std::size_t interpolationDepth { 0 };
        while (
            std::isalnum(mSourceChar) ||
            mSourceChar == '_' ||
            mSourceChar == '.' ||
            mSourceChar == '{' ||
            mSourceChar == '}' ||
            mSourceChar == ':' ||
            mSourceChar == '@' ||
            mSourceChar == '?' ||
            mSourceChar == '\"'
        )
        {
            if (mSourceChar == '{' || mSourceChar == '}')
            {
                if (mSecondPass == true)
                {
                    mDiag.ReportError(mLocation,
                        "'{}' in identifier encountered during second pass lexing.",
                        mSourceChar);
                    return std::nullopt;
                }

                if (mSourceChar == '{')
                    { interpolationDepth++; }
                else if (mSourceChar == '}')
                {
                    if (interpolationDepth == 0)
                    {
                        mDiag.ReportError(mLocation,
                            "Missing opening brace in identifier interpolation.");
                        return std::nullopt;
                    }

                    interpolationDepth--;
                }
            }
            else if (mSourceChar == ':')
            {
                if (mSecondPass == true || interpolationDepth == 0)
                    { break; }

                if (
                    lexeme.size() < 2 || 
                    lexeme[lexeme.size() - 2] != '{' ||
                    std::isalpha(lexeme.back()) == 0
                )
                {
                    mDiag.ReportError(mLocation,
                        "Format specifier in identifer interpolation is either "
                        "missing or malformed.");
                    return std::nullopt;
                }
            }
            else if (mSourceChar == '@')
            {
                if (mSecondPass == true || interpolationDepth == 0)
                    { break; }

                if (interpolationDepth == 0)
                {
                    mDiag.ReportError(mLocation,
                        "'@' in identifer encountered outside of interpolation.");
                    return std::nullopt;
                }
            }
            else if (mSourceChar == '?')
            {
                if (mSecondPass == true || interpolationDepth == 0)
                    { break; }

                if (
                    interpolationDepth == 0 ||
                    lexeme.back() != '@'
                )
                {
                    mDiag.ReportError(mLocation,
                        "'?' in identifer encountered out of place.");
                    return std::nullopt;
                }
            }
            else if (mSourceChar == '\"')
            {
                if (mSecondPass == true || interpolationDepth == 0)
                    { break; }
            }

            lexeme += mSourceChar;
            Next();
        }

        if (interpolationDepth > 0)
        {
            mDiag.ReportError(mLocation, 
                "Missing closing brace in identifier interpolation.");
            return std::nullopt;
        }

        const auto& keyword = Keyword::Lookup(lexeme);
        if (keyword.has_value() == true)
        {
            if (keyword->Is<Hint>())
            {
                if (mSecondPass == false)
                {
                    mDiag.ReportError(mLocation,
                        "Hint directive '{}' encountered during first-pass lexing.",
                        keyword->StringifyType());
                    return std::nullopt;
                }

                const auto type = *keyword->GetTypeIf<Hint>();
                if (type == Hint::FILE)
                    { mHintFile = true; }
                else if (type == Hint::LINE)
                    { mHintLine = true; }
                else
                {
                    mDiag.ReportError(mLocation,
                        "Un-recognized hint directive: '{}'.",
                        keyword->StringifyType());
                    return std::nullopt;
                }

                mHint = true;
                return std::nullopt;
            }

            return Token { *keyword, mLocation };
        }

        return Token { TokenType::Identifier, std::move(lexeme), mLocation };
    }

    auto Lexer::CollectParameter () -> std::optional<Token>
    {
        if (mSecondPass == true)
        {
            mDiag.ReportError(mLocation,
                "Parameter token encountered during second-pass lexing.");
            return std::nullopt;
        }

        Next(); // Skip '@'.

        std::string lexeme {};
        bool isInteger { false };
        bool isUnique { false };
        while (
            std::isalnum(mSourceChar) ||
            mSourceChar == '_' ||
            mSourceChar == '?'
        )
        {
            if (mSourceChar == '?')
            {
                if (lexeme.empty() == false)
                {
                    mDiag.ReportError(mLocation,
                        "'?' must be the only character in a unique placeholder parameter.");
                    return std::nullopt;
                }

                isUnique = true;
            }
            else if (isInteger == true)
            {
                if (std::isdigit(mSourceChar) == 0)
                    { break; }
            }
            else if (lexeme.empty() == true && std::isdigit(mSourceChar))
            {
                isInteger = true;
            }

            lexeme += mSourceChar;
            Next();

            if (isUnique == true)
                { break; }
        }

        if (lexeme.empty() == true)
        {
            mDiag.ReportError(mLocation,
                "Expected parameter name after '@'.");
            return std::nullopt;
        }
        
        return Token { TokenType::Parameter, std::move(lexeme), mLocation };
    }

    auto Lexer::CollectString () -> std::optional<Token>
    {
        Next();     // Skip the opening quote...

        std::string lexeme {};
        std::size_t interpolationDepth { 0 };
        bool escape { false };
        while (mSourceChar != '\0' || mSourceChar != '\n')
        {
            if (mSourceChar == '\\')
            {
                escape = !escape;
                if (escape == true)
                    { Next(); continue; }
            }
            else if (mSourceChar == '\"')
            {
                if (escape == false)
                    { break; }
                else
                    { escape = false; }
            }
            else if (
                mSecondPass == false && 
                (mSourceChar == '{' || mSourceChar == '}')
            )
            {
                if (escape == false)
                {
                    if (mSourceChar == '{')
                        { interpolationDepth++; }
                    else
                    {
                        if (interpolationDepth == 0)
                        {
                            mDiag.ReportError(mLocation,
                                "Missing opening brace in string interpolation.");
                            return std::nullopt;
                        }

                        interpolationDepth--;
                    }
                }
                else
                    { escape = false; }
            }
            else if (escape == true)
            {
                escape = false;
            }

            lexeme += mSourceChar;
            Next();
        }

        if (mSourceChar != '\"')
        {
            mDiag.ReportError(mLocation, "Missing closing '\"' in string literal.");
            return std::nullopt;
        }
        else if (interpolationDepth > 0)
        {
            mDiag.ReportError(mLocation, "Missing closing brace in string interpolation.");
            return std::nullopt;
        }

        Next(); // Skip closing quote.

        if (mHintFile == true)
        {
            mLocation.mPath = lexeme;
            mLocation.mLine = 0;
            mHint = false;
            return std::nullopt;
        }

        return Token { TokenType::StringLiteral, std::move(lexeme), mLocation };
    }

    auto Lexer::CollectChar () -> std::optional<Token>
    {
        Next(); // Skip opening quote.
        
        std::string lexeme {};
        while (
            mSourceIndex < mSource.size() &&
            mSourceChar != '\''
        ) { lexeme += mSourceChar; Next(); }

        if (mSourceChar != '\'')
        {
            mDiag.ReportError(mLocation, "Missing closing '\'' in string literal.");
            return std::nullopt;
        }

        Next(); // Skip closing quote.

        if (lexeme[0] == '\\')
        {
            std::uint64_t charCode = 0;
            if (lexeme.size() == 1)
            {
                charCode = '\\';
            }
            else switch (lexeme[1])
            {
                case '0':   charCode = '\0'; break;
                case 'n':   charCode = '\n'; break;
                case 't':   charCode = '\t'; break;
                case '\\':  charCode = '\\'; break;
                case '\'':  charCode = '\''; break;
                case '\"':  charCode = '\"'; break;
                case 'x':
                case 'X': {
                    if (lexeme.size() != 4)
                    {
                        mDiag.ReportError(mLocation,
                            "Malformed hexadecimal escape sequence in character literal.");
                        return std::nullopt;
                    }

                    charCode = std::stoull(lexeme.substr(2), nullptr, 16);
                } break;
                case 'u':
                case 'U': {
                    if ((lexeme[1] == 'u' && lexeme.size() != 6) ||
                        (lexeme[1] == 'U' && lexeme.size() != 10))
                    {
                        mDiag.ReportError(mLocation,
                            "Malformed Unicode escape sequence in character literal.");
                        return std::nullopt;
                    }

                    charCode = std::stoull(lexeme.substr(2), nullptr, 16);
                } break;
                default:    charCode = lexeme[1]; break;
            }

            return Token { charCode, mLocation };
        }
        else
        {
            if (lexeme.size() != 1)
            {
                mDiag.ReportError(mLocation,
                    "Non-escaped character literals must be one charater in length.");
                return std::nullopt;
            }

            return Token { static_cast<std::uint64_t>(lexeme[0]), mLocation };
        }
    }

    auto Lexer::CollectPixel () -> std::optional<Token>
    {
        bool leading = true;
        std::uint8_t leadingCount = 0;
        std::string lexeme {};
        while (mSourceChar == '`' || std::isxdigit(mSourceChar))
        {
            if (mSourceChar == '`')
            {
                if (leading == false)
                {
                    mDiag.ReportError(mLocation,
                        "Non-leading '`' in pixel literal.");
                    return std::nullopt;
                }

                leadingCount++;
            }
            else
            {
                if (leadingCount != 1)
                {
                    mDiag.ReportError(mLocation,
                        "Invalid bit depth in pixel literal.");
                    return std::nullopt;
                }

                if (
                    (leadingCount == 1 && (mSourceChar < '0' || mSourceChar > '3'))
                )
                {
                    mDiag.ReportError(mLocation,
                        "Malformed pixel literal.");
                    return std::nullopt;
                }

                leading = false;
            }

            lexeme += mSourceChar;
            Next();
        }
        
        if (leadingCount == 1)
        {
            std::uint8_t
                highByte = 0x00, lowByte = 0x00,
                highBit = 0b00, lowBit = 0b00;
            for (std::uint8_t i = 0; i < 8; ++i)
            {
                std::uint8_t bit = 7 - i;
                switch (lexeme[i + leadingCount])
                {
                    case '0': highBit = 0; lowBit = 0; break;
                    case '1': highBit = 0; lowBit = 1; break;
                    case '2': highBit = 1; lowBit = 0; break;
                    case '3': highBit = 1; lowBit = 1; break;
                    default:
                        mDiag.ReportError(mLocation,
                            "Malformed 2BPP pixel literal.");
                        return std::nullopt;
                }

                lowByte  |= (lowBit  << bit);
                highByte |= (highBit << bit);
            }

            std::uint64_t integer = 
                (static_cast<std::uint64_t>(highByte) << 8) |
                static_cast<std::uint64_t>(lowByte);
            return Token { TokenType::PixelLiteral, integer, mLocation };
        }
        else
        {
            mDiag.ReportError(mLocation,
                "Invalid pixel literal.");
            return std::nullopt;
        }
    }

    auto Lexer::CollectBinary (bool pPercentPrefix) -> std::optional<Token>
    {
        if (pPercentPrefix == true) { Next(); /* Skip '%' Prefix. */ }
        else { Next(); Next(); /* Skip '0b' or '0B' prefix. */ }

        std::string lexeme {};
        while (mSourceChar == '0' || mSourceChar == '1')
            { lexeme += mSourceChar; Next(); }

        if (lexeme.empty() == true)
        {
            if (pPercentPrefix == true)
            {
                if (mSourceChar == '=') { Next(); return Token { TokenType::AssignModulo, mLocation }; }
                return Token { TokenType::Modulo, mLocation };
            }

            mDiag.ReportError(mLocation,
                "Expected binary literal after prefix.");
            return std::nullopt;
        }

        return Token { TokenType::BinaryLiteral, std::move(lexeme), mLocation };
    }

    auto Lexer::CollectOctal (bool pAmpersandPrefix) -> std::optional<Token>
    {
        if (pAmpersandPrefix == true) { Next(); /* Skip '&' Prefix. */ }
        else { Next(); Next(); /* Skip '0o' or '0O' prefix. */ }

        std::string lexeme {};
        while (mSourceChar >= '0' && mSourceChar <= '7')
            { lexeme += mSourceChar; Next(); }

        if (lexeme.empty() == true)
        {
            if (pAmpersandPrefix == true)
            {
                if      (mSourceChar == '=') { Next(); return Token { TokenType::AssignAnd, mLocation }; }
                else if (mSourceChar == '&') { Next(); return Token { TokenType::LogicalAnd, mLocation }; }
                return Token { TokenType::BitwiseAnd, mLocation };
            }

            mDiag.ReportError(mLocation,
                "Expected octal literal after prefix.");
            return std::nullopt;
        }

        return Token { TokenType::OctalLiteral, std::move(lexeme), mLocation };
    }

    auto Lexer::CollectHexadecimal (bool pDollarPrefix) -> std::optional<Token>
    {
        if (pDollarPrefix == true) { Next(); /* Skip '$' Prefix. */ }
        else { Next(); Next(); /* Skip '0x' or '0X' prefix. */ }

        std::string lexeme {};
        while (std::isxdigit(mSourceChar))
            { lexeme += mSourceChar; Next(); }

        if (lexeme.empty() == true)
        {
            mDiag.ReportError(mLocation,
                "Expected hexadecimal literal after prefix.");
            return std::nullopt;
        }

        return Token { TokenType::HexadecimalLiteral, std::move(lexeme), mLocation };
    }

    auto Lexer::CollectDecimal () -> std::optional<Token>
    {
        if (mSourceChar == '0')
        {
            switch (Peek())
            {
                case 'b':
                case 'B': return CollectBinary(false);
                case 'o':
                case 'O': return CollectOctal(false);
                case 'x':
                case 'X': return CollectHexadecimal(false);
            }
        }

        std::string lexeme {};
        bool isFloat { false };
        while (std::isdigit(mSourceChar) || mSourceChar == '.')
        {
            if (mSourceChar == '.')
            {
                if (mSecondPass == true)
                {
                    mDiag.ReportError(mLocation,
                        "Floating-point literal encountered during second-pass lexing.");
                    return std::nullopt;
                }
                else if (isFloat == true)
                {
                    mDiag.ReportError(mLocation,
                        "More than one '.' encountered in floating-point literal.");
                    return std::nullopt;
                }

                isFloat = true;
            }

            lexeme += mSourceChar;
            Next();
        }

        if (lexeme.empty() == true)
        {
            mDiag.ReportError(mLocation, "Expected decimal literal.");
            return std::nullopt;
        }
        else if (lexeme.back() == '.')
        {
            mDiag.ReportError(mLocation, 
                "Expected fractional part after '.' in floating-point literal.");
            return std::nullopt;
        }

        if (mHintLine == true)
        {
            mLocation.mLine = std::stoull(lexeme) - 1;
            mHint = false;
            return std::nullopt;
        }

        return Token {
            (isFloat == true) ?
                TokenType::FloatingPointLiteral :
                TokenType::DecimalLiteral,
            std::move(lexeme), mLocation
        };
    }

    auto Lexer::CollectSymbol () -> std::optional<Token>
    {
        char p1 = Peek(1), p2 = Peek(2);
        switch (mSourceChar)
        {
            case '+':
                Next();
                if (p1 == '=') { Next(); return Token { TokenType::AssignPlus, mLocation }; }
                return Token { TokenType::Plus, mLocation };
            case '-':
                Next();
                if (p1 == '=') { Next(); return Token { TokenType::AssignMinus, mLocation }; }
                return Token { TokenType::Minus, mLocation };
            case '*':
                Next();
                if (p1 == '=') { Next(); return Token { TokenType::AssignTimes, mLocation }; }
                else if (p1 == '*')
                {
                    Next();
                    if (p1 == '=') { Next(); return Token { TokenType::AssignExponent, mLocation }; }
                    return Token { TokenType::Exponent, mLocation };
                }
                return Token { TokenType::Times, mLocation };
            case '/':
                Next();
                if (p1 == '=') { Next(); return Token { TokenType::AssignDivide, mLocation }; }
                return Token { TokenType::Divide, mLocation };
            case '>':
                Next();
                if (p1 == '=') { Next(); return Token { TokenType::CompareGreaterEqual, mLocation }; }
                else if (p1 == '>')
                {
                    Next();
                    if (p2 == '=') { Next(); return Token { TokenType::AssignShiftRight, mLocation }; }
                    return Token { TokenType::BitwiseShiftRight, mLocation };
                }
                return Token { TokenType::CompareGreater, mLocation };
            case '<':
                Next();
                if (p1 == '=') { Next(); return Token { TokenType::CompareLessEqual, mLocation }; }
                else if (p1 == '<')
                {
                    Next();
                    if (p2 == '=') { Next(); return Token { TokenType::AssignShiftLeft, mLocation }; }
                    return Token { TokenType::BitwiseShiftLeft, mLocation };
                }
                return Token { TokenType::CompareLess, mLocation };
            case '|':
                Next();
                if (p1 == '=') { Next(); return Token { TokenType::AssignOr, mLocation }; }
                else if (p1 == '|')
                {
                    Next();
                    return Token { TokenType::LogicalOr, mLocation };
                }
                return Token { TokenType::BitwiseOr, mLocation };
            case '^':
                Next();
                if (p1 == '=') { Next(); return Token { TokenType::AssignXor, mLocation }; }
                return Token { TokenType::BitwiseXor, mLocation };
            case '~':
                Next();
                return Token { TokenType::BitwiseNot, mLocation };
            case '=':
                Next();
                if (p1 == '=') { Next(); return Token { TokenType::CompareEqual, mLocation }; }
                return Token { TokenType::AssignEqual, mLocation };
            case '!':
                Next();
                if (p1 == '=') { Next(); return Token { TokenType::CompareNotEqual, mLocation }; }
                return Token { TokenType::LogicalNot, mLocation };
            case '(':
                Next();
                return Token { TokenType::LeftParenthesis, mLocation };
            case ')':
                Next();
                return Token { TokenType::RightParenthesis, mLocation };
            case '[':
                Next();
                return Token { TokenType::LeftBracket, mLocation };
            case ']':
                Next();
                return Token { TokenType::RightBracket, mLocation };
            case ',':
                Next();
                return Token { TokenType::Comma, mLocation };
            case ':':
                Next();
                return Token { TokenType::Colon, mLocation };
            default:
                mDiag.ReportError(mLocation, "Unexpected character '{}' encountered.",
                    mSourceChar);
                return std::nullopt;
        }
    }
}
