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
        _LOCATION,
        _RANDOM32,
        _RANDOM64
    };

    enum class PreprocessorDirective : std::uint8_t
    {
        CONST,
        LET,
        SNIPPET,
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
        CHARMAP,
        NEWCHARMAP,
        SETCHARMAP,
        PUSHCHARMAP,
        POPCHARMAP,
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
        DEFINED,
        ISCONST,
        ISMACRO,
        ISSYMBOL,
        ISSNIPPET,

        // Integer Functions
        HIGHDWORD,              // `HIGHDWORD(n) - Returns upper 32 bits of 64-bit integer `n`.
        LOWDWORD,               // `LOWDWORD(n)` - Returns lower 32 bits of 64-bit integer `n`.
        HIGHWORD,               // `HIGHWORD(n)` - Returns upper 16 bits of 32-bit integer `n`.
        LOWWORD,                // `LOWWORD(n)` - Returns lower 16 bits of 32-bit integer `n`.
        HIGHBYTE,               // `HIGHBYTE(n)` - Returns upper 8 bits of 16-bit integer `n`.
        LOWBYTE,                // `LOWBYTE(n)` - Returns lower 8 bits of 16-bit integer `n`.
        HIGHNIBBLE,             // `HIGHNIBBLE(n)` - Returns upper 4 bits of 8-bit integer `n`.
        LOWNIBBLE,              // `LOWNIBBLE(n)` - Returns lower 4 bits of 8-bit integer `n`.
        HIGH,                   // `HIGH(n)` - Returns upper half of integer `n`. Size-adaptive.
        LOW,                    // `LOW(n)` - Returns lower half of integer `n`. Size-adaptive.
        BITWIDTH,               // `BITWIDTH(n)` - Returns the number of bits required to represent the integer `n`.
        TZCOUNT,                // `TZCOUNT(n)` - Returns the number of trailing zero bits in the binary representation of integer `n`.
    
        // Fixed-Point Functions
        FINT,                   // `FINT(f)` - Returns the integer part of fixed-point `f`.
        FFRAC,                  // `FFRAC(f)` - Returns the fractional part of fixed-point `f`.
        FADD,                   // `FADD(x, y)` - Returns the fixed-point addition of `x` and `y`.
        FSUB,                   // `FSUB(x, y)` - Returns the fixed-point subtraction of `x` and `y`.
        FDIV,                   // `FDIV(x, y)` - Returns the fixed-point division of `x` by `y`.
        FMUL,                   // `FMUL(x, y)` - Returns the fixed-point multiplication of `x` by `y`.
        FMOD,                   // `FMOD(x, y)` - Returns the fixed-point modulus (remainder of division) of `x` by `y`.
        FPOW,                   // `FPOW(x, y)` - Returns the fixed-point power of `x` raised to the power of `y`.
        FSQRT,                  // `FSQRT(x)` - Returns the fixed-point square root of `x`.
        FROOT,                  // `FROOT(x, y)` - Returns the fixed-point `y`th root of `x`.
        FLOG,                   // `FLOG(x, y)` - Returns the fixed-point logarithm of `x` with base `y`.
        FLN,                    // `FLN(x)` - Returns the fixed-point natural logarithm of `x`.
        FCEIL,                  // `FCEIL(x)` - Returns fixed-point `x`, rounded to the next integer.
        FFLOOR,                 // `FFLOOR(x)` - Returns fixed-point `x`, rounded to the previous integer.
        FROUND,                 // `FROUND(x)` - Returns fixed-point `x`, rounded to the nearest integer.

        // Trigonometric Functions
        FRADT,                  // `FRADT(r)` - Converts `r` radians into turns. One turn = 2PI radians.
        FDEGT,                  // `FDEGT(d)` - Converts `d` degrees into turns. One turn = 360 degrees.
        FSIN,                   // `FSIN(t)` - Returns the fixed-point sine of `t` turns.
        FCOS,                   // `FCOS(t)` - Returns the fixed-point cosine of `t` turns.
        FTAN,                   // `FTAN(t)` - Returns the fixed-point tangent of `t` turns.
        FASIN,                  // `FASIN(t)` - Returns the fixed-point arcsine of `t` turns.
        FACOS,                  // `FACOS(t)` - Returns the fixed-point arccosine of `t` turns.
        FATAN,                  // `FATAN(t)` - Returns the fixed-point arctangent of `t` turns.
        FATAN2,                 // `FATAN2(y, x)` - Calculates the angle between points [`x`, `y`] and [1, 0].

        // String Functions
        STRCAT,                 // `STRCAT(s1, s2, ...)` - Concatenates two or more strings.
        STRUPR,                 // `STRUPR(s)` - Converts string `s` to uppercase.
        STRLWR,                 // `STRLWR(s)` - Converts string `s` to lowercase.
        STRSLICE,               // `STRSLICE(s, start, end)` - Returns a slice of the string `s` from index `start` to `end`.
        STRSLICEC,              // `STRSLICEC(s, start, count)` - Returns a slice of the string `s` from index `start` with a length of `count`.
        STRRPL,                 // `STRRPL(s, old, new)` - Replaces all occurrences of `old` with `new` in string `s`.
        STRCHAR,                // `STRCHAR(s, index)` - Returns the character/substring at the specified index of the current charmap in the string `s`.
        REVCHAR,                // `REVCHAR(n1, ...)` - Returns a character/substring in the current charmap, mapped to the given sequence of integers.
        // READFILE,               // `READFILE(name, max)` - Reads `max` bytes of file `name` as a string.
        STRLEN,                 // `STRLEN(s)` - Returns the character length of string `s`.
        STRCMP,                 // `STRCMP(s1, s2)` - Compares two strings.
        STRNCMP,                // `STRNCMP(s1, s2, n)` - Compares up to `n` characters of two strings.
        STRFIND,                // `STRFIND(s, substr)` - Finds the first occurrence of `substr` in string `s`.
        STRRFIND,               // `STRRFIND(s, substr)` - Finds the last occurrence of `substr` in string `s`.
        BYTELEN,                // `BYTELEN(s)` - Returns the byte length of string `s`. Non-ASCII characters can be more than one byte.
        STRBYTE,                // `STRBYTE(s, index)` - Returns the byte at the specified index of the string `s`.
        INCHARMAP,              // `INCHARMAP(s)` - Checks if the string `s` is in the current charmap.
        CHARLEN,                // `CHARLEN(str)` - Returns the number of charmap entries, in the current charmap, in string `str`.
        CHARCMP,                // `CHARCMP(s1, s2)` - Compares two strings according to their charmap entries in the current charmap.
        CHARSIZE,               // `CHARSIZE(c)` - Returns the number of values associated with the character/substring `c` in the current charmap.
        CHARVAL,                // `CHARVAL(c, index)` - Returns the value at the specified index of the character/substring `c` in the current charmap.
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
        LINE,
        CM
    };
}

