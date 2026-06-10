/**
 * @file    G10.ASM.Preprocessor.FixedPoint.hpp
 * @brief   Contains implementations for the G10 Assembler's fixed-point value
 *          class, and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.ASM.Common.hpp>

// Classes *********************************************************************

namespace G10::ASM
{
    class G10_API PreprocessorFixedPoint final
    {
    public: // Constructors & Destructor ***************************************

        inline PreprocessorFixedPoint () = default;
        inline explicit PreprocessorFixedPoint (double pValue)
            { ComputeFromFloat(pValue); }
        inline explicit PreprocessorFixedPoint (std::int32_t pInteger, 
            std::uint32_t pFractional)
                { ComputeFromParts(pInteger, pFractional); }

    public: // Methods - Computation *******************************************

        inline auto ComputeFromParts (std::int32_t pInteger, 
            std::uint32_t pFractional) -> void
        {
            mInteger = pInteger;
            mFractional = pFractional;
            mComputed = static_cast<double>(mInteger) + (
                static_cast<double>(mFractional) / 
                    static_cast<double>(std::numeric_limits<std::uint32_t>::max())
            );
            mRaw = (static_cast<std::uint64_t>(mInteger) << 32) | mFractional;
        }

        inline auto ComputeFromRaw (std::uint64_t pRaw) -> void
        {
            std::int32_t integer = static_cast<std::int32_t>(pRaw >> 32);
            std::uint32_t fractional = 
                static_cast<std::uint32_t>(pRaw & 0xFFFFFFFF);

            ComputeFromParts(integer, fractional);
        }

        inline auto ComputeFromFloat (double pValue) -> void
        {
            double integer = 0.0;
            double fractional = std::modf(pValue, &integer);

            ComputeFromParts(
                static_cast<std::int32_t>(integer),
                static_cast<std::uint32_t>(
                    fractional * std::numeric_limits<std::uint32_t>::max()
                )
            ); 
        }

        inline auto Correct () const -> PreprocessorFixedPoint
        {
            static constexpr std::uint64_t kFractionalEpsilon = 0x47;
            std::uint64_t integer = mInteger;
            std::uint64_t fractional = mFractional + kFractionalEpsilon;
            
            if (fractional > 0xFFFFFFFF)
            {
                integer++;
            }
            
            return PreprocessorFixedPoint {
                static_cast<std::int32_t>(integer & 0xFFFFFFFF),
                static_cast<std::uint32_t>(fractional & 0xFFFFFFFF)
            };
        }

    public: // Methods - Accessors *********************************************

        inline auto GetRaw () const -> std::uint64_t
            { return mRaw; }
        inline auto GetComputed () const -> double
            { return mComputed; }
        inline auto GetInteger () const -> std::int32_t
            { return mInteger; }
        inline auto GetUnsignedInteger () const -> std::uint32_t
            { return static_cast<std::uint32_t>(mInteger); }
        inline auto GetFractional () const -> std::uint32_t
            { return mFractional; }

    public: // Operators *******************************************************

        inline auto operator= (double pValue) -> PreprocessorFixedPoint&
            { ComputeFromFloat(pValue); return *this; }
        inline operator double () const
            { return mComputed; }
        inline operator bool () const
            { return (mInteger != 0 || mFractional != 0); }

    private: // Members ********************************************************

        std::uint64_t   mRaw { 0 };
        std::int32_t    mInteger { 0 };
        std::uint32_t   mFractional { 0 };
        double          mComputed { 0.0 };

    };
}
