/**
 * @file    G10.ASM.Preprocessor.cpp
 * @brief   Contains implementations for the G10 Assembler's preprocessor
 *          class, and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Lexer.hpp>
#include <G10.ASM.Preprocessor.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::ASM
{
    Preprocessor::Preprocessor (Diagnostic& pDiag) :
        mDiag   { pDiag }
    {
        mCharmaps[""] = {};
    }
}

// Public Methods - Input & Output *********************************************

namespace G10::ASM
{
    auto Preprocessor::PreprocessInput (TokenSlice pSlice) -> bool
    {
        switch (Preprocess(pSlice))
        {
            case PreprocessStatus::OK:
                return true;
            case PreprocessStatus::Continue:
                mDiag.ReportError(pSlice.front().mLocation,
                    "Found '.continue' directive outside of loop context.");
                return false;
            case PreprocessStatus::Break:
                mDiag.ReportError(pSlice.front().mLocation,
                    "Found '.break' directive outside of loop context.");
                return false;
            case PreprocessStatus::Error:
            default:
                return false;
        }
    }

    auto Preprocessor::GetOutput () const -> const std::string&
    {
        return mOutput;
    }
}

// Public Methods - Options ****************************************************

namespace G10::ASM
{
    auto Preprocessor::SetInterpolationDepthLimit (std::size_t pInterpolationDepthLimit) -> void
    {
        mLimitInterpolationDepth = stx::clamp(pInterpolationDepthLimit,
            kMinimumInterpolationDepth, kMaximumInterpolationDepth);
    }

    auto Preprocessor::SetLoopDepthLimit (std::size_t pLoopDepthLimit) -> void
    {
        mLimitLoopDepth = stx::clamp(pLoopDepthLimit,
            kMinimumLoopDepth, kMaximumLoopDepth);
    }

    auto Preprocessor::SetIncludeDepthLimit (std::size_t pIncludeDepthLimit) -> void
    {
        mLimitIncludeDepth = stx::clamp(pIncludeDepthLimit,
            kMinimumIncludeDepth, kMaximumIncludeDepth);
    }

    auto Preprocessor::SetRecursionDepthLimit (std::size_t pRecursionDepthLimit) -> void
    {
        mLimitRecursionDepth = stx::clamp(pRecursionDepthLimit,
            kMinimumRecursionDepth, kMaximumRecursionDepth);
    }

    auto Preprocessor::SetIncludeDirectories (const std::vector<std::string>& pIncludeDirs) -> void
    {
        mIncludeDirs.clear();
        for (const auto& dir : pIncludeDirs)
        {
            mIncludeDirs.push_back(NormalizePath(dir));
        }
    }

    auto Preprocessor::SetDefines (const stx::dictionary<std::string>& pDefines) -> bool
    {
        for (const auto& [key, val] : pDefines)
        {
            if (val.empty() == true)
            {
                mSymbols[key] = PreprocessorSymbol {
                    .mValue = PreprocessorInteger { 1 },
                    .mIsConstant = true,
                    .mLocation = SourceLocation {
                        .mPath = std::format("<define '{}'>", key),
                        .mLine = 0,
                        .mColumn = 0
                    }
                };
            }
            else
            {
                Lexer lex { mDiag };
                if (lex.LexString(val, false) == false)
                {
                    mDiag.ReportError("Error lexing value of define '{}'", key);
                    return false;
                }

                TokenCursor cursor { lex.GetTokens() };
                auto val = EvaluateExpression(cursor);
                if (val.IsUndefined() == true)
                {
                    mDiag.ReportError("Error evaluating value of define '{}'", key);
                    return false;
                }

                mSymbols[key] = PreprocessorSymbol {
                    .mValue = val,
                    .mIsConstant = true,
                    .mLocation = SourceLocation {
                        .mPath = std::format("<define '{}'>", key),
                        .mLine = 0,
                        .mColumn = 0
                    }
                };
            }
        }

        return true;
    }
}

// Private Methods - Preprocess ************************************************

namespace G10::ASM
{
    auto Preprocessor::Preprocess (TokenSlice pSlice) -> PreprocessStatus
    {
        if (pSlice.empty() == true)
            { return PreprocessStatus::OK; }

        TokenCursor cursor { pSlice };
        while (cursor.IsAtEnd() == false)
        {
            const auto& token = cursor.GetNextToken();
            switch (token.mType)
            {
                case TokenType::EndOfFile: {
                    cursor.Skip();
                } break;
                case TokenType::NewLine: {
                    EmitNewline();
                    cursor.Skip();
                } break;
                case TokenType::Identifier: {
                    if (DispatchIdentifier(cursor, token) == false)
                        { return AcknowledgeStatus(); }
                } break;
                case TokenType::Parameter: {
                    if (DispatchParameter(cursor, token) == false)
                        { return AcknowledgeStatus(); }
                } break;
                case TokenType::Keyword: {
                    if (DispatchKeyword(cursor, token) == false)
                        { return AcknowledgeStatus(); }
                } break;
                default: {
                    if (DispatchPassthrough(cursor, token) == false)
                        { return AcknowledgeStatus(); }
                } break;
            }

            if (mPendingStatus != PreprocessStatus::OK)
                { break; }
        }

        return mPendingStatus;
    }
}

// Private Methods - Helpers ***************************************************

namespace G10::ASM
{
    auto Preprocessor::AcknowledgeStatus () -> PreprocessStatus
    {
        if (mPendingStatus != PreprocessStatus::OK)
        {
            return mPendingStatus;
        }

        return PreprocessStatus::Error;
    }

    auto Preprocessor::ResetStatus () -> void
        { mPendingStatus = PreprocessStatus::OK; }

    auto Preprocessor::DeepSlice (std::span<Token> pTokens) -> std::vector<Token>
    {
        std::vector<Token> tokens(std::from_range, pTokens);
        for (std::size_t i = 0; i < tokens.size(); ++i)
        {
            const auto& token = tokens[i];
            if (token.mType != TokenType::Identifier)
                { continue; }

            auto findIt = mSnippets.find(token.Stringify().value_or(""));
            if (findIt != mSnippets.end())
            {
                for (const auto& inner : findIt->second.mBody)
                {
                    auto findIt = mSnippets.find(inner.Stringify().value_or(""));
                    if (findIt != mSnippets.end())
                    {
                        mDiag.ReportError(inner.mLocation,
                            "Attempted expansion of snippet '{}' inside of snippet '{}'.",
                            inner.Stringify().value_or(""),
                            token.Stringify().value_or(""));
                        mPendingStatus = PreprocessStatus::Error;
                        return {};
                    }
                }

                tokens.erase(tokens.begin() + i);
                tokens.insert_range(tokens.begin() + i, findIt->second.mBody);
            }
        }

        return tokens;
    }

    auto Preprocessor::DeepSlice (std::span<const Token> pTokens) -> std::vector<Token>
    {
        std::vector<Token> tokens(std::from_range, pTokens);
        for (std::size_t i = 0; i < tokens.size(); ++i)
        {
            const auto& token = tokens[i];
            if (token.mType != TokenType::Identifier)
                { continue; }

            auto findIt = mSnippets.find(token.Stringify().value_or(""));
            if (findIt != mSnippets.end())
            {
                for (const auto& inner : findIt->second.mBody)
                {
                    auto findIt = mSnippets.find(inner.Stringify().value_or(""));
                    if (findIt != mSnippets.end())
                    {
                        mDiag.ReportError(inner.mLocation,
                            "Attempted expansion of snippet '{}' inside of snippet '{}'.",
                            inner.Stringify().value_or(""),
                            token.Stringify().value_or(""));
                        mPendingStatus = PreprocessStatus::Error;
                        return {};
                    }
                }

                tokens.erase(tokens.begin() + i);
                tokens.insert_range(tokens.begin() + i, findIt->second.mBody);
            }
        }

        return tokens;
    }
}
