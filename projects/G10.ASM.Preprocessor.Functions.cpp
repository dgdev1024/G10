/**
 * @file    G10.ASM.Preprocessor.Functions.cpp
 * @brief   Contains implementations for the G10 Assembler Preprocessor's
 *          function evaluation methods, and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Preprocessor.hpp>

// Private Methods - Expressions - Functions ***********************************

namespace G10::ASM
{
    static constexpr double kTurn = (2 * std::numbers::pi);

    struct CallContext final
    {
        Diagnostic& mDiag;
        const Charmap& mCharmap;
        const std::vector<fs::path>& mIncludeDirs;
        std::vector<PreprocessorValue> mArgs {};

    public:

        inline auto GetInteger (std::size_t pIndex) ->
            std::optional<PreprocessorInteger>
        {
            if (pIndex >= mArgs.size() || mArgs[pIndex].IsInteger() == false)
                { return std::nullopt; }

            return *mArgs[pIndex].GetInteger();
        } 

        inline auto GetFixedPoint (std::size_t pIndex) ->
            std::optional<PreprocessorFixedPoint>
        {
            if (pIndex >= mArgs.size() || mArgs[pIndex].IsNumeric() == false)
                { return std::nullopt; }

            if (const auto fp = mArgs[pIndex].GetFixedPoint())
                { return *fp; }
            else if (const auto i = mArgs[pIndex].GetInteger())
                { return PreprocessorFixedPoint { static_cast<std::int32_t>(*i), 0 }; }

            return std::nullopt;
        }

        inline auto GetString (std::size_t pIndex) ->
            std::optional<std::string>
        {
            if (pIndex >= mArgs.size() || mArgs[pIndex].IsString() == false)
                { return std::nullopt; }

            return *mArgs[pIndex].GetString();
        }

        inline auto ParseStringAgainstCharmap (const std::string& pString) ->
            std::vector<std::string>
        {
            std::size_t index = 0;
            std::vector<std::string> substrings {};
            while (index < pString.length())
            {
                bool found = false;
                for (const auto& [substr, bytes] : mCharmap)
                {
                    if (pString.compare(index, substr.length(), substr) == 0)
                    {
                        substrings.emplace_back(substr);
                        index += substr.length();
                        found = true;
                        break;
                    }
                }

                if (found == false)
                {
                    substrings.emplace_back(pString.substr(index, 1));
                    index++;
                }
            }

            return substrings;
        }

    };

    static const std::unordered_map<
        PreprocessorFunction,
        std::function<PreprocessorValue(CallContext&)>
    > kBuiltinFunctions = 
    {
        // Integer Functions
        {
            PreprocessorFunction::HIGHDWORD,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto n = pContext.GetInteger(0);
                if (n.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected an integer for argument #1");
                    return {};
                }

                return PreprocessorInteger { 
                    static_cast<std::int64_t>((n.value() & 0xFFFFFFFF00000000) >> 32) };
            }
        },
        {
            PreprocessorFunction::LOWDWORD,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto n = pContext.GetInteger(0);
                if (n.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected an integer for argument #1");
                    return {};
                }

                return PreprocessorInteger { 
                    static_cast<std::int64_t>(n.value() & 0xFFFFFFFF) };
            }
        },
        {
            PreprocessorFunction::HIGHWORD,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto n = pContext.GetInteger(0);
                if (n.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected an integer for argument #1");
                    return {};
                }

                return PreprocessorInteger { 
                    static_cast<std::int64_t>((n.value() & 0xFFFF0000) >> 16) };
            }
        },
        {
            PreprocessorFunction::LOWWORD,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto n = pContext.GetInteger(0);
                if (n.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected an integer for argument #1");
                    return {};
                }

                return PreprocessorInteger { 
                    static_cast<std::int64_t>(n.value() & 0xFFFF) };
            }
        },
        {
            PreprocessorFunction::HIGHBYTE,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto n = pContext.GetInteger(0);
                if (n.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected an integer for argument #1");
                    return {};
                }

                return PreprocessorInteger { 
                    static_cast<std::int64_t>(((n.value() & 0xFF00) >> 8)) };
            }
        },
        {
            PreprocessorFunction::LOWBYTE,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto n = pContext.GetInteger(0);
                if (n.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected an integer for argument #1");
                    return {};
                }

                return PreprocessorInteger { 
                    static_cast<std::int64_t>(n.value() & 0xFF) };
            }
        },
        {
            PreprocessorFunction::HIGHNIBBLE,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto n = pContext.GetInteger(0);
                if (n.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected an integer for argument #1");
                    return {};
                }

                return PreprocessorInteger { 
                    static_cast<std::int64_t>(((n.value() & 0xF0) >> 4)) };
            }
        },
        {
            PreprocessorFunction::LOWNIBBLE,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto n = pContext.GetInteger(0);
                if (n.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected an integer for argument #1");
                    return {};
                }

                return PreprocessorInteger { 
                    static_cast<std::int64_t>(n.value() & 0x0F) };
            }
        },
        {
            PreprocessorFunction::HIGH,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto n = pContext.GetInteger(0);
                if (n.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected an integer for argument #1");
                    return {};
                }

                return PreprocessorInteger { 
                    static_cast<std::int64_t>(
                        n.value() > 0xFFFFFFFF ? ((n.value() & 0xFFFFFFFF00000000) >> 32) :
                        n.value() > 0xFFFF ? ((n.value() & 0xFFFF0000) >> 16) :
                        n.value() > 0xFF ? ((n.value() & 0xFF00) >> 8) :
                        ((n.value() & 0xF0) >> 4)
                    ) 
                };
            }
        },
        {
            PreprocessorFunction::LOW,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto n = pContext.GetInteger(0);
                if (n.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected an integer for argument #1");
                    return {};
                }

                return PreprocessorInteger { 
                    static_cast<std::int64_t>(
                        n.value() > 0xFFFFFFFF ? (n.value() & 0xFFFFFFFF) :
                        n.value() > 0xFFFF ? (n.value() & 0xFFFF) :
                        n.value() > 0xFF ? (n.value() & 0xFF) :
                        (n.value() & 0xF)
                    ) 
                };
            }
        },
        {
            PreprocessorFunction::BITWIDTH,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto n = pContext.GetInteger(0);
                if (n.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected an integer for argument #1");
                    return {};
                }

                return PreprocessorInteger { 
                    std::bit_width(static_cast<std::uint64_t>(n.value())) 
                };
            }
        },
        {
            PreprocessorFunction::TZCOUNT,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto n = pContext.GetInteger(0);
                if (n.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected an integer for argument #1");
                    return {};
                }

                return PreprocessorInteger { 
                    std::countr_zero(static_cast<std::uint64_t>(n.value())) 
                };
            }
        },

        // Fixed-Point Functions
        {
            PreprocessorFunction::FINT,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto f = pContext.GetFixedPoint(0);
                if (f.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorInteger { static_cast<std::int64_t>(f->Correct().GetInteger()) };
            }
        },
        {
            PreprocessorFunction::FFRAC,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto f = pContext.GetFixedPoint(0);
                if (f.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorInteger { static_cast<std::int64_t>(f->Correct().GetFractional()) };
            }
        },
        {
            PreprocessorFunction::FADD,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);
                auto rhs = pContext.GetFixedPoint(1);

                if (lhs.has_value() == false || rhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected two fixed-point numbers for arguments #1 and #2");
                    return {};
                }

                return PreprocessorFixedPoint { lhs->GetComputed() + rhs->GetComputed() };
            }
        },
        {
            PreprocessorFunction::FSUB,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);
                auto rhs = pContext.GetFixedPoint(1);

                if (lhs.has_value() == false || rhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected two fixed-point numbers for arguments #1 and #2");
                    return {};
                }

                return PreprocessorFixedPoint { lhs->GetComputed() - rhs->GetComputed() };
            }
        },
        {
            PreprocessorFunction::FDIV,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);
                auto rhs = pContext.GetFixedPoint(1);

                if (lhs.has_value() == false || rhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected two fixed-point numbers for arguments #1 and #2");
                    return {};
                }

                if (rhs.value().GetRaw() == 0)
                {
                    pContext.mDiag.ReportError("Division by zero is not allowed.");
                    return {};
                }

                return PreprocessorFixedPoint { lhs->GetComputed() / rhs->GetComputed() };
            }
        },
        {
            PreprocessorFunction::FMUL,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);
                auto rhs = pContext.GetFixedPoint(1);

                if (lhs.has_value() == false || rhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected two fixed-point numbers for arguments #1 and #2");
                    return {};
                }

                return PreprocessorFixedPoint { lhs->GetComputed() * rhs->GetComputed() };
            }
        },
        {
            PreprocessorFunction::FMOD,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);
                auto rhs = pContext.GetFixedPoint(1);

                if (lhs.has_value() == false || rhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected two fixed-point numbers for arguments #1 and #2");
                    return {};
                }

                if (rhs.value().GetRaw() == 0)
                {
                    pContext.mDiag.ReportError("Modulo by zero is not allowed.");
                    return {};
                }

                return PreprocessorFixedPoint { std::fmod(lhs->GetComputed(), rhs->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FPOW,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);
                auto rhs = pContext.GetFixedPoint(1);

                if (lhs.has_value() == false || rhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected two fixed-point numbers for arguments #1 and #2");
                    return {};
                }

                return PreprocessorFixedPoint { std::pow(lhs->GetComputed(), rhs->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FSQRT,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);

                if (lhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorFixedPoint { std::sqrt(lhs->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FROOT,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);
                auto rhs = pContext.GetFixedPoint(1);

                if (lhs.has_value() == false || rhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected two fixed-point numbers for arguments #1 and #2");
                    return {};
                }

                return PreprocessorFixedPoint { std::pow(lhs->GetComputed(), 1.0 / rhs->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FLOG,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);
                auto rhs = pContext.GetFixedPoint(1);

                if (lhs.has_value() == false || rhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected two fixed-point numbers for arguments #1 and #2");
                    return {};
                }

                return PreprocessorFixedPoint { 
                    std::log(lhs->GetComputed()) /
                        std::log(rhs->GetComputed())
                };
            }
        },
        {
            PreprocessorFunction::FLN,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);

                if (lhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorFixedPoint { std::log(lhs->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FROUND,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);

                if (lhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorFixedPoint { std::round(lhs->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FCEIL,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);

                if (lhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorFixedPoint { std::ceil(lhs->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FFLOOR,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);

                if (lhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorFixedPoint { std::floor(lhs->GetComputed()) };
            }
        },

        // Trigonometric Functions
        {
            PreprocessorFunction::FRADT,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);

                if (lhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorFixedPoint { lhs->GetComputed() * kTurn };
            }
        },
        {
            PreprocessorFunction::FDEGT,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);

                if (lhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                double radians = lhs->GetComputed() * (std::numbers::pi / 180.0);
                return PreprocessorFixedPoint { radians * kTurn };
            }
        },
        {
            PreprocessorFunction::FSIN,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);

                if (lhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorFixedPoint { 
                    std::sin(lhs->GetComputed() * kTurn) };
            }
        },
        {
            PreprocessorFunction::FCOS,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);

                if (lhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorFixedPoint { 
                    std::cos(lhs->GetComputed() * kTurn) };
            }
        },
        {
            PreprocessorFunction::FTAN,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);

                if (lhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorFixedPoint { 
                    std::tan(lhs->GetComputed() * kTurn) };
            }
        },
        {
            PreprocessorFunction::FASIN,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);

                if (lhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorFixedPoint { 
                    std::asin(lhs->GetComputed()) / kTurn };
            }
        },
        {
            PreprocessorFunction::FACOS,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);

                if (lhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorFixedPoint { 
                    std::acos(lhs->GetComputed()) / kTurn };
            }
        },
        {
            PreprocessorFunction::FATAN,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);

                if (lhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected a fixed-point number for argument #1");
                    return {};
                }

                return PreprocessorFixedPoint { 
                    std::atan(lhs->GetComputed()) / kTurn };
            }
        },
        {
            PreprocessorFunction::FATAN2,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto lhs = pContext.GetFixedPoint(0);
                auto rhs = pContext.GetFixedPoint(1);

                if (lhs.has_value() == false || rhs.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected fixed-point numbers for both arguments");
                    return {};
                }

                return PreprocessorFixedPoint { 
                    std::atan2(lhs->GetComputed(), rhs->GetComputed()) / kTurn };
            }
        },

        // String Functions
        {
            PreprocessorFunction::STRCAT,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `STRCAT(str1, str2, ...)`
                // - Concatenates two or more strings.
                auto str1 = pContext.GetString(0);
                auto str2 = pContext.GetString(1);

                if (str1.has_value() == false || str2.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected at least two string arguments for `STRCAT`.");
                    return {};
                }
                
                std::string concat = *str1 + *str2;
                for (std::size_t i = 2; i < pContext.mArgs.size(); ++i)
                {
                    auto str = pContext.GetString(i);
                    if (str.has_value() == false)
                    {
                        pContext.mDiag.ReportError("Expected string argument #{} for `STRCAT`.", i + 1);
                        return {};
                    }
                    concat += *str;
                }

                return PreprocessorString { concat };
            }
        },
        {
            PreprocessorFunction::STRUPR,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto str = pContext.GetString(0);
                if (str.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected string argument for `STRUPR`.");
                    return {};
                }

                std::string upper = *str;
                std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
                return PreprocessorString { upper };
            }
        },
        {
            PreprocessorFunction::STRLWR,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                auto str = pContext.GetString(0);
                if (str.has_value() == false)
                {
                    pContext.mDiag.ReportError("Expected string argument for `STRLWR`.");
                    return {};
                }

                std::string lower = *str;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                return PreprocessorString { lower };
            }
        },
        {
            PreprocessorFunction::STRSLICE,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `STRSLICE(str, start[, stop])`
                // - Returns a substring of `str`, starting at `start` and ending
                //   at `stop` (exclusive).
                // - If `stop` is not specified, then the substring returned
                //   runs to the end of `str`.
                auto str = pContext.GetString(0);
                if (str.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #1 to `STRSLICE` must be a string.");
                    return {};
                }

                auto start = pContext.GetInteger(1);
                if (start.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #2 to `STRSLICE` must be an integer.");
                    return {};
                }

                std::size_t start_pos = *start;
                std::size_t stop_pos = stx::npos64;
                if (pContext.mArgs.size() >= 3)
                {
                    auto stop = pContext.GetInteger(2);
                    if (stop.has_value() == false)
                    {
                        pContext.mDiag.ReportError("Argument #3 to `STRSLICE` must be an integer.");
                        return {};
                    }
                    stop_pos = *stop;
                }

                return PreprocessorString { str->substr(start_pos, stop_pos - start_pos) };
            }
        },
        {
            PreprocessorFunction::STRSLICEC,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `STRSLICEC(str, start[, count])`
                // - Returns a substring of `str`, starting at `start` and ending
                //   at `start + count` (exclusive).
                // - If `count` is not specified, then the substring returned
                //   runs to the end of `str`.
                auto str = pContext.GetString(0);
                if (str.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #1 to `STRSLICEC` must be a string.");
                    return {};
                }

                auto start = pContext.GetInteger(1);
                if (start.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #2 to `STRSLICEC` must be an integer.");
                    return {};
                }

                std::size_t start_pos = *start;
                std::size_t count = stx::npos64;
                if (pContext.mArgs.size() >= 3)
                {
                    auto _count = pContext.GetInteger(2);
                    if (_count.has_value() == false)
                    {
                        pContext.mDiag.ReportError("Argument #3 to `STRSLICEC` must be an integer.");
                        return {};
                    }
                    count = *_count;
                }

                return PreprocessorString { str->substr(start_pos, count) };
            }
        },
        {
            PreprocessorFunction::STRRPL,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `STRRPL(str, old, new)`
                // - Returns a string with all occurrences of `old` replaced by `new`.
                auto str = pContext.GetString(0);
                if (str.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #1 to `STRRPL` must be a string.");
                    return {};
                }

                auto old = pContext.GetString(1);
                if (old.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #2 to `STRRPL` must be a string.");
                    return {};
                }

                auto _new = pContext.GetString(2);
                if (_new.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #3 to `STRRPL` must be a string.");
                    return {};
                }
                
                std::size_t find = str->find(*old);
                while (find != std::string::npos)
                {
                    str->replace(find, old->length(), *_new);
                    find = str->find(*old, find + _new->length());
                }

                return PreprocessorString { *str };
            }
        },
        {
            PreprocessorFunction::STRCHAR,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `STRCHAR(str, idx)`
                // - Returns the substring from `str` for the charmap entry at
                //   index `idx` within the current charmap.
                // - `idx` counts charmap entries, not characters.
                // - The call context contains a const handle to the currently
                //   selected charmap.
                auto str = pContext.GetString(0);
                if (str.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #1 to `STRCHAR` must be a string.");
                    return {};
                }

                auto idx = pContext.GetInteger(1);
                if (idx.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #2 to `STRCHAR` must be an integer.");
                    return {};
                }

                auto substrs = pContext.ParseStringAgainstCharmap(*str);
                if (*idx < 0 || *idx >= static_cast<std::int64_t>(substrs.size()))
                {
                    pContext.mDiag.ReportError("Index out of bounds for `STRCHAR`.");
                    return {};
                }

                return PreprocessorString { substrs[*idx] };
            }
        },
        {
            PreprocessorFunction::REVCHAR,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `REVCHAR(val)`
                // - Returns the string which is mapped to `val` within the
                //   current charmap.
                auto val = pContext.GetInteger(0);
                if (val.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #1 to `REVCHAR` must be an integer.");
                    return {};
                }

                for (const auto& [str, value] : pContext.mCharmap)
                {
                    if (value == *val)
                    {
                        return PreprocessorString { str };
                    }
                }

                pContext.mDiag.ReportError("Value '{}' not found in charmap.", *val);
                return {};
            }
        },
        {
            PreprocessorFunction::STRLEN,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `STRLEN(str)`
                // - Returns the number of characters in the string `str`.
                // - The length returned is in UTF-8 characters, excluding
                //   the null terminator.
                auto str = pContext.GetString(0);
                if (str.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #1 to `STRLEN` must be a string.");
                    return {};
                }

                std::size_t length = 0;
                for (std::size_t i = 0; i < str->length(); )
                {
                    auto c = (*str)[i];
                    if ((c & 0x80) == 0)            { i += 1; }
                    else if ((c & 0xE0) == 0xC0)    { i += 2; }
                    else if ((c & 0xF0) == 0xE0)    { i += 3; }
                    else                            { i += 4; }
                    length += 1;
                }

                return PreprocessorInteger { static_cast<std::int64_t>(length) };
            }
        },
        {
            PreprocessorFunction::STRCMP,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `STRCMP(str1, str2)`
                // - Compares strings `str1` and `str2`.
                // - Comparison is performed according to the ASCII ordering
                //   of each character.
                auto str1 = pContext.GetString(0);
                auto str2 = pContext.GetString(1);

                if (str1.has_value() == false || str2.has_value() == false)
                {
                    pContext.mDiag.ReportError("Both arguments to `STRCMP` must be strings.");
                    return {};
                }

                return PreprocessorInteger { static_cast<std::int64_t>(str1->compare(*str2)) };
            }
        },
        {
            PreprocessorFunction::STRNCMP,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `STRNCMP(str1, str2, n)`
                // - Compares up to `n` characters of strings `str1` and `str2`.
                // - Comparison is performed according to the ASCII ordering of each character.
                auto str1 = pContext.GetString(0);
                auto str2 = pContext.GetString(1);
                auto n = pContext.GetInteger(2);

                if (str1.has_value() == false || str2.has_value() == false || n.has_value() == false)
                {
                    pContext.mDiag.ReportError("All arguments to `STRNCMP` must be provided.");
                    return {};
                }

                if (*n < 0)
                {
                    pContext.mDiag.ReportError("Argument #3 to `STRNCMP` must be a non-negative integer.");
                    return {};
                }

                return PreprocessorInteger { 
                    static_cast<std::int64_t>(
                        str1->compare(0, static_cast<std::size_t>(*n), *str2, 0, static_cast<std::size_t>(*n))
                    )
                };
            }
        },
        {
            PreprocessorFunction::STRFIND,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `STRFIND(str, substr)`
                // - Returns the index of the first occurrence of `substr` in `str`.
                // - If `substr` is not found, returns -1.
                auto str = pContext.GetString(0);
                auto substr = pContext.GetString(1);

                if (str.has_value() == false || substr.has_value() == false)
                {
                    pContext.mDiag.ReportError("Both arguments to `STRFIND` must be strings.");
                    return {};
                }

                auto pos = str->find(*substr);
                return PreprocessorInteger { 
                    static_cast<std::int64_t>(pos != std::string::npos ? pos : -1) };
            }
        },
        {
            PreprocessorFunction::STRRFIND,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `STRRFIND(str, substr)`
                // - Returns the index of the last occurrence of `substr` in `str`.
                // - If `substr` is not found, returns -1.
                auto str = pContext.GetString(0);
                auto substr = pContext.GetString(1);

                if (str.has_value() == false || substr.has_value() == false)
                {
                    pContext.mDiag.ReportError("Both arguments to `STRRFIND` must be strings.");
                    return {};
                }

                auto pos = str->rfind(*substr);
                return PreprocessorInteger { 
                    static_cast<std::int64_t>(pos != std::string::npos ? pos : -1) };
            }
        },
        {
            PreprocessorFunction::BYTELEN,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `BYTELEN(str)`
                // - Returns the number of bytes in `str`.
                // - Non-ASCII characters can be multiple bytes.
                auto str = pContext.GetString(0);
                if (str.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #1 to `BYTELEN` must be a string.");
                    return {};
                }

                return PreprocessorInteger { static_cast<std::int64_t>(str->length()) };
            }
        },
        {
            PreprocessorFunction::STRBYTE,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `STRBYTE(str, index)`
                // - Returns the byte at the specified index in `str`.
                auto str = pContext.GetString(0);
                auto index = pContext.GetInteger(1);

                if (str.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #1 to `STRBYTE` must be a string.");
                    return {};
                }
                else if (index.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #2 to `STRBYTE` must be an integer.");
                    return {};
                }
                else if (*index < 0 || static_cast<std::size_t>(*index) >= str->length())
                {
                    pContext.mDiag.ReportError("Index out of bounds for `STRBYTE`.");
                    return {};
                }

                return PreprocessorInteger { static_cast<std::int64_t>((*str)[*index]) };
            }
        },
        {
            PreprocessorFunction::INCHARMAP,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `INCHARMAP(str)`
                // - Checks to see if `str` has an entry in the current charmap.
                // - Returns `1` if so, `0` otherwise.
                auto str = pContext.GetString(0);
                if (str.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #1 to `INCHARMAP` must be a string.");
                    return {};
                }

                auto findIt = std::find_if(
                    pContext.mCharmap.begin(), 
                    pContext.mCharmap.end(), 
                    [&str] (const auto& pair) -> bool
                    {
                    return pair.first == *str;
                    }
                );

                return PreprocessorInteger {
                    (findIt != pContext.mCharmap.end() ? 1 : 0)
                };
            }
        },
        {
            PreprocessorFunction::CHARLEN,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `CHARLEN(str)`
                // - Returns the number of charmap entries in `str` within
                //   the current charmap.
                auto str = pContext.GetString(0);
                if (str.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #1 to `CHARLEN` must be a string.");
                    return {};
                }

                auto substrs = pContext.ParseStringAgainstCharmap(*str);
                return PreprocessorInteger { static_cast<std::int64_t>(substrs.size()) };
            }
        },
        {
            PreprocessorFunction::CHARCMP,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `CHARCMP(str1, str2)`
                // - Compares strings `str1` and `str2`.
                // - Comparison is performed according to the charmap values in
                //   each string.
                auto str1 = pContext.GetString(0);
                auto str2 = pContext.GetString(1);

                if (str1.has_value() == false || str2.has_value() == false)
                {
                    pContext.mDiag.ReportError("Arguments to `CHARCMP` must be strings.");
                    return {};
                }
                
                auto parse1 = pContext.ParseStringAgainstCharmap(*str1);
                auto parse2 = pContext.ParseStringAgainstCharmap(*str2);
                for (std::size_t i = 0; i < std::min(parse1.size(), parse2.size()); ++i)
                {
                    auto findIt1 = std::find_if(
                        pContext.mCharmap.begin(),
                        pContext.mCharmap.end(),
                        [&parse1, i](const auto& pair) {
                            return pair.first == parse1[i];
                        }
                    );

                    auto findIt2 = std::find_if(
                        pContext.mCharmap.begin(),
                        pContext.mCharmap.end(),
                        [&parse2, i](const auto& pair) {
                            return pair.first == parse2[i];
                        }
                    );

                    if (findIt1 != pContext.mCharmap.end() && findIt2 != pContext.mCharmap.end())
                    {
                        for (uint8_t i = 0; i < 8; ++i)
                        {
                            std::uint8_t byte0 = ((findIt1->second >> (i * 8)) & 0xFF);
                            std::uint8_t byte1 = ((findIt2->second >> (i * 8)) & 0xFF);
                            if (byte0 < byte1)
                                { return PreprocessorInteger { -1 }; }
                            if (byte0 > byte1)
                                { return PreprocessorInteger { 1 }; }
                        }
                        
                        // if (findIt1->second < findIt2->second)
                        //     { return PreprocessorInteger { -1 }; }
                        // if (findIt1->second > findIt2->second)
                        //     { return PreprocessorInteger { 1 }; }
                    }
                }

                return PreprocessorInteger { 0 };
            }
        },
        {
            PreprocessorFunction::CHARSIZE,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `CHARSIZE(str)`
                // - Returns the size in bytes of the charmap entry for `str`.
                auto str = pContext.GetString(0);
                if (str.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #1 to `CHARSIZE` must be a string.");
                    return {};
                }

                auto findIt = std::find_if(
                    pContext.mCharmap.begin(),
                    pContext.mCharmap.end(),
                    [&str](const auto& pair) {
                        return pair.first == *str;
                    }
                );
                if (findIt == pContext.mCharmap.end())
                {
                    pContext.mDiag.ReportError("Character '{}' not found in charmap.", *str);
                    return {};
                }

                if (findIt->second > 0xFFFFFFFF) { return PreprocessorInteger { 8 }; }
                else if (findIt->second > 0xFFFF) { return PreprocessorInteger { 4 }; }
                else if (findIt->second > 0xFF) { return PreprocessorInteger { 2 }; }
                else { return PreprocessorInteger { 1 }; }
            }
        },
        {
            PreprocessorFunction::CHARVAL,
            [] (CallContext& pContext) -> PreprocessorValue
            {
                // `CHARVAL(str)`
                // - Returns the value of the charmap entry for `str`.
                auto str = pContext.GetString(0);
                if (str.has_value() == false)
                {
                    pContext.mDiag.ReportError("Argument #1 to `CHARVAL` must be a string.");
                    return {};
                }

                auto findIt = std::find_if(
                    pContext.mCharmap.begin(),
                    pContext.mCharmap.end(),
                    [&str](const auto& pair) {
                        return pair.first == *str;
                    }
                );
                if (findIt == pContext.mCharmap.end())
                {
                    if (str->length() == 1)
                    {
                        return PreprocessorInteger { static_cast<std::int64_t>((*str)[0]) };
                    }

                    pContext.mDiag.ReportError("Character '{}' not found in charmap.", *str);
                    return {};
                }

                return PreprocessorInteger { static_cast<std::int64_t>(findIt->second) };
            }
        }
    };

    auto Preprocessor::EvaluateBuiltinFunction (TokenCursor& pCursor,
        const PreprocessorFunction& pFunction) -> PreprocessorValue
    {
        if (
            pFunction == PreprocessorFunction::DEFINED ||
            pFunction == PreprocessorFunction::ISCONST ||
            pFunction == PreprocessorFunction::ISSYMBOL ||
            pFunction == PreprocessorFunction::ISSNIPPET ||
            pFunction == PreprocessorFunction::ISMACRO
        )
        {
            // Special case: These functions require raw identifiers.
            if (pCursor.ExpectNextToken(TokenType::LeftParenthesis).has_value() == false)
            {
                mDiag.ReportError("Expected '(' after function name.");
                return {};
            }

            auto id = pCursor.ExpectNextToken(TokenType::Identifier);
            if (id.has_value() == false)
            {
                mDiag.ReportError("Expected identifier after function name.");
                return {};
            }

            auto lexeme = InterpolateIdentifier(id->mLocation,
                id->Stringify().value_or(""));
            if (lexeme.has_value() == false)
            {
                mDiag.ReportError("Failed to interpolate identifier.");
                return {};
            }

            if (pCursor.ExpectNextToken(TokenType::RightParenthesis).has_value() == false)
            {
                mDiag.ReportError("Expected ')' after function argument.");
                return {};
            }

            switch (pFunction)
            {
                case PreprocessorFunction::DEFINED:
                    return PreprocessorInteger {
                        (mSymbols.contains(*lexeme) || mMacros.contains(*lexeme) || mSnippets.contains(*lexeme))
                            ? 1 : 0  
                    };
                case PreprocessorFunction::ISMACRO:
                    return PreprocessorInteger {
                        mMacros.contains(*lexeme) ? 1 : 0
                    };
                case PreprocessorFunction::ISSYMBOL:
                    return PreprocessorInteger {
                        mSymbols.contains(*lexeme) ? 1 : 0
                    };
                case PreprocessorFunction::ISSNIPPET:
                    return PreprocessorInteger {
                        mSnippets.contains(*lexeme) ? 1 : 0
                    };
                case PreprocessorFunction::ISCONST: {
                    auto findIt = mSymbols.find(*lexeme);
                    return PreprocessorInteger {
                        (findIt != mSymbols.end() && findIt->second.mIsConstant) 
                            ? 1 : 0
                    };
                } break;
            }
        }

        auto findIt = kBuiltinFunctions.find(pFunction);
        if (findIt == kBuiltinFunctions.end())
        {
            mDiag.ReportError("Unknown builtin function.");
            return {};
        }

        if (pCursor.ExpectNextToken(TokenType::LeftParenthesis).has_value() == false)
        {
            mDiag.ReportError("Expected '(' after function name.");
            return {};
        }

        CallContext ctx { mDiag, mCharmaps.at(mActiveCharmap), mIncludeDirs };
        while (pCursor.IsAtEnd() == false)
        {
            const auto& lead = pCursor.GetNextToken();
            auto val = EvaluateExpression(pCursor);
            if (val.IsUndefined() == true)
            {
                mDiag.ReportError(lead.mLocation, 
                    "Undefined value in function call.");
                return {};
            }

            ctx.mArgs.push_back(std::move(val));

            if (pCursor.ExpectNextToken(TokenType::Comma, false).has_value() == true)
            {
                pCursor.Skip();
                continue;
            }
            else if (pCursor.ExpectNextToken(TokenType::RightParenthesis, false).has_value() == true)
            {
                break;
            }
            else
            {
                mDiag.ReportError(lead.mLocation, "Expected ')' after function arguments.");
                return {};
            }
        }

        if (pCursor.ExpectNextToken(TokenType::RightParenthesis).has_value() == false)
        {
            const auto& tk = pCursor.GetNextToken();
            mDiag.ReportError(tk.mLocation, "Expected ')' after function arguments.");
            return {};
        }

        auto result = findIt->second(ctx);
        return result;
    }
}
