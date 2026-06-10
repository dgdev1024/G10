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

    };

    static const std::unordered_map<
        PreprocessorFunction,
        std::function<PreprocessorValue(CallContext&)>
    > kBuiltinFunctions = 
    {
        { 
            PreprocessorFunction::HIGH,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetInteger(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected integer argument.");
                    return {};
                }

                if ((*arg1 & 0xFFFFFFFF00000000) != 0)
                    { return PreprocessorInteger { (*arg1 >> 32) & 0xFFFFFFFF }; }
                else if ((*arg1 & 0xFFFF0000) != 0)
                    { return PreprocessorInteger { (*arg1 >> 16) & 0xFFFF }; }
                else if ((*arg1 & 0xFF00) != 0)
                    { return PreprocessorInteger { (*arg1 >> 8) & 0xFF }; }
                else
                    { return PreprocessorInteger { (*arg1 >> 4) & 0xF }; }
            }
        },
        {
            PreprocessorFunction::LOW,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetInteger(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected integer argument.");
                    return {};
                }

                if ((*arg1 & 0xFFFFFFFF00000000) != 0)
                    { return PreprocessorInteger { (*arg1 & 0xFFFFFFFF) }; }
                else if ((*arg1 & 0xFFFF0000) != 0)
                    { return PreprocessorInteger { (*arg1 & 0xFFFF) }; }
                else if ((*arg1 & 0xFF00) != 0)
                    { return PreprocessorInteger { (*arg1 & 0xFF) }; }
                else
                    { return PreprocessorInteger { (*arg1 & 0xF) }; }
            }
        },
        {
            PreprocessorFunction::HIDWORD,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetInteger(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected integer argument.");
                    return {};
                }

                return PreprocessorInteger { (*arg1 >> 32) & 0xFFFFFFFF };
            }
        },
        {
            PreprocessorFunction::LODWORD,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetInteger(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected integer argument.");
                    return {};
                }

                return PreprocessorInteger { *arg1 & 0xFFFFFFFF };
            }
        },
        {
            PreprocessorFunction::HIWORD,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetInteger(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected integer argument.");
                    return {};
                }

                return PreprocessorInteger { (*arg1 >> 16) & 0xFFFF };
            }
        },
        {
            PreprocessorFunction::LOWORD,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetInteger(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected integer argument.");
                    return {};
                }

                return PreprocessorInteger { *arg1 & 0xFFFF };
            }
        },
        {
            PreprocessorFunction::HIBYTE,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetInteger(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected integer argument.");
                    return {};
                }

                return PreprocessorInteger { (*arg1 >> 8) & 0xFF };
            }
        },
        {
            PreprocessorFunction::LOBYTE,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetInteger(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected integer argument.");
                    return {};
                }

                return PreprocessorInteger { *arg1 & 0xFF };
            }
        },
        {
            PreprocessorFunction::HINIBBLE,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetInteger(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected integer argument.");
                    return {};
                }

                return PreprocessorInteger { (*arg1 >> 4) & 0xF };
            }
        },
        {
            PreprocessorFunction::LONIBBLE,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetInteger(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected integer argument.");
                    return {};
                }

                return PreprocessorInteger { *arg1 & 0xF };
            }
        },
        {
            PreprocessorFunction::BITWIDTH,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetInteger(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected integer argument.");
                    return {};
                }

                return PreprocessorInteger { 
                    std::bit_width(static_cast<std::uint64_t>(*arg1))
                };
            }
        },
        {
            PreprocessorFunction::TZCOUNT,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetInteger(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected integer argument.");
                    return {};
                }

                return PreprocessorInteger { 
                    std::countr_zero(static_cast<std::uint64_t>(*arg1))
                };
            }
        },
        {
            PreprocessorFunction::FINT,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                return PreprocessorInteger { arg1->GetInteger() };
            }
        },
        {
            PreprocessorFunction::FFRAC,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                return PreprocessorInteger { arg1->GetFractional() };
            }
        },
        {
            PreprocessorFunction::FADD,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                auto arg2 = pCtx.GetFixedPoint(1);
                if (arg1.has_value() == false || arg2.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point arguments.");
                    return {};
                }

                return PreprocessorFixedPoint { arg1->GetComputed() + arg2->GetComputed() };
            }
        },
        {
            PreprocessorFunction::FSUB,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                auto arg2 = pCtx.GetFixedPoint(1);
                if (arg1.has_value() == false || arg2.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point arguments.");
                    return {};
                }

                return PreprocessorFixedPoint { arg1->GetComputed() - arg2->GetComputed() };
            }
        },
        {
            PreprocessorFunction::FMUL,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                auto arg2 = pCtx.GetFixedPoint(1);
                if (arg1.has_value() == false || arg2.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point arguments.");
                    return {};
                }

                return PreprocessorFixedPoint { arg1->GetComputed() * arg2->GetComputed() };
            }
        },
        {
            PreprocessorFunction::FDIV,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                auto arg2 = pCtx.GetFixedPoint(1);
                if (arg1.has_value() == false || arg2.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point arguments.");
                    return {};
                }
                else if (arg2->GetInteger() == 0 && arg2->GetFractional() == 0)
                {
                    pCtx.mDiag.ReportError("Division by zero.");
                    return {};
                }

                return PreprocessorFixedPoint { arg1->GetComputed() / arg2->GetComputed() };
            }
        },
        {
            PreprocessorFunction::FMOD,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                auto arg2 = pCtx.GetFixedPoint(1);
                if (arg1.has_value() == false || arg2.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point arguments.");
                    return {};
                }
                else if (arg2->GetInteger() == 0 && arg2->GetFractional() == 0)
                {
                    pCtx.mDiag.ReportError("Modulo by zero.");
                    return {};
                }

                return PreprocessorFixedPoint { std::fmod(arg1->GetComputed(), 
                    arg2->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FPOW,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                auto arg2 = pCtx.GetFixedPoint(1);
                if (arg1.has_value() == false || arg2.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point arguments.");
                    return {};
                }

                return PreprocessorFixedPoint { std::pow(arg1->GetComputed(), 
                    arg2->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FSQRT,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                return PreprocessorFixedPoint { std::sqrt(arg1->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FROOT,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                auto arg2 = pCtx.GetFixedPoint(1);
                if (arg1.has_value() == false || arg2.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point arguments.");
                    return {};
                }

                return PreprocessorFixedPoint { std::pow(arg1->GetComputed(), 
                    1.0 / arg2->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FLOG,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                auto arg2 = pCtx.GetFixedPoint(1);
                if (arg1.has_value() == false || arg2.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point arguments.");
                    return {};
                }

                return PreprocessorFixedPoint { std::log(arg1->GetComputed()) / 
                    std::log(arg2->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FLN,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                return PreprocessorFixedPoint { std::log(arg1->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FROUND,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                return PreprocessorFixedPoint { std::round(arg1->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FCEIL,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                return PreprocessorFixedPoint { std::ceil(arg1->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FFLOOR,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                return PreprocessorFixedPoint { std::floor(arg1->GetComputed()) };
            }
        },
        {
            PreprocessorFunction::FRADT,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                return PreprocessorFixedPoint { arg1->GetComputed() * kTurn };
            }
        },
        {
            PreprocessorFunction::FDEGT,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                // To radians, first, then to turns.
                double radians = arg1->GetComputed() * (std::numbers::pi / 180.0);
                return PreprocessorFixedPoint { radians / kTurn };
            }
        },
        {
            PreprocessorFunction::FSIN,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                // Angle expected in turns. 1 turn = 2PI radians or 360 degrees.
                return PreprocessorFixedPoint { 
                    std::sin(arg1->GetComputed() * kTurn) };
            }
        },
        {
            PreprocessorFunction::FCOS,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                // Angle expected in turns. 1 turn = 2PI radians or 360 degrees.
                return PreprocessorFixedPoint { 
                    std::cos(arg1->GetComputed() * kTurn) };
            }
        },
        {
            PreprocessorFunction::FTAN,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                // Angle expected in turns. 1 turn = 2PI radians or 360 degrees.
                return PreprocessorFixedPoint { 
                    std::tan(arg1->GetComputed() * kTurn) };
            }
        },
        {
            PreprocessorFunction::FASIN,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                return PreprocessorFixedPoint { 
                    std::asin(arg1->GetComputed()) / kTurn };
            }
        },
        {
            PreprocessorFunction::FACOS,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                return PreprocessorFixedPoint { 
                    std::acos(arg1->GetComputed()) / kTurn };
            }
        },
        {
            PreprocessorFunction::FATAN,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point argument.");
                    return {};
                }

                return PreprocessorFixedPoint { 
                    std::atan(arg1->GetComputed()) / kTurn };
            }
        },
        {
            PreprocessorFunction::FATAN2,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetFixedPoint(0);
                auto arg2 = pCtx.GetFixedPoint(1);
                if (arg1.has_value() == false || arg2.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected fixed-point arguments.");
                    return {};
                }

                return PreprocessorFixedPoint { 
                    std::atan2(arg1->GetComputed(), arg2->GetComputed()) / kTurn };
            }
        },
        {
            PreprocessorFunction::STRLEN,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected string argument.");
                    return {};
                }

                return PreprocessorInteger { static_cast<std::int64_t>(arg1->length()) };
            }
        },
        {
            PreprocessorFunction::BYTELEN,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected string argument.");
                    return {};
                }

                // Same as `STRLEN`, but with null terminator.
                return PreprocessorInteger { static_cast<std::int64_t>(arg1->length()) + 1 };
            }
        },
        {
            PreprocessorFunction::STRCAT,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                auto arg2 = pCtx.GetString(1);
                if (arg1.has_value() == false || arg2.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected string arguments.");
                    return {};
                }

                return PreprocessorString { *arg1 + *arg2 };
            }
        },
        {
            PreprocessorFunction::STRNCAT,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                auto arg2 = pCtx.GetString(1);
                auto arg3 = pCtx.GetInteger(2);
                if (
                    arg1.has_value() == false ||
                    arg2.has_value() == false ||
                    arg3.has_value() == false
                )
                {
                    pCtx.mDiag.ReportError("Received invalid argument(s).");
                    return {};
                }

                return PreprocessorString { *arg1 + arg2->substr(0, *arg3) };
            }
        },
        {
            PreprocessorFunction::STRUPR,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected string argument.");
                    return {};
                }

                std::transform(arg1->begin(), arg1->end(), arg1->begin(), 
                    ::toupper);
                return PreprocessorString { *arg1 };
            }
        },
        {
            PreprocessorFunction::STRLWR,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                if (arg1.has_value() == false)
                {
                    pCtx.mDiag.ReportError("Expected string argument.");
                    return {};
                }

                std::transform(arg1->begin(), arg1->end(), arg1->begin(), 
                    ::tolower);
                return PreprocessorString { *arg1 };
            }
        },
        {
            PreprocessorFunction::STRSLICE,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                auto arg2 = pCtx.GetInteger(1);
                auto arg3 = pCtx.GetInteger(2);
                if (
                    arg1.has_value() == false ||
                    arg2.has_value() == false ||
                    arg3.has_value() == false
                )
                {
                    pCtx.mDiag.ReportError("Received invalid argument(s).");
                    return {};
                }

                return PreprocessorString { arg1->substr(*arg2, *arg3) };
            }
        },
        {
            PreprocessorFunction::STRREPLACE,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                auto arg2 = pCtx.GetString(1);
                auto arg3 = pCtx.GetString(2);
                if (
                    arg1.has_value() == false ||
                    arg2.has_value() == false ||
                    arg3.has_value() == false
                )
                {
                    pCtx.mDiag.ReportError("Received invalid argument(s).");
                    return {};
                }

                std::string result = *arg1;
                size_t pos = result.find(*arg2);
                if (pos != std::string::npos)
                {
                    result.replace(pos, arg2->length(), *arg3);
                }
                return PreprocessorString { result };
            }
        },
        {
            PreprocessorFunction::STRCMP,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                auto arg2 = pCtx.GetString(1);
                if (
                    arg1.has_value() == false ||
                    arg2.has_value() == false
                )
                {
                    pCtx.mDiag.ReportError("Received invalid argument(s).");
                    return {};
                }

                return PreprocessorInteger { arg1->compare(*arg2) };
            }
        },
        {
            PreprocessorFunction::STRNCMP,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                auto arg2 = pCtx.GetString(1);
                auto arg3 = pCtx.GetInteger(2);
                if (
                    arg1.has_value() == false ||
                    arg2.has_value() == false ||
                    arg3.has_value() == false
                )
                {
                    pCtx.mDiag.ReportError("Received invalid argument(s).");
                    return {};
                }

                return PreprocessorInteger { arg1->compare(0, *arg3, *arg2) };
            }
        },
        {
            PreprocessorFunction::STRFIND,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                auto arg2 = pCtx.GetString(1);
                if (
                    arg1.has_value() == false ||
                    arg2.has_value() == false
                )
                {
                    pCtx.mDiag.ReportError("Received invalid argument(s).");
                    return {};
                }

                size_t pos = arg1->find(*arg2);
                if (pos == std::string::npos)
                {
                    return PreprocessorInteger { -1 };
                }

                return PreprocessorInteger { static_cast<int>(pos) };
            }
        },
        {
            PreprocessorFunction::STRNFIND,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                auto arg2 = pCtx.GetString(1);
                auto arg3 = pCtx.GetInteger(2);
                if (
                    arg1.has_value() == false ||
                    arg2.has_value() == false ||
                    arg3.has_value() == false
                )
                {
                    pCtx.mDiag.ReportError("Received invalid argument(s).");
                    return {};
                }

                size_t pos = arg1->substr(0, *arg3).find(*arg2);
                if (pos == std::string::npos)
                {
                    return PreprocessorInteger { -1 };
                }

                return PreprocessorInteger { static_cast<int>(pos) };
            }
        },
        {
            PreprocessorFunction::STRRFIND,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                auto arg2 = pCtx.GetString(1);
                if (
                    arg1.has_value() == false ||
                    arg2.has_value() == false
                )
                {
                    pCtx.mDiag.ReportError("Received invalid argument(s).");
                    return {};
                }

                size_t pos = arg1->find_last_of(*arg2);
                if (pos == std::string::npos)
                {
                    return PreprocessorInteger { -1 };
                }

                return PreprocessorInteger { static_cast<int>(pos) };
            }
        },
        {
            PreprocessorFunction::STRNRFIND,
            [] (CallContext& pCtx) -> PreprocessorValue
            {
                auto arg1 = pCtx.GetString(0);
                auto arg2 = pCtx.GetString(1);
                auto arg3 = pCtx.GetInteger(2);
                if (
                    arg1.has_value() == false ||
                    arg2.has_value() == false ||
                    arg3.has_value() == false
                )
                {
                    pCtx.mDiag.ReportError("Received invalid argument(s).");
                    return {};
                }

                size_t pos = arg1->substr(0, *arg3).find_last_of(*arg2);
                if (pos == std::string::npos)
                {
                    return PreprocessorInteger { -1 };
                }

                return PreprocessorInteger { static_cast<int>(pos) };
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
                        (mSymbols.contains(*lexeme) || mMacros.contains(*lexeme))
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

        CallContext ctx { mDiag };
        while (pCursor.IsAtEnd() == false)
        {
            auto val = EvaluateExpression(pCursor);
            if (val.IsUndefined() == true)
            {
                mDiag.ReportError("Undefined value in function call.");
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
                mDiag.ReportError("Expected ')' after function arguments.");
                return {};
            }
        }

        if (pCursor.ExpectNextToken(TokenType::RightParenthesis).has_value() == false)
        {
            mDiag.ReportError("Expected ')' after function arguments.");
            return {};
        }

        return findIt->second(ctx);
    }
}
