/**
 * @file    G10.ASM.Preprocessor.Dispatch.cpp
 * @brief   Contains implementations for the G10 Assembler Preprocessor's
 *          dispatch methods, and related definitions.
 */

// Includes ********************************************************************

#include <print>
#include <G10.ASM.Lexer.hpp>
#include <G10.ASM.Preprocessor.hpp>

// Private Methods - Dispatch **************************************************

namespace G10::ASM
{   
    auto Preprocessor::DispatchKeyword (TokenCursor& pCursor, 
        const Token& pToken) -> bool
    {
        if (const auto kw = pToken.GetKeyword())
        {
            switch (kw->GetGroup())
            {
                case KeywordGroup::PreprocessorDirective:
                    pCursor.Skip();
                    return DispatchDirective(pCursor, *kw, pToken.mLocation);
                default: break;
            }
        }

        return DispatchPassthrough(pCursor, pToken);
    }

    auto Preprocessor::DispatchDirective (TokenCursor& pCursor, 
        const Keyword& pKeyword, const SourceLocation& pLocation) -> bool
    {
        if (const auto type = pKeyword.GetTypeIf<PreprocessorDirective>())
        {
            switch (*type)
            {
                case PreprocessorDirective::PRINT:      return DispatchPrint(pCursor, pLocation, false);
                case PreprocessorDirective::PRINTLN:    return DispatchPrint(pCursor, pLocation, true);
                case PreprocessorDirective::INFO:       return DispatchInfo(pCursor, pLocation);
                case PreprocessorDirective::WARNING:    return DispatchWarning(pCursor, pLocation);
                case PreprocessorDirective::ERROR:      return DispatchError(pCursor, pLocation);
                case PreprocessorDirective::ASSERT:     return DispatchAssert(pCursor, pLocation);
                case PreprocessorDirective::LET:        return DispatchLet(pCursor, pLocation);
                case PreprocessorDirective::CONST:      return DispatchConst(pCursor, pLocation);
                case PreprocessorDirective::IF:         return DispatchIf(pCursor, pLocation);
                case PreprocessorDirective::REPEAT:     return DispatchRepeat(pCursor, pLocation);
                case PreprocessorDirective::WHILE:      return DispatchWhile(pCursor, pLocation);
                case PreprocessorDirective::FOR:        return DispatchFor(pCursor, pLocation);
                case PreprocessorDirective::CONTINUE:   return DispatchContinue(pCursor, pLocation);
                case PreprocessorDirective::BREAK:      return DispatchBreak(pCursor, pLocation);
                case PreprocessorDirective::MACRO:      return DispatchMacro(pCursor, pLocation);
                case PreprocessorDirective::SHIFT:      return DispatchShift(pCursor, pLocation);
                case PreprocessorDirective::RETURN:     return DispatchReturn(pCursor, pLocation);
                case PreprocessorDirective::INCLUDE:    return DispatchInclude(pCursor, pLocation);
                case PreprocessorDirective::ONCE:       return DispatchOnce(pCursor, pLocation);
                case PreprocessorDirective::ELSEIF:
                case PreprocessorDirective::ELSE:
                case PreprocessorDirective::ENDIF:
                    mDiag.ReportError(pLocation, "'{}' without matching '.IF'.",
                        pKeyword.StringifyType());
                    return false;
                case PreprocessorDirective::ENDREPEAT:
                    mDiag.ReportError(pLocation, "'.ENDREPEAT' without matching '.REPEAT'.");
                    return false;
                case PreprocessorDirective::ENDWHILE:
                    mDiag.ReportError(pLocation, "'.ENDWHILE' without matching '.WHILE'.");
                    return false;
                case PreprocessorDirective::ENDFOR:
                    mDiag.ReportError(pLocation, "'.ENDFOR' without matching '.FOR'.");
                    return false;
                case PreprocessorDirective::ENDMACRO:
                    mDiag.ReportError(pLocation, "'.ENDMACRO' without matching '.MACRO'.");
                    return false;
            }
        }

        return false;
    }

    auto Preprocessor::DispatchIdentifier (TokenCursor& pCursor, 
        const Token& pToken) -> bool
    {
        auto val = CollectAndEvaluate(pCursor);
        if (val.IsUndefined() == false)
        {
            EmitValue(val);
            return true;
        }

        pCursor.Skip();
        auto lexeme = InterpolateIdentifier(pToken.mLocation,
            pToken.Stringify().value_or(""));
        if (lexeme.has_value() == false)
            { return false; }

        if (auto macroFindIt = mMacros.find(*lexeme);
            macroFindIt != mMacros.end())
        {
            return DispatchMacroCall(pCursor, pToken.mLocation,
                macroFindIt->second);
        }

        // auto val = EvaluateSymbol(*lexeme);
        // if (val.IsUndefined() == false)
        // {
        //     pCursor.Unskip();


        //     // EmitValue(val);
        //     // return true;
        // }

        EmitText(*lexeme + " ");
        return true;
    }

