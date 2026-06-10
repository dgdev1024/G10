/**
 * @file    G10.ASM.Preprocessor.Emission.cpp
 * @brief   Contains implementations for the G10 Assembler Preprocessor's
 *          text output emission methods, and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Preprocessor.hpp>

// Private Methods - Text Output Emission **************************************

namespace G10::ASM
{
    auto Preprocessor::EmitNewline () -> void
        { mOutput += '\n'; }

    auto Preprocessor::EmitText (std::string_view pText) -> void
        { mOutput += pText; }

    auto Preprocessor::EmitToken (const Token& pToken) -> void
    {
        if (pToken.mType == TokenType::FloatingPointLiteral)
        {
            PreprocessorFixedPoint fp { pToken.mFloat.value_or(0.0) };
            mOutput += std::to_string(fp.Correct().GetUnsignedInteger()) + ' ';
        }
        else if (pToken.IsInteger() == true)
        {
            mOutput += std::to_string(pToken.mInteger.value_or(0) & 0xFFFFFFFF) + ' ';
        }
        else if (const auto str = pToken.Stringify())
        {
            if (pToken.mType == TokenType::StringLiteral)
                { mOutput += std::format("\"{}\" ", *str); }
            else
                { mOutput += *str + ' '; }
        }
    }

    auto Preprocessor::EmitValue (const PreprocessorValue& pValue) -> void
    {
        if (const auto fp = pValue.GetFixedPoint())
            { mOutput += std::to_string(fp->Correct().GetUnsignedInteger()) + " "; }
        else if (const auto i = pValue.GetInteger())
            { mOutput += std::to_string(static_cast<std::uint32_t>(*i & 0xFFFFFFFF)) + " "; }
        else if (const auto str = pValue.GetString())
            { mOutput += std::format("\"{}\" ", *str); }
    }
}
