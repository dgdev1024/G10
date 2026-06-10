/**
 * @file    G10.ASM.Preprocessor.cpp
 * @brief   Contains implementations for the G10 Assembler's preprocessor
 *          class, and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Preprocessor.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::ASM
{
    Preprocessor::Preprocessor (Diagnostic& pDiag) :
        mDiag   { pDiag }
    {}
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
}
