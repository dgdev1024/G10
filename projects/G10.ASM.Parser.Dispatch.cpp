/**
 * @file    G10.ASM.Parser.Dispatch.cpp
 * @brief   Contains implementations for the G10 Assembler Parser's dispatch
 *          methods, and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Parser.hpp>

// Private Methods - Dispatch **************************************************

namespace G10::ASM
{
    auto Parser::DispatchKeyword (const Keyword& pKeyword,
        const SourceLocation& pLocation, TokenCursor& pCursor) -> bool
    {
        if (pKeyword.Is<AssemblerDirective>() == true)
        {
            return DispatchDirective(pKeyword, pLocation, pCursor);
        }
        else if (pKeyword.Is<CPU::InstructionType>() == true)
        {
            return DispatchInstructionStatement(pKeyword, pLocation, pCursor);
        }

        mDiag.ReportError(pLocation,
            "Unexpected keyword '{}'", pKeyword.StringifyType());
        return false;
    }

    auto Parser::DispatchDirective (const Keyword& pKeyword,
        const SourceLocation& pLocation, TokenCursor& pCursor) -> bool
    {
        switch (pKeyword.GetType<AssemblerDirective>())
        {
            case AssemblerDirective::BYTE:      return DispatchByteDirective(pLocation, pCursor);
            case AssemblerDirective::WORD:      return DispatchWordDirective(pLocation, pCursor);
            case AssemblerDirective::DWORD:     return DispatchDoubleWordDirective(pLocation, pCursor);
            case AssemblerDirective::ASCIZ:     return DispatchStringDirective(pLocation, pCursor);
            case AssemblerDirective::SPACE:     return DispatchSpaceDirective(pLocation, pCursor);
            case AssemblerDirective::EXPORT:    return DispatchExportDirective(pLocation, pCursor);
            case AssemblerDirective::IMPORT:    return DispatchImportDirective(pLocation, pCursor);
            case AssemblerDirective::SECTION:   return DispatchSectionDirective(pLocation, pCursor);
            case AssemblerDirective::ORG:       return DispatchOrgDirective(pLocation, pCursor);
            case AssemblerDirective::ALIGN:     return DispatchAlignDirective(pLocation, pCursor);
            default:
                mDiag.ReportError(pLocation,
                    "Un-implemented assembler directive '{}'", pKeyword.StringifyType());
                return false;
        }
    }
}

// Private Methods - Dispatch - Directives *************************************

namespace G10::ASM
{
    auto Parser::DispatchByteDirective (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> bool
    {
        auto node = std::make_shared<ByteDirectiveNode>();
        auto slice = pCursor.CollectLine();
        TokenCursor cursor { slice };
        while (cursor.IsAtEnd() == false)
        {
            const auto& token = pCursor.GetNextToken();
            const auto expr = EvaluateOperandExpression(pLocation, cursor);
            if (!expr)
            {
                mDiag.ReportError(pLocation,
                    "Unexpected '{}' token in '.BYTE' directive.",
                    token.StringifyType());
                return false;
            }

            if (
                std::dynamic_pointer_cast<IntegerExpressionNode>(expr) == nullptr &&
                std::dynamic_pointer_cast<StringExpressionNode>(expr) == nullptr
            )
            {
                mDiag.ReportError(pLocation,
                    "Expected integer or string in '.BYTE' directive.",
                    token.StringifyType());
                return false;
            }

            node->mOperands.push_back(expr);
            if (!cursor.ExpectNextToken(TokenType::Comma))
                { break; }
        }

        node->mLocation = pLocation;
        mOutput.mNodes.push_back(node);
        return true;
    }

    auto Parser::DispatchWordDirective (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> bool
    {
        auto node = std::make_shared<WordDirectiveNode>();
        auto slice = pCursor.CollectLine();
        TokenCursor cursor { slice };
        while (cursor.IsAtEnd() == false)
        {
            const auto& token = pCursor.GetNextToken();
            const auto expr = EvaluateOperandExpression(pLocation, cursor);
            if (!expr)
            {
                mDiag.ReportError(pLocation,
                    "Unexpected '{}' token in '.WORD' directive.",
                    token.StringifyType());
                return false;
            }

            if (std::dynamic_pointer_cast<IntegerExpressionNode>(expr) == nullptr)
            {
                mDiag.ReportError(pLocation,
                    "Expected integer in '.WORD' directive.",
                    token.StringifyType());
                return false;
            }

            node->mOperands.push_back(expr);
            if (!cursor.ExpectNextToken(TokenType::Comma))
                { break; }
        }

        node->mLocation = pLocation;
        mOutput.mNodes.push_back(node);
        return true;
    }

    auto Parser::DispatchDoubleWordDirective (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> bool
    {
        auto node = std::make_shared<DoubleWordDirectiveNode>();
        auto slice = pCursor.CollectLine();
        TokenCursor cursor { slice };
        while (cursor.IsAtEnd() == false)
        {
            const auto& token = pCursor.GetNextToken();
            const auto expr = EvaluateOperandExpression(pLocation, cursor);
            if (!expr)
            {
                mDiag.ReportError(pLocation,
                    "Unexpected '{}' token in '.DOUBLEWORD' directive.",
                    token.StringifyType());
                return false;
            }

            if (
                std::dynamic_pointer_cast<IntegerExpressionNode>(expr) == nullptr &&
                std::dynamic_pointer_cast<LabelExpressionNode>(expr) == nullptr
            )
            {
                mDiag.ReportError(pLocation,
                    "Expected integer or address label in '.DOUBLEWORD' directive.",
                    token.StringifyType());
                return false;
            }

            node->mOperands.push_back(expr);
            if (!cursor.ExpectNextToken(TokenType::Comma))
                { break; }
        }

        node->mLocation = pLocation;
        mOutput.mNodes.push_back(node);
        return true;
    }

    auto Parser::DispatchStringDirective (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> bool
    {
        auto node = std::make_shared<StringDirectiveNode>();
        auto slice = pCursor.CollectLine();
        TokenCursor cursor { slice };
        while (cursor.IsAtEnd() == false)
        {
            const auto& token = pCursor.GetNextToken();
            const auto expr = EvaluateOperandExpression(pLocation, cursor);
            if (!expr)
            {
                mDiag.ReportError(pLocation,
                    "Unexpected '{}' token in '.STRING' directive.",
                    token.StringifyType());
                return false;
            }

            if (std::dynamic_pointer_cast<StringExpressionNode>(expr) == nullptr)
            {
                mDiag.ReportError(pLocation,
                    "Expected string in '.STRING' directive.",
                    token.StringifyType());
                return false;
            }

            node->mOperands.push_back(expr);
            if (!cursor.ExpectNextToken(TokenType::Comma))
                { break; }
        }

        node->mLocation = pLocation;
        mOutput.mNodes.push_back(node);
        return true;
    }

    auto Parser::DispatchSpaceDirective (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> bool
    {
        auto node = std::make_shared<SpaceDirectiveNode>();
        auto slice = pCursor.CollectLine();
        TokenCursor cursor { slice };
        while (cursor.IsAtEnd() == false)
        {
            const auto& token = pCursor.GetNextToken();
            const auto expr = EvaluateOperandExpression(pLocation, cursor);
            if (!expr)
            {
                mDiag.ReportError(pLocation,
                    "Unexpected '{}' token in '.SPACE' directive.",
                    token.StringifyType());
                return false;
            }

            if (std::dynamic_pointer_cast<IntegerExpressionNode>(expr) == nullptr)
            {
                mDiag.ReportError(pLocation,
                    "Expected integer in '.SPACE' directive.",
                    token.StringifyType());
                return false;
            }

            node->mOperands.push_back(expr);
            if (!cursor.ExpectNextToken(TokenType::Comma))
                { break; }
        }

        node->mLocation = pLocation;
        mOutput.mNodes.push_back(node);
        return true;
    }

    auto Parser::DispatchIncbinDirective (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> bool
    {
        auto node = std::make_shared<IncbinDirectiveNode>();
        auto slice = pCursor.CollectLine();
        TokenCursor cursor { slice };

        auto filename = EvaluateStringExpression(pLocation, cursor);
        if (!filename)
        {
            mDiag.ReportError(pLocation,
                "Expected filename string in '.INCBIN' directive.");
            return false;
        }

        std::shared_ptr<IntegerExpressionNode> offset {};
        std::shared_ptr<IntegerExpressionNode> size {};
        if (cursor.ExpectNextToken(TokenType::Comma))
        {
            offset = EvaluateIntegerExpression(pLocation, cursor);
            if (!offset)
            {
                mDiag.ReportError(pLocation,
                    "Expected integer offset in '.INCBIN' directive.");
                return false;
            }

            if (cursor.ExpectNextToken(TokenType::Comma))
            {
                size = EvaluateIntegerExpression(pLocation, cursor);
                if (!size)
                {
                    mDiag.ReportError(pLocation,
                        "Expected integer size in '.INCBIN' directive.");
                    return false;
                }
            }
        }

        node->mFilename = filename;
        node->mOffset = offset;
        
        node->mLocation = pLocation;node->mSize = size;
        mOutput.mNodes.push_back(node);
        return true;
    }

    auto Parser::DispatchExportDirective (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> bool
    {
        auto node = std::make_shared<ExportDirectiveNode>();
        auto slice = pCursor.CollectLine();
        TokenCursor cursor { slice };
        while (cursor.IsAtEnd() == false)
        {
            const auto& token = pCursor.GetNextToken();
            const auto expr = EvaluateLabelExpression(pLocation, cursor);
            if (!expr)
            {
                mDiag.ReportError(pLocation,
                    "Unexpected '{}' token in '.EXPORT' directive.",
                    token.StringifyType());
                return false;
            }

            node->mLabels.push_back(expr);
            if (!cursor.ExpectNextToken(TokenType::Comma))
                { break; }
        }

        node->mLocation = pLocation;
        mOutput.mNodes.push_back(node);
        return true;
    }

    auto Parser::DispatchImportDirective (const SourceLocation& pLocation,
        TokenCursor& pCursor) -> bool
    {
        auto node = std::make_shared<ImportDirectiveNode>();
        auto slice = pCursor.CollectLine();
        TokenCursor cursor { slice };
        while (cursor.IsAtEnd() == false)
        {
            const auto& token = pCursor.GetNextToken();
            const auto expr = EvaluateLabelExpression(pLocation, cursor);
            if (!expr)
            {
                mDiag.ReportError(pLocation,
                    "Unexpected '{}' token in '.IMPORT' directive.",
                    token.StringifyType());
                return false;
            }

            node->mLabels.push_back(expr);
            if (!cursor.ExpectNextToken(TokenType::Comma))
                { break; }
        }

        node->mLocation = pLocation;
        mOutput.mNodes.push_back(node);
        return true;
    }

    auto Parser::DispatchSectionDirective (const SourceLocation& pLocation, 
        TokenCursor& pCursor) -> bool
    {
        TokenSlice slice = pCursor.CollectLine();
        TokenCursor cursor { slice };

        auto displayNameExpr = EvaluateStringExpression(pLocation, cursor);
        if (!displayNameExpr)
        {
            mDiag.ReportError(pLocation,
                "Expected display name string in '.SECTION' directive.");
            return false;
        }
        
        if (!cursor.ExpectNextToken(TokenType::Comma))
        {
            mDiag.ReportError(pLocation,
                "Expected ',' after display name in '.SECTION' directive.");
            return false;
        }

        auto sectionNameExpr = EvaluateSectionNameExpression(pLocation, cursor);
        if (!sectionNameExpr)
        {
            mDiag.ReportError(pLocation,
                "Expected section name in '.SECTION' directive.");
            return false;
        }

        std::shared_ptr<ExpressionNode> fixedAddressExpr {};
        if (cursor.IsAtEnd() == false)
        {
            fixedAddressExpr = EvaluatePointerExpression(pLocation, cursor);
            if (!fixedAddressExpr)
            {
                mDiag.ReportError(pLocation,
                    "Expected fixed address in '.SECTION' directive.");
                return false;
            }
        }

        auto node = std::make_shared<SectionDirectiveNode>();
        node->mDisplayName = displayNameExpr;
        node->mSectionName = sectionNameExpr;
        
        node->mLocation = pLocation;node->mFixedAddress = fixedAddressExpr;
        mOutput.mNodes.push_back(node);
        return true;
    }

    auto Parser::DispatchOrgDirective (const SourceLocation& pLocation, 
        TokenCursor& pCursor) -> bool
    {
        auto expr = EvaluateIntegerExpression(pLocation, pCursor);
        if (!expr)
        {
            mDiag.ReportError(pLocation,
                "Expected integer expression in '.ORG' directive.");
            return false;
        }

        auto node = std::make_shared<OrgDirectiveNode>();
        
        node->mLocation = pLocation;node->mAddress = expr;
        mOutput.mNodes.push_back(node);
        return true;
    }

    auto Parser::DispatchAlignDirective (const SourceLocation& pLocation, 
        TokenCursor& pCursor) -> bool
    {
        auto expr = EvaluateIntegerExpression(pLocation, pCursor);
        if (!expr)
        {
            mDiag.ReportError(pLocation,
                "Expected integer expression in '.ALIGN' directive.");
            return false;
        }

        auto node = std::make_shared<AlignDirectiveNode>();
        
        node->mLocation = pLocation;node->mBoundary = expr;
        mOutput.mNodes.push_back(node);
        return true;
    }
}

// Private Methods - Dispatch - Statements *************************************

namespace G10::ASM
{
    auto Parser::DispatchLabelStatement (const SourceLocation& pLocation, 
        TokenCursor& pCursor) -> bool
    {
        auto expr = EvaluateLabelExpression(pLocation, pCursor);
        if (expr == nullptr)
        {
            mDiag.ReportError(pLocation,
                "Expected label expression in label statement.");
            return false;
        }

        if (!pCursor.ExpectNextToken(TokenType::Colon))
        {
            mDiag.ReportError(pLocation,
                "Expected ':' after label expression in label statement.");
            return false;
        }

        auto node = std::make_shared<LabelStatementNode>();
        
        node->mLocation = pLocation;node->mLabelExpr = *expr;
        mOutput.mNodes.push_back(node);
        return true;
    }

    auto Parser::DispatchInstructionStatement (const Keyword& pKeyword,
        const SourceLocation& pLocation, TokenCursor& pCursor) -> bool
    {
        TokenSlice slice = pCursor.CollectLine();
        TokenCursor cursor { slice };
        auto node = std::make_shared<InstructionStatementNode>();
        node->mType = pKeyword.GetType<CPU::InstructionType>();

        while (cursor.IsAtEnd() == false)
        {
            auto operand = EvaluateOperandExpression(pLocation, cursor);
            if (!operand)
            {
                mDiag.ReportError(pLocation,
                    "Expected operand in instruction statement.");
                return false;
            }

            node->mOperands.push_back(operand);
            if (!cursor.ExpectNextToken(TokenType::Comma))
                { break; }
        }

        if (node->mOperands.size() > 2)
        {
            mDiag.ReportError(pLocation,
                "Too many operands in instruction statement.");
            return false;
        }

        node->mLocation = pLocation;
        mOutput.mNodes.push_back(node);
        return true;
    }
}