    auto Preprocessor::DispatchParameter (TokenCursor& pCursor, 
        const Token& pToken) -> bool
    {
        if (mMacroCallStack.empty() == true)
        {
            mDiag.ReportError(pToken.mLocation,
                "Parameter token outside of macro body.");
            return false;
        }

        auto& call = mMacroCallStack.back();
        const auto& macro = *call.mMacro;
        const auto& lexeme = pToken.Stringify().value_or("");

        PreprocessorMacroArgument arg {};
        if (pToken.HasInteger() == true)
        {
            auto integer = pToken.mInteger.value_or(0);
            if (integer == 0)
            {
                PreprocessorValue val { macro.mName };
                EmitValue(val);
                return true;
            }
            else if (integer > call.mArguments.size())
            {
                mDiag.ReportError(pToken.mLocation,
                    "Macro argument index #{} is out of range.", integer);
                return false;
            }

            arg = call.mArguments[integer - 1];
        }
        else if (lexeme == "?")
        {
            PreprocessorValue val { macro.mInvocationCount };
            EmitValue(val);
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
                mDiag.ReportError(pToken.mLocation,
                    "Named argument '{}' not found in macro '{}'.",
                    lexeme, macro.mName);
                return false;
            }
            else if (index >= call.mArguments.size())
            {
                mDiag.ReportError(pToken.mLocation,
                    "Named argument '{}' in macro '{}' resolves to an argument "
                    "index which is out of range.", lexeme, macro.mName);
                return false;
            }

            arg = call.mArguments[index];
        }

        if (std::holds_alternative<PreprocessorValue>(arg))
        {
            return DispatchPassthrough(pCursor, pToken);
        }
        else if (std::holds_alternative<TokenSlice>(arg))
        {
            pCursor.Skip();
            TokenCursor cursor { *std::get_if<TokenSlice>(&arg) };
            while (cursor.IsAtEnd() == false)
            {
                const auto& tk = cursor.GetNextToken();
                DispatchPassthrough(cursor, tk);
            }
        }
        else { return false; }
        
        return true;
    }

    auto Preprocessor::DispatchPassthrough (TokenCursor& pCursor, 
        const Token& pToken) -> bool
    {
        if (pToken.mType == TokenType::StringLiteral)
        {
            pCursor.Skip();
            auto lexeme = InterpolateString(pToken.mLocation,
                pToken.Stringify().value_or(""));
            if (lexeme.has_value() == false)
                { return false; }

            EmitText(std::format("\"{}\" ", *lexeme));
            return true;
        }
        else if (pToken.CanStartExpression() == true)
        {
            mPassthroughExpr = true;
            auto exprValue = CollectAndEvaluate(pCursor);
            mPassthroughExpr = false;

            if (exprValue.IsUndefined() == false)
            {
                EmitValue(exprValue);
                return true;
            }
        }

        pCursor.Skip();
        EmitToken(pToken);
        return true;
    }
}

// Private Methods - Dispatch - Print & Debug **********************************

namespace G10::ASM
{
    auto Preprocessor::DispatchPrint (TokenCursor& pCursor, 
        const SourceLocation& pLocation, bool pNewline) -> bool
    {
        auto exprSlice = pCursor.CollectExpression();
        if (exprSlice.empty() == true)
        {
            mDiag.ReportError(pLocation,
                "Expected expression after '{}' directive.",
                (pNewline == true) ? ".PRINTLN" : ".PRINT");
            return false;
        }

        TokenCursor exprCursor { exprSlice };
        auto exprVal = EvaluateExpression(exprCursor);
        if (exprVal.IsString() == false)
        {
            mDiag.ReportError(pLocation,
                "Expected string after '{}' directive.",
                (pNewline == true) ? ".PRINTLN" : ".PRINT");
            return false;
        }

        if (pNewline == true)
            { std::println("{}", *exprVal.GetString()); }
        else
            { std::print("{}", *exprVal.GetString()); }

        return true;
    }

    auto Preprocessor::DispatchInfo (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        auto exprSlice = pCursor.CollectExpression();
        if (exprSlice.empty() == true)
        {
            mDiag.ReportError(pLocation,
                "Expected expression after '.INFO' directive.");
            return false;
        }

        TokenCursor exprCursor { exprSlice };
        auto exprVal = EvaluateExpression(exprCursor);
        if (exprVal.IsString() == false)
        {
            mDiag.ReportError(pLocation,
                "Expected string after '.INFO' directive.");
            return false;
        }

        mDiag.ReportInfo("{}", *exprVal.GetString());
        return true;
    }