// Structures ******************************************************************

namespace G10::ASM
{
    struct SourceLocation final
    {
        std::string     mPath { "" };
        std::size_t     mLine { 1 };
        std::size_t     mColumn { 1 };

    public: // Methods *********************************************************

        inline constexpr auto ToString () const -> std::string
        {
            if (mPath.empty() == true)
                { return ""; }
            else if (mLine > 0 && mColumn > 0)
                { return std::format("{}:{}:{}: ", mPath, mLine, mColumn); }
            else if (mLine > 0)
                { return std::format("{}:{}: ", mPath, mLine); }
            else
                { return std::format("{}: ", mPath); }
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

    inline constexpr auto ToUTF8 (const std::uint64_t& charCode) -> std::string
    {
        if (charCode <= 0x7F)
        {
            return std::string(1, static_cast<char>(charCode));
        }
        else if (charCode <= 0x7FF)
        {
            return std::string({
                static_cast<char>(0xC0 | (charCode >> 6)),
                static_cast<char>(0x80 | (charCode & 0x3F))
            });
        }
        else if (charCode <= 0xFFFF)
        {
            return std::string({
                static_cast<char>(0xE0 | (charCode >> 12)),
                static_cast<char>(0x80 | ((charCode >> 6) & 0x3F)),
                static_cast<char>(0x80 | (charCode & 0x3F))
            });
        }
        else
        {
            return std::string({
                static_cast<char>(0xF0 | (charCode >> 18)),
                static_cast<char>(0x80 | ((charCode >> 12) & 0x3F)),
                static_cast<char>(0x80 | ((charCode >> 6) & 0x3F)),
                static_cast<char>(0x80 | (charCode & 0x3F))
            });
        }
    }
}