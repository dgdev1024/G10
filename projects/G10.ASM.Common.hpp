/**
 * @file    G10.ASM.Common.hpp
 * @brief   Contains definitions commonly used across the G10 CPU Assembler and
 *          Linker library's codebase.
 */

#pragma once

// Includes ********************************************************************

#include <G10.Common.hpp>

// Constants & Enumerations ****************************************************

namespace G10::ASM
{
    constexpr std::uint32_t
        kMajorVersion = 1,
        kMinorVersion = 0,
        kPatchVersion = 0,
        kVersion =
            (kMajorVersion * 10000) +
            (kMinorVersion * 100) +
            kPatchVersion;

    constexpr std::size_t
        kDefaultInterpolationDepth  = 16,
        kDefaultLoopDepth           = 32,
        kDefaultIncludeDepth        = 16,
        kDefaultRecursionDepth      = 256;

    constexpr std::size_t
        kMinimumInterpolationDepth  = 1,
        kMinimumLoopDepth           = 1,
        kMinimumIncludeDepth        = 1,
        kMinimumRecursionDepth      = 1;

    constexpr std::size_t
        kMaximumInterpolationDepth  = 256,
        kMaximumLoopDepth           = 65536,
        kMaximumIncludeDepth        = 256,
        kMaximumRecursionDepth      = 65536;

    enum class AssemblerDirective : std::uint8_t
    {
        BYTE,
        WORD,
        DWORD,
        ASCIZ,
        SPACE,
        INCBIN,
        EXPORT,
        IMPORT,
        SECTION,
        ORG,
        ALIGN
    };

    enum class PreprocessorBuiltinSymbol : std::uint8_t
    {
        _NARG,
        _TARG,
        _LINE,
        _FILE,
        _DATE,
        _TIME,
        _VERSION,
        _UNIQUE,
        _RANDOM32,
        _RANDOM64
    };

    enum class PreprocessorDirective : std::uint8_t
    {
        CONST,
        LET,
        IF,
        IFDEF,
        IFNDEF,
        ELSEIF,
        ELSE,
        ENDIF,
        REPEAT,
        ENDREPEAT,
        FOR,
        ENDFOR,
        WHILE,
        ENDWHILE,
        CONTINUE,
        BREAK,
        INCLUDE,
        ONCE,
        MACRO,
        ENDMACRO,
        SHIFT,
        RETURN,
        OPTION,
        PRINT,
        PRINTLN,
        INFO,
        WARNING,
        ERROR,
        ASSERT
    };

    enum class PreprocessorFunction : std::uint8_t
    {
        HIGH,
        LOW,
        HIDWORD,
        LODWORD,
        HIWORD,
        LOWORD,
        HIBYTE,
        LOBYTE,
        HINIBBLE,
        LONIBBLE,
        BITWIDTH,
        TZCOUNT,
        FINT,
        FFRAC,
        FADD,
        FSUB,
        FDIV,
        FMUL,
        FMOD,
        FPOW,
        FSQRT,
        FROOT,
        FLOG,
        FLN,
        FROUND,
        FCEIL,
        FFLOOR,
        FRADT,
        FDEGT,
        FSIN,
        FCOS,
        FTAN,
        FASIN,
        FACOS,
        FATAN,
        FATAN2,
        STRCAT,
        STRNCAT,
        STRUPR,
        STRLWR,
        STRSLICE,
        STRREPLACE,
        STRFMT,
        STRLEN,
        STRCMP,
        STRNCMP,
        STRFIND,
        STRRFIND,
        STRNFIND,
        STRNRFIND,
        BYTELEN,
        STRBYTE,
        DEFINED,
        ISCONST,
        ISMACRO,
        ISSYMBOL,
    };

    enum class SectionName : std::uint8_t
    {
        METADATA,
        INT0,
        INT1,
        INT2,
        INT3,
        INT4,
        INT5,
        INT6,
        INT7,
        INT8,
        INT9,
        INT10,
        INT11,
        INT12,
        INT13,
        INT14,
        INT15,
        INT16,
        INT17,
        INT18,
        INT19,
        INT20,
        INT21,
        INT22,
        INT23,
        INT24,
        INT25,
        INT26,
        INT27,
        INT28,
        INT29,
        INT30,
        INT31,
        CODE,
        DATA,
        BSS
    };

    enum class Hint : std::uint8_t
    {
        Directive,
        FILE,
        LINE
    };
}

// Structures ******************************************************************

namespace G10::ASM
{
    struct SourceLocation final
    {
        fs::path    mPath { "" };
        std::size_t mLine { 1 };
        std::size_t mColumn { 1 };

    public: // Methods *********************************************************

        inline constexpr auto ToString () const -> std::string
        {
            if (mPath.empty() == true)
                { return ""; }
            else if (mLine > 0 && mColumn > 0)
                { return std::format("{}:{}:{}: ", mPath.string(), mLine, mColumn); }
            else if (mLine > 0)
                { return std::format("{}:{}: ", mPath.string(), mLine); }
            else
                { return std::format("{}: ", mPath.string()); }
        }

    };
}

// Functions *******************************************************************

namespace G10::ASM
{
    inline constexpr auto NormalizePath (const fs::path& path) -> fs::path
    {
        return fs::absolute(path).lexically_normal();
    }
}