    auto Preprocessor::DispatchWarning (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        auto exprSlice = pCursor.CollectExpression();
        if (exprSlice.empty() == true)
        {
            mDiag.ReportError(pLocation,
                "Expected expression after '.WARNING' directive.");
            return false;
        }

        TokenCursor exprCursor { exprSlice };
        auto exprVal = EvaluateExpression(exprCursor);
        if (exprVal.IsString() == false)
        {
            mDiag.ReportError(pLocation,
                "Expected string after '.WARNING' directive.");
            return false;
        }

        if (mDiag.WarningsAreErrors() == true)
        {
            mDiag.ReportError("{}", *exprVal.GetString());
            return false;
        }
        else
        {
            mDiag.ReportWarning("{}", *exprVal.GetString());
        }

        return true;
    }

    auto Preprocessor::DispatchError (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        auto exprSlice = pCursor.CollectExpression();
        if (exprSlice.empty() == true)
        {
            mDiag.ReportError(pLocation,
                "Expected expression after '.ERROR' directive.");
            return false;
        }

        TokenCursor exprCursor { exprSlice };
        auto exprVal = EvaluateExpression(exprCursor);
        if (exprVal.IsString() == false)
        {
            mDiag.ReportError(pLocation,
                "Expected string after '.ERROR' directive.");
            return false;
        }

        mDiag.ReportError("{}", *exprVal.GetString());
        return false;
    }

    auto Preprocessor::DispatchAssert (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        auto exprSlice = pCursor.CollectExpression();
        if (exprSlice.empty() == true)
        {
            mDiag.ReportError(pLocation,
                "Expected expression after '.ASSERT' directive.");
            return false;
        }

        TokenSlice messageSlice {};
        PreprocessorString message { "Assertion Failure." };
        if (pCursor.ExpectNextToken(TokenType::Comma).has_value() == true)
        {
            messageSlice = pCursor.CollectExpression();
            TokenCursor messageCursor { messageSlice };
            auto messageVal = EvaluateExpression(messageCursor);
            if (messageVal.IsString() == false)
            {
                mDiag.ReportError(pLocation,
                    "Expected string after expression in '.ASSERT'. directive.");
                return false;
            }

            message = std::format("Assertion Failure: '{}'.",
                *messageVal.GetString());
        }

        TokenCursor exprCursor { exprSlice };
        auto exprVal = EvaluateExpression(exprCursor);
        if (exprVal.IsTruthy() == false)
        {
            mDiag.ReportError(pLocation, "{}", message);
            mPendingStatus = PreprocessStatus::Error;
            return false;
        }

        return true;
    }
}

// Private Methods - Dispatch - Symbols ****************************************

namespace G10::ASM
{
    auto Preprocessor::DispatchLet (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        // Capture the constant's name token. Make sure it's an identifier, and
        // interpolate it.
        const auto& nameToken = pCursor.GetNextToken(true);
        if (nameToken.mType != TokenType::Identifier)
        {
            mDiag.ReportError(pLocation,
                "Expected an identifier after '.LET' directive.");
            return false;
        }

        auto nameLexeme = InterpolateIdentifier(pLocation,
            nameToken.Stringify().value_or(""));
        if (nameLexeme.has_value() == false)
            { return false; }

        // Check the symbols map. Ensure that we are not attempting to overwrite
        // another constant.
        auto findIt = mSymbols.find(*nameLexeme);
        if (findIt != mSymbols.end() &&
            findIt->second.mIsConstant == true)
        {
            mDiag.ReportError(pLocation,
                "Attempted re-definition of constant symbol '{}'.",
                *nameLexeme);
            return false;
        }

        // Repeat this for the macros map.
        auto macroFindIt = mMacros.find(*nameLexeme);
        if (macroFindIt != mMacros.end())
        {
            mDiag.ReportError(pLocation,
                "Symbol '{}' is defined as a macro.",
                *nameLexeme);
            return false;
        }

        // If the next token is a control token (new line, end of file), then
        // map a blank symbol to this variable.
        if (pCursor.GetNextToken().GetGroup() == TokenGroup::ControlToken)
        {
            mSymbols[*nameLexeme] = PreprocessorSymbol {
                .mValue             = PreprocessorInteger { 0 },
                .mIsConstant        = false,
                .mLocation          = pLocation
            };
            return true;
        }

        auto opToken = pCursor.ExpectNextToken(TokenGroup::AssignmentOperator);
        if (opToken.has_value() == false)
        {  
            mDiag.ReportError(pLocation,
                "Expected assignment operator after name in '.LET' directive.");
            return false;
        }
        else if (opToken->mType == TokenType::AssignEqual)
        {
            auto right = EvaluateExpression(pCursor);
            if (right.IsUndefined() == true) 
                { return false; }

            mSymbols[*nameLexeme] = PreprocessorSymbol {
                .mValue             = right,
                .mIsConstant        = false,
                .mLocation          = pLocation
            };
        }
        else
        {
            if (findIt == mSymbols.end())
            {
                mDiag.ReportError(pLocation,
                    "Cannot perform compound assignment on undefined symbol '{}'.",
                    *nameLexeme);
                return false;
            }

            auto left = findIt->second.mValue;
            auto right = EvaluateExpression(pCursor);
            if (right.IsUndefined() == true) 
                { return false; }
            auto result = ApplyCompound(left, right, *opToken);
            if (result.IsUndefined() == true) 
                { return false; }

            mSymbols[*nameLexeme] = PreprocessorSymbol {
                .mValue             = result,
                .mIsConstant        = false,
                .mLocation          = pLocation
            };
        }

        return true;
    }

