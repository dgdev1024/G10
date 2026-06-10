/**
 * @file    G10.ASM.Preprocessor.Expressions.cpp
 * @brief   Contains implementations for the G10 Assembler Preprocessor's
 *          expression evaluation methods, and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Lexer.hpp>
#include <G10.ASM.Preprocessor.hpp>

// Private Methods - Expressions - Helpers *************************************

namespace G10::ASM
{
    auto Preprocessor::CollectAndEvaluate (TokenCursor& pCursor) -> PreprocessorValue
    {
        auto        startIndex = pCursor.GetIndex();
        auto        exprSlice = pCursor.CollectExpression();
        TokenCursor exprCursor { exprSlice };
        auto        exprValue = EvaluateExpression(exprCursor);
        if (exprValue.IsUndefined())
            { pCursor.SetIndex(startIndex); }

        return exprValue;
    }
}

// Private Methods - Expressions - Applicators *********************************

namespace G10::ASM
{
    auto Preprocessor::ApplyCompound (PreprocessorValue& pLeft, 
        const PreprocessorValue& pRight, const Token& pOperator) -> PreprocessorValue&
    {
        switch (pOperator.mType)
        {
            case TokenType::AssignEqual: pLeft = pRight; break;
            case TokenType::AssignPlus: pLeft += pRight; break;
            case TokenType::AssignMinus: pLeft -= pRight; break;
            case TokenType::AssignTimes: pLeft *= pRight; break;
            case TokenType::AssignExponent: pLeft = stx::pow(pLeft, pRight); break;
            case TokenType::AssignDivide: pLeft /= pRight; break;
            case TokenType::AssignModulo: pLeft %= pRight; break;
            case TokenType::AssignAnd: pLeft &= pRight; break;
            case TokenType::AssignOr: pLeft |= pRight; break;
            case TokenType::AssignXor: pLeft ^= pRight; break;
            case TokenType::AssignShiftLeft: pLeft <<= pRight; break;
            case TokenType::AssignShiftRight: pLeft >>= pRight; break;
            default: pLeft = {};
        }

        return pLeft;
    }

    auto Preprocessor::ApplyBinary (const PreprocessorValue& pLeft, 
        const PreprocessorValue& pRight, const Token& pOperator) -> PreprocessorValue
    {
        switch (pOperator.mType)
        {
            case TokenType::Plus: return pLeft + pRight;
            case TokenType::Minus: return pLeft - pRight;
            case TokenType::Times: return pLeft * pRight;
            case TokenType::Exponent: return stx::pow(pLeft, pRight);
            case TokenType::Divide: return pLeft / pRight;
            case TokenType::Modulo: return pLeft % pRight;
            case TokenType::BitwiseAnd: return pLeft & pRight;
            case TokenType::BitwiseOr: return pLeft | pRight;
            case TokenType::BitwiseXor: return pLeft ^ pRight;
            case TokenType::BitwiseShiftLeft: return pLeft << pRight;
            case TokenType::BitwiseShiftRight: return pLeft >> pRight;
            case TokenType::LogicalAnd: return pLeft && pRight; break;
            case TokenType::LogicalOr: return pLeft || pRight; break;
            case TokenType::CompareEqual: return pLeft == pRight; break;
            case TokenType::CompareNotEqual: return pLeft != pRight; break;
            case TokenType::CompareGreater: return pLeft > pRight; break;
            case TokenType::CompareGreaterEqual: return pLeft >= pRight; break;
            case TokenType::CompareLess: return pLeft < pRight; break;
            case TokenType::CompareLessEqual: return pLeft <= pRight; break;
            default: return {};
        }
    }

    auto Preprocessor::ApplyUnary (const PreprocessorValue& pRight, 
        const Token& pOperator) -> PreprocessorValue
    {
        switch (pOperator.mType)
        {
            case TokenType::Plus: return +pRight;
            case TokenType::Minus: return -pRight;
            case TokenType::BitwiseNot: return ~pRight;
            case TokenType::LogicalNot: return !pRight;
            default: return {};
        }
    }
}

// Private Methods - Expressions - Arithmetic **********************************

namespace G10::ASM
{
    #define EvalBinary(pClause, pNext) \
        auto left = pNext(pCursor); \
        while (pCursor.IsAtEnd() == false) { \
            auto op = pCursor.GetNextToken(); \
            if ((pClause)) { \
                pCursor.Skip(); \
                auto right = pNext(pCursor); \
                left = ApplyBinary(left, right, op); \
            } else { break; } \
        } \
        return left;

    auto Preprocessor::EvaluateExpression (TokenCursor& pCursor) -> PreprocessorValue
    {
        return EvaluateLogicalOr(pCursor);
    }

    auto Preprocessor::EvaluateLogicalOr (TokenCursor& pCursor) -> PreprocessorValue
    {
        EvalBinary(
            op.mType == TokenType::LogicalOr,
            EvaluateLogicalAnd
        );
    }

    auto Preprocessor::EvaluateLogicalAnd (TokenCursor& pCursor) -> PreprocessorValue
    {
        EvalBinary(
            op.mType == TokenType::LogicalAnd,
            EvaluateBitwiseOr
        );
    }

    auto Preprocessor::EvaluateBitwiseOr (TokenCursor& pCursor) -> PreprocessorValue
    {
        EvalBinary(
            op.mType == TokenType::BitwiseOr,
            EvaluateBitwiseXor
        );
    }

    auto Preprocessor::EvaluateBitwiseXor (TokenCursor& pCursor) -> PreprocessorValue
    {
        EvalBinary(
            op.mType == TokenType::BitwiseXor,
            EvaluateBitwiseAnd
        );
    }

    auto Preprocessor::EvaluateBitwiseAnd (TokenCursor& pCursor) -> PreprocessorValue
    {
        EvalBinary(
            op.mType == TokenType::BitwiseAnd,
            EvaluateEquality
        );
    }

    auto Preprocessor::EvaluateEquality (TokenCursor& pCursor) -> PreprocessorValue
    {
        EvalBinary(
            op.mType == TokenType::CompareEqual ||
            op.mType == TokenType::CompareNotEqual,
            EvaluateComparison
        );
    }

    auto Preprocessor::EvaluateComparison (TokenCursor& pCursor) -> PreprocessorValue
    {
        EvalBinary(
            op.mType == TokenType::CompareLess ||
            op.mType == TokenType::CompareLessEqual ||
            op.mType == TokenType::CompareGreater ||
            op.mType == TokenType::CompareGreaterEqual,
            EvaluateShift
        );
    }

    auto Preprocessor::EvaluateShift (TokenCursor& pCursor) -> PreprocessorValue
    {
        EvalBinary(
            op.mType == TokenType::BitwiseShiftLeft ||
            op.mType == TokenType::BitwiseShiftRight,
            EvaluateAdditive
        );
    }

    auto Preprocessor::EvaluateAdditive (TokenCursor& pCursor) -> PreprocessorValue
    {
        EvalBinary(
            op.mType == TokenType::Plus ||
            op.mType == TokenType::Minus,
            EvaluateMultiplicative
        );
    }

    auto Preprocessor::EvaluateMultiplicative (TokenCursor& pCursor) -> PreprocessorValue
    {
        EvalBinary(
            op.mType == TokenType::Times ||
            op.mType == TokenType::Divide ||
            op.mType == TokenType::Modulo,
            EvaluateExponent
        );
    }

    #undef EvalBinary

    auto Preprocessor::EvaluateExponent (TokenCursor& pCursor) -> PreprocessorValue
    {
        auto left = EvaluateUnary(pCursor);
        while (pCursor.IsAtEnd() == false)
        {
            auto op = pCursor.GetNextToken();
            if (op.mType == TokenType::Exponent)
            {
                pCursor.Skip();
                auto right = EvaluateExponent(pCursor);
                left = ApplyBinary(left, right, op);
            } else { break; }
        }

        return left;
    }

    auto Preprocessor::EvaluateUnary (TokenCursor& pCursor) -> PreprocessorValue
    {
        if (pCursor.IsAtEnd() == true)
            { return EvaluatePrimary(pCursor); }

        auto op = pCursor.GetNextToken();
        if (op.mType == TokenType::Plus)
            { pCursor.Skip(); return EvaluateUnary(pCursor); }
        else if (
            op.mType == TokenType::Minus ||
            op.mType == TokenType::BitwiseNot ||
            op.mType == TokenType::LogicalNot
        )
        {
            pCursor.Skip();
            auto right = EvaluateUnary(pCursor);
            return ApplyUnary(right, op);
        }

        return EvaluatePrimary(pCursor);
    }

    auto Preprocessor::EvaluatePrimary (TokenCursor& pCursor) -> PreprocessorValue
    {
        const auto& token = pCursor.GetNextToken();
        if (pCursor.IsAtEnd() == true)
        {
            mDiag.ReportError(token.mLocation,
                "Unexpected end of token stream while evaluating expression.");
            mPendingStatus = PreprocessStatus::Error;
            return {};
        }
        
        else if (token.mType == TokenType::LeftParenthesis)
        {
            pCursor.Skip();
            auto pValue = EvaluateExpression(pCursor);
            if (pCursor.IsAtEnd() == false && 
                pCursor.GetNextToken().mType == TokenType::RightParenthesis)
            {
                pCursor.Skip();
                return pValue;
            }
            else
            {
                mDiag.ReportError(token.mLocation,
                    "Expected closing parenthesis in parenthesized expression.");
                mPendingStatus = PreprocessStatus::Error;
                return {};
            }
        }
        else if (token.IsInteger() == true)
        {
            pCursor.Skip();
            if (token.mType == TokenType::FloatingPointLiteral)
                { return PreprocessorFixedPoint { token.mFloat.value_or(0.0) }; }
            else
                { return PreprocessorInteger { token.mSignedInteger.value_or(0) }; }
        }
        else if (token.mType == TokenType::StringLiteral)
        {
            pCursor.Skip();
            auto lexeme = InterpolateString(token.mLocation,
                token.Stringify().value_or(""));
            if (lexeme.has_value() == true)
            {
                return PreprocessorString { lexeme.value_or("") };
            }
            else
            {
                return {};
            }
        }
        else if (token.mType == TokenType::Identifier)
        {
            pCursor.Skip();
            auto lexeme = InterpolateIdentifier(token.mLocation,
                token.Stringify().value_or(""));
                
            if (lexeme.has_value() == true)
            {
                auto val = EvaluateSymbol(*lexeme);
                if (val.IsUndefined() == true)
                {
                    // mDiag.ReportError(token.mLocation,
                    //     "Symbol '{}' is not defined.", *lexeme);
                    // mPendingStatus = PreprocessStatus::Error;
                    return {};
                }
                
                return val;
            }
            else
            {
                return {};
            }
        }
        else if (token.mType == TokenType::Parameter)
        {
            if (mMacroCallStack.empty() == true)
            {
                mDiag.ReportError(token.mLocation,
                    "Parameter token outside of macro body.");
                mPendingStatus = PreprocessStatus::Error;
                return {};
            }

            pCursor.Skip();
            auto lexeme = token.Stringify();
            if (lexeme.has_value() == true)
            {
                auto& call = mMacroCallStack.back();
                const auto& macro = *call.mMacro;

                PreprocessorMacroArgument arg {};
                if (token.HasInteger() == true)
                {
                    auto integer = token.mInteger.value_or(0);
                    if (integer == 0)
                    {
                        PreprocessorValue val { macro.mName };
                        return val;
                    }
                    else if (integer > call.mArguments.size())
                    {
                        mDiag.ReportError(token.mLocation,
                            "Macro argument index #{} is out of range.", integer);
                        mPendingStatus = PreprocessStatus::Error;
                        return {};
                    }

                    arg = call.mArguments[integer - 1];
                }
                else if (lexeme == "?")
                {
                    PreprocessorValue val { macro.mInvocationCount };
                    return val;
                }
                else
                {
                    std::size_t index = std::string::npos;
                    for (std::size_t i = 0; i < macro.mNamedParameters.size(); ++i)
                    {
                        if (lexeme == macro.mNamedParameters[i])
                            { index = i; break; }
                    }

                    if (index == std::string::npos)
                    {
                        mDiag.ReportError(token.mLocation,
                            "Named argument '{}' not found in macro '{}'.",
                            *lexeme, macro.mName);
                        mPendingStatus = PreprocessStatus::Error;
                        return {};
                    }
                    else if (index >= call.mArguments.size())
                    {
                        mDiag.ReportError(token.mLocation,
                            "Named argument '{}' in macro '{}' resolves to an argument "
                            "index which is out of range.", *lexeme, macro.mName);
                        mPendingStatus = PreprocessStatus::Error;
                        return {};
                    }

                    arg = call.mArguments[index];
                }

                if (std::holds_alternative<PreprocessorValue>(arg))
                {
                    return (*std::get_if<PreprocessorValue>(&arg));
                }
                else
                {
                    mDiag.ReportError(token.mLocation,
                        "Macro argument does not resolve to a value.");
                    mPendingStatus = PreprocessStatus::Error;
                    return {};
                }
            }
            else
            {
                return {};
            }
        }
        else if (token.mType == TokenType::Keyword)
        {
            pCursor.Skip();
            if (const auto kw = token.GetKeyword())
            {
                if (kw->Is<PreprocessorBuiltinSymbol>())
                {
                    auto val = EvaluateBuiltinSymbol(token.mLocation, 
                        kw->GetType<PreprocessorBuiltinSymbol>());
                    if (val.IsUndefined() == true)
                    {
                        mDiag.ReportInfo(token.mLocation,
                            "Evaluating builtin symbol: '{}'.",
                            token.StringifyType());
                        mPendingStatus = PreprocessStatus::Error;
                    }

                    return val;
                }
                else if (kw->Is<PreprocessorFunction>())
                {
                    auto val = EvaluateBuiltinFunction(pCursor,
                        kw->GetType<PreprocessorFunction>());
                    if (val.IsUndefined() == true)
                    {
                        mDiag.ReportInfo(token.mLocation,
                            "Evaluating builtin function: '{}'.",
                            token.StringifyType());
                        mPendingStatus = PreprocessStatus::Error;
                    }

                    return val;
                }
            }

            mDiag.ReportError(token.mLocation,
                "Encountered unexpected '{}' token: '{}'.",
                token.StringifyGroup(),
                token.StringifyType());
            mPendingStatus = PreprocessStatus::Error;
            return {};
        }
        else
        {
            mDiag.ReportError(token.mLocation,
                "Encountered unexpected '{}' token: '{}'.",
                token.StringifyGroup(),
                token.StringifyType());
            mPendingStatus = PreprocessStatus::Error;
            return {};
        }
    }
}

// Private Methods - Expressions - Symbols *************************************

namespace G10::ASM
{
    static auto GetDateString () -> std::string
    {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf {};
        localtime_r(&now_time_t, &tm_buf);

        return std::format("{:04d}-{:02d}-{:02d}", 
            tm_buf.tm_year + 1900, 
            tm_buf.tm_mon + 1, 
            tm_buf.tm_mday);
    }

    static auto GetTimeString () -> std::string
    {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf {};
        localtime_r(&now_time_t, &tm_buf);

        return std::format("{:02d}:{:02d}:{:02d}", 
            tm_buf.tm_hour, 
            tm_buf.tm_min, 
            tm_buf.tm_sec);
    }

    static auto GetRandom32 () -> std::int64_t
    {
        static std::random_device rd;
        static std::mt19937 gen { rd() };
        static std::uniform_int_distribution<std::uint32_t> dist;
        return static_cast<std::int64_t>(dist(gen));
    }

    static auto GetRandom64 () -> std::int64_t
    {
        static std::random_device rd;
        static std::mt19937_64 gen { rd() };
        static std::uniform_int_distribution<std::uint64_t> dist;
        return static_cast<std::int64_t>(dist(gen));
    }

    auto Preprocessor::EvaluateSymbol (const std::string& pName) 
        -> PreprocessorValue
    {
        auto findIt = mSymbols.find(pName);
        if (findIt != mSymbols.end())
            { return findIt->second.mValue; }
        else
            { return {}; }
    }

    auto Preprocessor::EvaluateBuiltinSymbol (const SourceLocation& pLocation, 
        const PreprocessorBuiltinSymbol& pSymbol) -> PreprocessorValue
    {
        switch (pSymbol)
        {
            case PreprocessorBuiltinSymbol::_NARG: {
                if (mMacroCallStack.empty() == true)
                {
                    mDiag.ReportError("'_NARG' outside of macro body.");
                    return {};
                }

                auto count = static_cast<PreprocessorInteger>(
                    mMacroCallStack.back().mArguments.size()
                );
                return PreprocessorValue { count };
            } break;
            case PreprocessorBuiltinSymbol::_TARG: {
                if (mMacroCallStack.empty() == true)
                {
                    mDiag.ReportError("'_TARG' outside of macro body.");
                    return {};
                }

                auto count = static_cast<PreprocessorInteger>(
                    mMacroCallStack.back().mAllArguments.size()
                );
                return PreprocessorValue { count };
            } break;
            case PreprocessorBuiltinSymbol::_LINE: {
                return PreprocessorValue { 
                    static_cast<PreprocessorInteger>(pLocation.mLine) };
            } break;
            case PreprocessorBuiltinSymbol::_FILE: {
                return PreprocessorValue { NormalizePath(pLocation.mPath).string() };
            } break;
            case PreprocessorBuiltinSymbol::_DATE: {
                return PreprocessorValue { GetDateString() };
            } break;
            case PreprocessorBuiltinSymbol::_TIME: {
                return PreprocessorValue { GetTimeString() };
            } break;
            case PreprocessorBuiltinSymbol::_VERSION: {
                return PreprocessorValue { std::to_string(kVersion) };
            } break;
            case PreprocessorBuiltinSymbol::_UNIQUE: {
                return PreprocessorValue { mUniqueCounter++ };
            } break;
            case PreprocessorBuiltinSymbol::_RANDOM32: {
                return PreprocessorValue { GetRandom32() };
            } break;
            case PreprocessorBuiltinSymbol::_RANDOM64: {
                return PreprocessorValue { GetRandom64() };
            } break;
            default:
                return {};
        }
    }
}

// Private Methods - Expressions - Interpolation *******************************

namespace G10::ASM
{
    static auto FormatInterpolation (const PreprocessorValue& pValue,
        const char pFormat) -> std::string
    {
        if (pFormat == 's' || pFormat == '\0')
        {
            if (const auto str = pValue.GetString())
                { return *str; }
            else if (const auto i = pValue.GetInteger())
                { return std::format("{}", *i); }
            else if (const auto fp = pValue.GetFixedPoint())
                { return std::format("{}", fp->Correct().GetComputed()); }
            else { return ""; }
        }

        PreprocessorInteger val { 0 };
        if (const auto str = pValue.GetString())
            { return *str; }
        else if (const auto i = pValue.GetInteger())
            { val = *i; }
        else if (const auto fp = pValue.GetFixedPoint())
            { val = fp->Correct().GetInteger(); }

        switch (pFormat)
        {
            case 'd': return std::format("{}", val);
            case 'x': return std::format("{:x}", val);
            case 'X': return std::format("{:X}", val);
            case 'o': return std::format("{:o}", val);
            case 'b': return std::format("{:b}", val);
            default: return std::to_string(val);  
        }
    }

    auto Preprocessor::InterpolateString (const SourceLocation& pLocation,
        const std::string& pText, std::size_t pDepth)
            -> std::optional<std::string>
    {
        if (pDepth > mLimitInterpolationDepth)
        {
            mDiag.ReportError(pLocation,
                "Maximum interpolation depth ({}) exceeded.", mLimitInterpolationDepth);
            return std::nullopt;
        }

        std::size_t index { 0 };
        std::string result { "" };
        while (index < pText.size())
        {
            // Braces escaped with '\' and any characters which are not '{'
            // should be emitted literally and skipped over.
            if (pText[index] == '\\' && index + 1 < pText.size())
            {
                if (pText[index + 1] == '{' || pText[index + 1] == '}')
                    { result += pText[index + 1]; index += 2; continue; }
            }
            else if (pText[index] != '{')
                { result += pText[index++]; continue; }

            // This is the start of an interpolation. Skip over the opening
            // brace.
            index++;

            // Interpolation strings may begin with an optional format specifier,
            // which consists of one character followed by a colon. If one is
            // not present, then the default specifier, 's', is used.
            char format = 's';
            if (index + 1 < pText.size() &&
                std::isalpha(pText[index]) &&
                pText[index + 1] == ':')
            {
                format = pText[index];
                index += 2;
            }

            // Collect the text of the interpolation until the closing '}'.
            std::size_t depthCount = 1;
            std::string interpolation = "";
            while (index < pText.size() && depthCount > 0)
            {
                if (pText[index] == '{')
                    { depthCount++; }
                else if (pText[index] == '}')
                {
                    depthCount--;
                    if (depthCount == 0)
                        { break; }
                }

                interpolation += pText[index++];
            }

            if (depthCount > 0)
            {
                mDiag.ReportError(pLocation,
                    "Encountered unterminated interpolation in string.");
                return std::nullopt;
            }

            // This is the end of the interpolation. Skip past the closing
            // brace.
            index++;

            // Lex, evaluate, then format the expression.
            Lexer lexer { mDiag };
            if (lexer.LexString(interpolation, false) == false)
            {
                mDiag.ReportError(pLocation,
                    "Error lexing interpolation in string.");
                return std::nullopt;
            }

            TokenCursor cursor { lexer.GetTokens() };
            auto value = EvaluateExpression(cursor);
            if (value.IsUndefined() == true)
            {
                mDiag.ReportError(pLocation,
                    "Error evaluating interpolation in string.");
                return std::nullopt;
            }

            std::string formatted = FormatInterpolation(value, format);
            
            // If our formatted string contains a brace, then there may be
            // further interpolations therein.
            if (formatted.find('{') != std::string::npos)
            {
                auto nested = InterpolateString(pLocation, formatted, pDepth + 1);
                if (nested.has_value() == false)
                    { return std::nullopt; }
                formatted = *nested;
            }

            result += formatted;
        }

        return result;
    }
    
    auto Preprocessor::InterpolateIdentifier (const SourceLocation& pLocation,
        const std::string& pText, std::size_t pDepth)
            -> std::optional<std::string>
    {
        if (pDepth > mLimitInterpolationDepth)
        {
            mDiag.ReportError(pLocation,
                "Maximum interpolation depth ({}) exceeded.", mLimitInterpolationDepth);
            return std::nullopt;
        }

        std::size_t index { 0 };
        std::string result { "" };
        while (index < pText.size())
        {
            // Same process as `InterpolateString`, only we don't check for
            // escaped braces.
            if (pText[index] != '{')
                { result += pText[index++]; continue; }
            index++;

            char format = 's';
            if (index + 1 < pText.size() &&
                std::isalpha(pText[index]) &&
                pText[index + 1] == ':')
            {
                format = pText[index];
                index += 2;
            }

            std::size_t depthCount = 1;
            std::string interpolation = "";
            while (index < pText.size() && depthCount > 0)
            {
                if (pText[index] == '{')
                    { depthCount++; }
                else if (pText[index] == '}')
                {
                    depthCount--;
                    if (depthCount == 0)
                        { break; }
                }

                interpolation += pText[index++];
            }

            if (depthCount > 0)
            {
                mDiag.ReportError(pLocation,
                    "Encountered unterminated interpolation in identifier.");
                return std::nullopt;
            }

            index++;

            Lexer lexer { mDiag };
            if (lexer.LexString(interpolation, false) == false)
            {
                mDiag.ReportError(pLocation,
                    "Error lexing interpolation in identifier.");
                return std::nullopt;
            }

            TokenCursor cursor { lexer.GetTokens() };
            auto value = EvaluateExpression(cursor);
            if (value.IsUndefined() == true)
            {
                mDiag.ReportError(pLocation,
                    "Error evaluating interpolation in identifier.");
                return std::nullopt;
            }

            std::string formatted = FormatInterpolation(value, format);
            if (formatted.find('{') != std::string::npos)
            {
                auto nested = InterpolateIdentifier(pLocation, formatted, pDepth + 1);
                if (nested.has_value() == false)
                    { return std::nullopt; }
                formatted = *nested;
            }
            result += formatted;
        }

        return result;
    }
}
