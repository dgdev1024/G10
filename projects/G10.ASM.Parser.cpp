/**
 * @file    G10.ASM.Parser.cpp
 * @brief   Contains implementations for the G10 Assembler's parser component,
 *          and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Parser.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::ASM
{
    Parser::Parser (Diagnostic& pDiag, const PreprocessorSymbolTable& pSymbolTable) :
        mDiag           { pDiag },
        mSymbolTable    { pSymbolTable }
    {}
}

// Public Methods - Input & Output *********************************************

namespace G10::ASM
{
    auto Parser::ParseInput (TokenSlice pSlice) -> bool
    {
        TokenCursor cursor { pSlice };
        while (cursor.IsAtEnd() == false)
        {
            cursor.SkipNewlines();
            const auto& token = cursor.GetNextToken();
            // debug("Parsing token: '{}' ({}, {})", 
            //     token.Stringify().value_or(""),
            //     cursor.GetIndex(),
            //     token.mLocation.ToString());
            if (const auto kw = token.GetKeyword())
            {
                cursor.Skip();
                if (DispatchKeyword(*kw, token.mLocation, cursor) == false)
                    { return false; }
            }
            else if (token.mType == TokenType::Identifier)
            {
                if (DispatchLabelStatement(token.mLocation, cursor) == false)
                    { return false; }
            }
        }

        return true;
    }

    auto Parser::GetOutput () const -> const SyntaxModule&
    {
        return mOutput;
    }
}

// Private Methods *************************************************************

namespace G10::ASM
{
    
}