    auto Preprocessor::DispatchConst (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        // Capture the constant's name token. Make sure it's an identifier, and
        // interpolate it.
        const auto& nameToken = pCursor.GetNextToken(true);
        if (nameToken.mType != TokenType::Identifier)
        {
            mDiag.ReportError(pLocation,
                "Expected an identifier after '.CONST' directive.");
            return false;
        }

        auto nameLexeme = InterpolateIdentifier(pLocation,
            nameToken.Stringify().value_or(""));
        if (nameLexeme.has_value() == false)
            { return false; }

        // Check the symbols map. Ensure that we are not attempting to overwrite
        // another constant.
        auto findIt = mSymbols.find(*nameLexeme);
        if (findIt != mSymbols.end() &&
            findIt->second.mIsConstant == true)
        {
            mDiag.ReportError(pLocation,
                "Attempted re-definition of constant symbol '{}'.",
                *nameLexeme);
            return false;
        }

        // Repeat this for the macros map.
        auto macroFindIt = mMacros.find(*nameLexeme);
        if (macroFindIt != mMacros.end())
        {
            mDiag.ReportError(pLocation,
                "Symbol '{}' is defined as a macro.",
                *nameLexeme);
            return false;
        }

        // The `.const` directive allows an optional assign-equals token between
        // the name and value. Skip it if we find it here.
        pCursor.ExpectNextToken(TokenType::AssignEqual);

        // Collect and evaluate the expression ahead. Store its value.
        auto exprSlice = pCursor.CollectExpression();
        if (exprSlice.empty() == true)
        {
            mDiag.ReportError(pLocation,
                "Expected value after name in '.CONST' directive.");
            return false;
        }

        TokenCursor exprCursor { exprSlice };
        auto exprValue = EvaluateExpression(exprCursor);
        if (exprValue.IsUndefined() == true)
        {
            mDiag.ReportError(pLocation,
                "Expected value after name in '.CONST' directive.");
            return false;
        }

        mSymbols[*nameLexeme] = PreprocessorSymbol {
            .mValue         = exprValue,
            .mIsConstant    = true,
            .mLocation      = pLocation
        };
        return true;
    }
}

// Private Methods - Dispatch - Conditionals ***********************************

namespace G10::ASM
{
    auto Preprocessor::DispatchIf (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        // A vector to store our conditional's branches.
        std::vector<PreprocessorConditional> branches {};

        // Expect an expression after the `.IF`.
        auto exprSlice = pCursor.CollectExpression();
        if (exprSlice.empty() == true)
        {
            mDiag.ReportError(pLocation,
                "Expected condition expression after '.IF' directive.");
            return false;
        }

        // Collect the body for the first branch.
        auto condSlice = pCursor.CollectConditional();
        if (condSlice.has_value() == false)
        {
            mDiag.ReportError(pLocation,
                "'.IF' without matching '.ENDIF'.");
            return false;
        }

        // Store the first branch, then check for further branches.
        branches.emplace_back(PreprocessorConditional { exprSlice, *condSlice });

        bool foundElse = false;
        while (true)
        {
            const auto& token = pCursor.GetNextToken(true);
            const auto type = token.GetKeyword()->GetType<PreprocessorDirective>();
            if (type == PreprocessorDirective::ENDIF)
                { break; }
            
            TokenSlice exprSlice {};
            if (type == PreprocessorDirective::ELSEIF)
            {
                if (foundElse == true)
                {
                    mDiag.ReportError(pLocation,
                        "'.ELSEIF' after '.ELSE' in conditional.");
                    return false;
                }

                exprSlice = pCursor.CollectExpression();
            }
            else if (type == PreprocessorDirective::ELSE)
            {
                if (foundElse == true)
                {
                    mDiag.ReportError(pLocation,
                        "'.ELSE' after '.ELSE' in conditional.");
                    return false;
                }

                foundElse = true;
            }

            auto condSlice = pCursor.CollectConditional();
            if (condSlice.has_value() == false)
            {
                mDiag.ReportError(pLocation,
                    "'{}' without matching '.ENDIF'.",
                    token.Stringify().value_or(""));
                return false;
            }

            branches.emplace_back(PreprocessorConditional { exprSlice, *condSlice });
        }

        for (const auto& branch : branches)
        {
            bool branchGood = false;
            bool isElseCase = branch.mExprSlice.empty();
            if (isElseCase == false)
            {
                TokenCursor exprCursor { branch.mExprSlice };
                auto exprVal = EvaluateExpression(exprCursor);
                if (exprVal.IsUndefined() == true)
                    { return false; }
                else if (exprVal.IsTruthy() == false)
                    { continue; }
                else
                    { branchGood = true; }
            }

            if (branchGood == true || isElseCase == true)
            {
                return (Preprocess(branch.mBodySlice) != PreprocessStatus::Error);
            }
        }

        return true;
    }
}

