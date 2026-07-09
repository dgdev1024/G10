/**
 * @file    G10.ASM.Preprocessor.Middleware.cpp
 * @brief   Contains implementations for the G10 assembler preprocessor's
 *          passthrough middleware methods, and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Preprocessor.hpp>

// Private Methods - Middleware ************************************************

namespace G10::ASM
{
    auto Preprocessor::DispatchMiddleware (TokenCursor& pCursor, 
        const Token& pToken) -> bool
    {
        pCursor.Skip();
        if (auto kw = pToken.GetKeyword(); kw)
        {
            if (auto asd = kw->GetTypeIf<AssemblerDirective>(); asd)
            {
                switch (*asd)
                {
                    case AssemblerDirective::ORG: return DispatchOrgMiddleware(pCursor, pToken.mLocation);
                    case AssemblerDirective::SECTION: return DispatchSectionMiddleware(pCursor, pToken.mLocation);
                    case AssemblerDirective::BYTE: return DispatchByteMiddleware(pCursor, pToken.mLocation);
                    case AssemblerDirective::WORD: return DispatchWordMiddleware(pCursor, pToken.mLocation);
                    case AssemblerDirective::DWORD: return DispatchDoubleWordMiddleware(pCursor, pToken.mLocation);
                    case AssemblerDirective::ASCIZ: return DispatchStringMiddleware(pCursor, pToken.mLocation);
                    default: break;
                }
            }
            else if (auto inst = kw->GetTypeIf<CPU::InstructionType>(); inst)
            {
                return DispatchInstructionMiddleware(pCursor, pToken.mLocation, *inst);
            }
        }

        return true;
    }

    auto Preprocessor::DispatchOrgMiddleware (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        return true;
    }

    auto Preprocessor::DispatchSectionMiddleware (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        return true;
    }

    auto Preprocessor::DispatchByteMiddleware (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        return true;
    }

    auto Preprocessor::DispatchWordMiddleware (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        return true;
    }

    auto Preprocessor::DispatchDoubleWordMiddleware (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        return true;
    }

    auto Preprocessor::DispatchStringMiddleware (TokenCursor& pCursor, 
        const SourceLocation& pLocation) -> bool
    {
        return true;
    }

    auto Preprocessor::DispatchInstructionMiddleware (TokenCursor& pCursor, 
        const SourceLocation& pLocation, CPU::InstructionType pInst) -> bool
    {
        return true;
    }
}