// Private Methods - Dispatch - Loops ******************************************

namespace G10::ASM
{
    auto Preprocessor::DispatchRepeat (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        // Collect and evaluate the expression following the `.REPEAT` directive.
        auto exprSlice = pCursor.CollectExpression();
        if (exprSlice.empty() == true)
        {
            mDiag.ReportError(pLocation,
                "Expected an expression after the '.REPEAT' directive.");
            return false;
        }

        // Next, collect the body slice.
        auto bodySlice = pCursor.CollectBody(PreprocessorDirective::REPEAT);
        if (bodySlice.has_value() == false)
        {
            mDiag.ReportError(pLocation,
                "'.REPEAT' without matching '.ENDREPEAT'.");
            return false;
        }

        // Skip past the `.ENDREPEAT` directive.
        pCursor.Skip();

        // Evaluate the expression. It must resolve to a non-zero integer.
        TokenCursor exprCursor { exprSlice };
        auto exprVal = EvaluateExpression(exprCursor);
        if (exprVal.IsUndefined() == true)
            { return false; }
        else if (exprVal.IsInteger() == false)
        {
            mDiag.ReportError(pLocation,
                "Count expression in '.REPEAT' directive must resolve to an integer.");
            return false;
        }

        // Make sure we have not breached the specified loop depth.
        if (mLoopDepth >= mLimitLoopDepth)
        {
            mDiag.ReportError(pLocation,
                "Exceeded the loop depth limit of '{}'.", mLimitLoopDepth);
            return false;
        } else { mLoopDepth++; }
        
        PreprocessorInteger count = *exprVal.GetInteger();
        while (count > 0)
        {
            auto status = Preprocess(*bodySlice);
            if (status == PreprocessStatus::Break)
                { ResetStatus(); break; }
            if (status == PreprocessStatus::Continue)
                { ResetStatus(); }
            else if (status == PreprocessStatus::Error)
                { return false; }

            count--;
        }

        mLoopDepth--;
        return true;
    }

    auto Preprocessor::DispatchWhile (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        // Collect the conditional expression after the `.WHILE` directive.
        auto exprSlice = pCursor.CollectExpression();
        if (exprSlice.empty() == true)
        {
            mDiag.ReportError(pLocation,
                "Expected an expression after the '.WHILE' directive.");
            return false;
        }

        // Next, collect the body slice.
        auto bodySlice = pCursor.CollectBody(PreprocessorDirective::WHILE);
        if (bodySlice.has_value() == false)
        {
            mDiag.ReportError(pLocation,
                "'.WHILE' without matching '.ENDWHILE'.");
            return false;
        }

        // Skip past the `.ENDWHILE` directive.
        pCursor.Skip();

        // Make sure we have not breached the specified loop depth.
        if (mLoopDepth >= mLimitLoopDepth)
        {
            mDiag.ReportError(pLocation,
                "Exceeded the loop depth limit of '{}'.", mLimitLoopDepth);
            return false;
        } else { mLoopDepth++; }

        TokenCursor exprCursor { exprSlice };
        while (true)
        {
            exprCursor.SetIndex(0);
            auto val = EvaluateExpression(exprCursor);
            if (val.IsUndefined() == true)
                { return false; }
            else if (val.IsTruthy() == false)
                { break; }

            auto status = Preprocess(*bodySlice);
            if (status == PreprocessStatus::Break)
                { ResetStatus(); break; }
            if (status == PreprocessStatus::Continue)
                { ResetStatus(); }
            else if (status == PreprocessStatus::Error)
                { return false; }
        }

        mLoopDepth--;
        return true;
    }

    auto Preprocessor::DispatchFor (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        // First, collect and interpolate the identifer containing the loop's
        // variable name.
        auto nameToken = pCursor.ExpectNextToken(TokenType::Identifier);
        if (nameToken.has_value() == false)
        {
            mDiag.ReportError(pLocation,
                "Expected identifier for variable name in '.FOR' directive.");
            return false;
        }

        auto name = InterpolateIdentifier(pLocation, 
            nameToken->Stringify().value_or(""));
        if (name.has_value() == false)
            { return false; }

        if (pCursor.ExpectNextToken(TokenType::Comma).has_value() == false)
        {
            mDiag.ReportError(pLocation,
                "Expected ',' between variable name and start expression "
                "in '.FOR' directive.");
            return false;
        }

        // Next, collect the start expression.
        auto startSlice = pCursor.CollectExpression();
        if (startSlice.empty() == true)
        {
            mDiag.ReportError(pLocation,
                "Expected start expression in '.FOR' directive.");
            return false;
        }
        else if (pCursor.ExpectNextToken(TokenType::Comma).has_value() == false)
        {
            mDiag.ReportError(pLocation,
                "Expected ',' between start and end expression "
                "in '.FOR' directive.");
            return false;
        }

        // Repeat for the end expression.
        auto endSlice = pCursor.CollectExpression();
        if (endSlice.empty() == true)
        {
            mDiag.ReportError(pLocation,
                "Expected end expression in '.FOR' directive.");
            return false;
        }

        // At this point, if there is a comma, then expect the optional step
        // expression.
        TokenSlice stepSlice {};
        if (pCursor.ExpectNextToken(TokenType::Comma).has_value() == true)
        {
            stepSlice = pCursor.CollectExpression();
            if (stepSlice.empty() == true)
            {
                mDiag.ReportError(pLocation,
                    "Expected step expression in '.FOR' directive.");
                return false;
            }
        }

        // Now collect the '.FOR' loop's body. Skip past the `.ENDFOR`.
        auto bodySlice = pCursor.CollectBody(PreprocessorDirective::FOR);
        if (bodySlice.has_value() == false)
        {
            mDiag.ReportError(pLocation,
                "'.FOR' without matching '.ENDFOR'.");
            return false;
        }
        pCursor.Skip();

        // Next, evalaute the start expression. Store the evaluated start value 
        // in our looping variable.
        TokenCursor startCursor { startSlice },
                    endCursor { endSlice },
                    stepCursor { stepSlice };
        auto [startVarIter, inserted] = mSymbols.try_emplace(*name,
            PreprocessorSymbol {
                .mValue         = EvaluateExpression(startCursor),
                .mIsConstant    = false,
                .mLocation      = nameToken->mLocation
            }
        );
        auto& startVar = startVarIter->second.mValue;
        if (startVar.IsNumeric() == false)
        {
            mDiag.ReportError(pLocation,
                "Start expression in '.FOR' loop must resolve to a number.");
            return false;
        }

        // Also evaluate the initial value of our loop's step.
        PreprocessorValue stepValue { PreprocessorInteger { 1 }};
        static const PreprocessorValue kZero { PreprocessorInteger { 0 }};
        if (stepSlice.empty() == false)
        {
            stepValue = EvaluateExpression(stepCursor);
            if (stepValue.IsNumeric() == false)
            {
                mDiag.ReportError(pLocation,
                    "Step expression in '.FOR' loop must resolve to a number.");
                return false;
            }
            else if (stepValue == kZero)
            {
                mDiag.ReportError(pLocation,
                    "Step expression in '.FOR' loop must not resolve to zero.");
                return false;
            }
        }

        // At this point, check for a loop depth limit break.
        if (mLoopDepth >= mLimitLoopDepth)
        {
            mDiag.ReportError(pLocation,
                "Exceeded the loop depth limit of '{}'.", mLimitLoopDepth);
            return false;
        } else { mLoopDepth++; }

        while (true)
        {
            // First, check to see if we need to exit the loop.
            endCursor.SetIndex(0);
            auto endValue = EvaluateExpression(endCursor);
            if (endValue.IsNumeric() == false)
            {
                mDiag.ReportError(pLocation,
                    "End expression in '.FOR' loop must resolve to a number.");
                return false;
            }
            else if (
                (stepValue > kZero && startVar >= endValue) ||
                (stepValue < kZero && startVar <= endValue)
            ) { break; }

            // At this point, carry out our loop.
            auto status = Preprocess(*bodySlice);
            if (status == PreprocessStatus::Break)
                { ResetStatus(); break; }
            if (status == PreprocessStatus::Continue)
                { ResetStatus(); }
            else if (status == PreprocessStatus::Error)
                { return false; }

            // Lastly, if provided, re-calculate the step value, then adjust the
            // loop variable accordingly.
            if (stepSlice.empty() == false)
            {
                stepCursor.SetIndex(0);
                stepValue = EvaluateExpression(stepCursor);
                if (stepValue.IsNumeric() == false)
                {
                    mDiag.ReportError(pLocation,
                        "Step expression in '.FOR' loop must resolve to a number.");
                    return false;
                }
                else if (stepValue == kZero)
                {
                    mDiag.ReportError(pLocation,
                        "Step expression in '.FOR' loop must not resolve to zero.");
                    return false;
                }
            }

            startVar += stepValue;
        }

        mLoopDepth--;
        mSymbols.erase(startVarIter);
        return true;
    }

    auto Preprocessor::DispatchContinue (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        if (mLoopDepth == 0)
        {
            mDiag.ReportError(pLocation,
                "'.CONTINUE' encountered outside of loop.");
            return false;
        }

        mPendingStatus = PreprocessStatus::Continue;
        return true;
    }

    auto Preprocessor::DispatchBreak (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        if (mLoopDepth == 0)
        {
            mDiag.ReportError(pLocation,
                "'.BREAK' encountered outside of loop.");
            return false;
        }

        mPendingStatus = PreprocessStatus::Break;
        return true;
    }
}

// Private Methods - Dispatch - Macros *****************************************

namespace G10::ASM
{
    auto Preprocessor::DispatchMacro (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        // 1. Collect, then interpolate the macro name.
        auto nameToken = pCursor.ExpectNextToken(TokenType::Identifier);
        if (nameToken.has_value() == false)
        {
            mDiag.ReportError(pLocation,
                "Expected identifier after '.MACRO' directive.");
            return false;
        }
        auto name = InterpolateIdentifier(pLocation, nameToken->Stringify().value_or(""));
        if (name.has_value() == false)
            { return false; }

        // 2. Collect the macro's named parameter list, then its body.
        auto paramSlice = pCursor.CollectLine();
        auto bodySlice = pCursor.CollectBody(PreprocessorDirective::MACRO);
        if (bodySlice.has_value() == false)
        {
            mDiag.ReportError(pLocation,
                "'.MACRO' without matching '.ENDMACRO'.");
            return false;
        }
        pCursor.Skip();

        // 3. Create the macro structure. Deep-copy the macro body's tokens
        // into it, then parse the parameter list.
        PreprocessorMacro macro {};
        macro.mName = *name;
        macro.mLocation = pLocation;
        macro.mBody.assign_range(*bodySlice);
        
        TokenCursor paramCursor { paramSlice };
        while (paramCursor.IsAtEnd() == false)
        {
            auto paramToken = paramCursor.ExpectNextToken(TokenType::Identifier);
            if (paramToken.has_value() == false)
            {
                mDiag.ReportError(paramToken->mLocation,
                    "Expected identifier for parameter #{} "
                    "in definition of macro '{}'.",
                    macro.mNamedParameters.size() + 1, *name);
                return false;
            }

            auto param = InterpolateIdentifier(paramToken->mLocation,
                paramToken->Stringify().value_or(""));
            if (param.has_value() == false)
                { return false; }

            macro.mNamedParameters.push_back(*param);
            if (paramCursor.ExpectNextToken(TokenType::Comma).has_value() == false)
                { break; }
        }

        mMacros[*name] = std::move(macro);
        return true;
    }

    auto Preprocessor::DispatchMacroCall (TokenCursor& pCursor, 
        const SourceLocation& pLocation, PreprocessorMacro& pMacro) -> bool
    {
        // 0. First, make sure we are not exceeding our set recursion depth limit.
        if (mRecursionDepth > mLimitRecursionDepth)
        {
            mDiag.ReportError(pLocation,
                "Exceeded the recursion depth limit of '{}'.", mLimitRecursionDepth);
            return false;
        } else { mRecursionDepth++; }

        // 1. Collect the arguments being passed into the macro.
        auto argSlice = pCursor.CollectLine();
        
        // 2. Push the macro call into the call stack, then parse the arguments
        // list.
        auto& call = mMacroCallStack.emplace_back(PreprocessorMacroCall {});
        call.mMacro = pMacro;
        call.mLocation = pLocation;
        
        TokenCursor argCursor { argSlice };
        while (argCursor.IsAtEnd() == false)
        {
            auto index = argCursor.GetIndex();
            auto argValue = EvaluateExpression(argCursor);
            if (argValue.IsUndefined() == false)
                { call.mAllArguments.push_back(argValue); }
            else
            {
                argCursor.SetIndex(index);
                auto slice = argCursor.CollectUntil(TokenType::Comma);
                call.mAllArguments.push_back(slice);
            }

            if (argCursor.ExpectNextToken(TokenType::Comma).has_value() == false)
                { break; }
        }

        if (call.mAllArguments.size() < pMacro.mNamedParameters.size())
        {
            mDiag.ReportError(pLocation,
                "Not enough arguments to macro '{}'.", pMacro.mName);
            return false;
        }

        call.mArguments.assign_range(call.mAllArguments);
        call.mMacro->mInvocationCount++;
        auto status = Preprocess(call.mMacro->mBody);
        if (mPendingStatus == PreprocessStatus::Return)
            { ResetStatus(); }

        if (status == PreprocessStatus::Error)
        {
            mDiag.ReportInfo(pLocation,
                "In invocation of macro '{}'.", pMacro.mName);
        }

        mMacroCallStack.pop_back();
        mRecursionDepth--;
        return
            (status != PreprocessStatus::Error) && 
            (mPendingStatus == PreprocessStatus::OK);
    }

    auto Preprocessor::DispatchShift (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        if (mMacroCallStack.empty() == true)
        {
            mDiag.ReportError(pLocation, "'.SHIFT' outside of macro body.");
            return false;
        }

        PreprocessorValue shiftCount { PreprocessorInteger { 1 }};
        auto countSlice = pCursor.CollectExpression();
        if (countSlice.empty() == false)
        {
            TokenCursor countCursor { countSlice };
            shiftCount = EvaluateExpression(countCursor);
            if (shiftCount.IsInteger() == false)
            {
                mDiag.ReportError(pLocation,
                    "Count expression in '.SHIFT' directive must resolve to an integer.");
                return false;
            }
        }    
        
        auto shiftCountInt = *shiftCount.GetInteger();
        if (shiftCountInt <= 0)
        {
            mDiag.ReportError(pLocation,
                "Count expression in '.SHIFT' directive must resolve to a "
                "non-zero, positive integer.");
            return false;
        }

        auto& call = mMacroCallStack.back();
        if (shiftCountInt >=
            static_cast<PreprocessorInteger>(call.mArguments.size()))
        {
            return true;
        }

        for (PreprocessorInteger i = 0; i < shiftCountInt; ++i)
        {
            if (call.mArguments.empty())
                { break; }
            
            call.mArguments.pop_front();
        }

        return true;
    }

    auto Preprocessor::DispatchReturn (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        if (mMacroCallStack.empty() == true)
        {
            mDiag.ReportError(pLocation, "'.RETURN' outside of macro body.");
            return false;
        }

        mPendingStatus = PreprocessStatus::Return;
        return true;
    }
}

// Private Methods - Dispatch - Include ****************************************

namespace G10::ASM
{
    auto Preprocessor::DispatchInclude (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        // 1. Collect and evaluate the expression after the directive.
        auto fileSlice = pCursor.CollectExpression();
        if (fileSlice.empty())
        {
            mDiag.ReportError(pLocation,
                "Expected expression after '.INCLUDE' directive.");
            return false;
        }

        TokenCursor fileCursor { fileSlice };
        auto fileExpr = EvaluateExpression(fileCursor);
        if (fileExpr.IsString() == false)
        {
            mDiag.ReportError(pLocation,
                "Expected string value after '.INCLUDE' directive.");
            return false;
        }

        // 2. The resultant string corresponds to a relative path to the source
        // file to include. Resolve that path relative to this token's source
        // location, and ensure it exists.
        auto pathString = fs::path(*fileExpr.GetString());
        auto resolved = NormalizePath(pLocation.mPath.parent_path() / pathString);
        bool exists = fs::exists(resolved);
        if (exists == false)
        {
            for (const auto& dir : mIncludeDirs)
            {
                auto candidate = NormalizePath(dir / pathString);
                if (fs::exists(candidate))
                {
                    resolved = candidate;
                    exists = true;
                    break;
                }
            }

            if (exists == false)
            {
                mDiag.ReportError(pLocation,
                    "Source file '{}' not found.", resolved.string());
                return false;
            }
        }

        // 3. If the file was marked 'once' and already included, skip it.
        if (mOnceFiles.contains(resolved.string()) == true)
            { return true; }

        // 4. Check for the include depth limit.
        if (mIncludeDepth >= mLimitIncludeDepth)
        {
            mDiag.ReportError(pLocation,
                "Exceeded the include depth limit of '{}'.", mLimitIncludeDepth);
            return false;
        }

        // 5a. Lex the include file.
        Lexer lexer { mDiag };
        if (lexer.LexFile(resolved, false) == false)
        {
            mDiag.ReportInfo(pLocation, "Lexing include file '{}'.",
                resolved.string());
            return false;
        }

        // 5b. Prepare to preprocess the include file's contents.
        std::size_t line = pLocation.mLine;
        mIncludeDepth++;
        EmitText(std::format(
            ".FILE \"{}\"\n"
            ".LINE 1\n",
            resolved.string()
        ));

        // 5c. Preprocess.
        auto status = Preprocess(lexer.GetTokens());
        if (status == PreprocessStatus::Error)
        {
            mDiag.ReportInfo(pLocation, "In included source file '{}'.",
                resolved.string());
        }
        else
        {
            EmitText(std::format(
                ".FILE \"{}\"\n"
                ".LINE {}\n",
                NormalizePath(pLocation.mPath).string(),
                pLocation.mLine + 1
            ));
        }

        mIncludeDepth--;
        return (status != PreprocessStatus::Error);
    }

    auto Preprocessor::DispatchOnce (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool
    {
        fs::path absolute = NormalizePath(pCursor.GetNextToken().mLocation.mPath);
        if (mOnceFiles.contains(absolute.string()) == false)
            { mOnceFiles.insert(absolute.string()); }

        return true;
    }
}
