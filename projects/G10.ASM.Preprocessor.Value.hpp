/**
 * @file    G10.ASM.Preprocessor.Value.hpp
 * @brief   Contains implementations for the G10 Assembler Preprocessor's
 *          value class, and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.ASM.Preprocessor.FixedPoint.hpp>

// Macros **********************************************************************

#define PPI(pVal)   PreprocessorValue { PreprocessorInteger     { (pVal) }}
#define PPFP(pVal)  PreprocessorValue { PreprocessorFixedPoint  { (pVal) }}
#define PPS(pVal)   PreprocessorValue { PreprocessorString      { (pVal) }}
#define DBL(pVal)   static_cast<double>((pVal))
#define APX(pLeft, pRight) (std::fabs(DBL(pLeft) - DBL(pRight)) < \
    std::numeric_limits<double>::epsilon())

// Types ***********************************************************************

namespace G10::ASM
{
    using PreprocessorUndefined = std::monostate;
    using PreprocessorInteger   = std::int64_t;
    using PreprocessorString    = std::string;

    template <typename T>
    concept PreprocessorValueTypename = (
        std::same_as<T, PreprocessorUndefined> ||
        std::same_as<T, PreprocessorInteger> ||
        std::same_as<T, PreprocessorFixedPoint> ||
        std::same_as<T, PreprocessorString>
    );
}

// Constants & Enumerations ****************************************************

namespace G10::ASM
{
    enum class PreprocessorValueType
    {
        Undefined,
        Integer,
        FixedPoint,
        String
    };
}

// Classes *********************************************************************

namespace G10::ASM
{
    class G10_API PreprocessorValue final
    {
    public: // Constructors & Destructor ***************************************

        PreprocessorValue () = default;
        PreprocessorValue (const PreprocessorValue&) = default;
        PreprocessorValue (PreprocessorValue&&) = default;
        ~PreprocessorValue () = default;

        template <PreprocessorValueTypename T>
        inline PreprocessorValue (T pValue) :
            mVariant { std::move(pValue) } {}

    public: // Methods *********************************************************

        inline auto GetType () const -> PreprocessorValueType
        {
            return std::visit(stx::overload {
                [] (const PreprocessorUndefined&)   { return PreprocessorValueType::Undefined; },
                [] (const PreprocessorInteger&)     { return PreprocessorValueType::Integer; },
                [] (const PreprocessorFixedPoint&)  { return PreprocessorValueType::FixedPoint; },
                [] (const PreprocessorString&)      { return PreprocessorValueType::String; }
            }, mVariant);
        }

        template <PreprocessorValueTypename T>
        inline auto Is () const -> bool
            { return std::holds_alternative<T>(mVariant); }
        inline auto IsUndefined () const -> bool
            { return Is<PreprocessorUndefined>(); }
        inline auto IsInteger () const -> bool
            { return Is<PreprocessorInteger>(); }
        inline auto IsFixedPoint () const -> bool
            { return Is<PreprocessorFixedPoint>(); }
        inline auto IsString () const -> bool
            { return Is<PreprocessorString>(); }

        inline auto IsNumeric () const -> bool
        {
            return
                Is<PreprocessorInteger>() ||
                Is<PreprocessorFixedPoint>();
        }

        inline auto IsTruthy () const -> bool
        {
            return std::visit(stx::overload {
                [] (const PreprocessorUndefined&)  
                    { return false; },
                [] (const PreprocessorInteger& pInteger)    
                    { return (pInteger != 0); },
                [] (const PreprocessorFixedPoint& pNumber) 
                    { return (pNumber.GetInteger() != 0 || pNumber.GetFractional() != 0); },
                [] (const PreprocessorString& pString)     
                    { return (pString.empty() == false); }
            }, mVariant);
        }

        template <PreprocessorValueTypename T>
        inline auto Get () -> T*
            { return std::get_if<T>(&mVariant); }
        inline auto GetInteger () -> PreprocessorInteger*
            { return Get<PreprocessorInteger>(); }
        inline auto GetFixedPoint () -> PreprocessorFixedPoint*
            { return Get<PreprocessorFixedPoint>(); }
        inline auto GetString () -> PreprocessorString*
            { return Get<PreprocessorString>(); }

        template <PreprocessorValueTypename T>
        inline auto Get () const -> const T*
            { return std::get_if<T>(&mVariant); }
        inline auto GetInteger () const -> const PreprocessorInteger*
            { return Get<PreprocessorInteger>(); }
        inline auto GetFixedPoint () const -> const PreprocessorFixedPoint*
            { return Get<PreprocessorFixedPoint>(); }
        inline auto GetString () const -> const PreprocessorString*
            { return Get<PreprocessorString>(); }

        template <PreprocessorValueTypename T>
        inline auto GetOr (const T& pDefault = {}) const -> T
        {
            if (auto ptr = Get<T>())
                { return *ptr; }

            return pDefault;
        }

    public: // Operators *******************************************************

        auto operator= (const PreprocessorValue&) -> PreprocessorValue& = default;
        auto operator= (PreprocessorValue&&) -> PreprocessorValue& = default;

        template <PreprocessorValueTypename T>
        inline auto operator= (const T& pValue) -> PreprocessorValue&
            { mVariant = pValue; return *this; }

    public: // Operators - Unary ***********************************************

        inline auto operator+ () const 
            -> PreprocessorValue
        {
            if (IsInteger())
                { return PPI(+(*GetInteger())); }
            else if (IsFixedPoint())
                { return PPFP(+(GetFixedPoint()->GetComputed())); }
            else
                { return {}; }
        }

        inline auto operator- () const 
            -> PreprocessorValue
        {
            if (IsInteger())
                { return PPI(-(*GetInteger())); }
            else if (IsFixedPoint())
                { return PPFP(-(GetFixedPoint()->GetComputed())); }
            else
                { return {}; }
        }

        inline auto operator~ () const 
            -> PreprocessorValue
        {
            if (IsInteger())
                { return PPI(~(*GetInteger())); }
            else
                { return {}; }
        }

        inline auto operator! () const 
            -> PreprocessorValue
        {
            return PPI(IsTruthy() ? 0 : 1);
        }

    public: // Operators - Binary Arithmetic ***********************************

        inline auto operator+ (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI(*GetInteger() + *pRight.GetInteger()); }
            else if (IsFixedPoint() && pRight.IsFixedPoint())
                { return PPFP(GetFixedPoint()->GetComputed() + 
                    pRight.GetFixedPoint()->GetComputed()); }
            else if (IsInteger() && pRight.IsFixedPoint())
                { return PPFP(DBL(*GetInteger()) + 
                    pRight.GetFixedPoint()->GetComputed()); }
            else if (IsFixedPoint() && pRight.IsInteger())
                { return PPFP(GetFixedPoint()->GetComputed() +
                    DBL(*pRight.GetInteger())); }
            else if (IsString() && pRight.IsString())
                { return PPS(*GetString() + *pRight.GetString()); }
            else { return {}; }
        }

        inline auto operator- (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI(*GetInteger() - *pRight.GetInteger()); }
            else if (IsFixedPoint() && pRight.IsFixedPoint())
                { return PPFP(GetFixedPoint()->GetComputed() - 
                    pRight.GetFixedPoint()->GetComputed()); }
            else if (IsInteger() && pRight.IsFixedPoint())
                { return PPFP(DBL(*GetInteger()) - 
                    pRight.GetFixedPoint()->GetComputed()); }
            else if (IsFixedPoint() && pRight.IsInteger())
                { return PPFP(GetFixedPoint()->GetComputed() -
                    DBL(*pRight.GetInteger())); }
            else { return {}; }
        }

        inline auto operator* (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI(*GetInteger() * *pRight.GetInteger()); }
            else if (IsFixedPoint() && pRight.IsFixedPoint())
                { return PPFP(GetFixedPoint()->GetComputed() * 
                    pRight.GetFixedPoint()->GetComputed()); }
            else if (IsInteger() && pRight.IsFixedPoint())
                { return PPFP(DBL(*GetInteger()) * 
                    pRight.GetFixedPoint()->GetComputed()); }
            else if (IsFixedPoint() && pRight.IsInteger())
                { return PPFP(GetFixedPoint()->GetComputed() *
                    DBL(*pRight.GetInteger())); }
            else { return {}; }
        }

        inline auto operator/ (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (pRight.IsTruthy() == false)
                { return {}; }

            if (IsInteger() && pRight.IsInteger())
                { return PPI(*GetInteger() / *pRight.GetInteger()); }
            else if (IsFixedPoint() && pRight.IsFixedPoint())
                { return PPFP(GetFixedPoint()->GetComputed() / 
                    pRight.GetFixedPoint()->GetComputed()); }
            else if (IsInteger() && pRight.IsFixedPoint())
                { return PPFP(DBL(*GetInteger()) / 
                    pRight.GetFixedPoint()->GetComputed()); }
            else if (IsFixedPoint() && pRight.IsInteger())
                { return PPFP(GetFixedPoint()->GetComputed() /
                    DBL(*pRight.GetInteger())); }
            else { return {}; }
        }

        inline auto operator% (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (pRight.IsTruthy() == false)
                { return {}; }

            if (IsInteger() && pRight.IsInteger())
                { return PPI(*GetInteger() % *pRight.GetInteger()); }
            else if (IsFixedPoint() && pRight.IsFixedPoint())
                { return PPFP(std::fmod(GetFixedPoint()->GetComputed(),
                    pRight.GetFixedPoint()->GetComputed())); }
            else if (IsInteger() && pRight.IsFixedPoint())
                { return PPFP(std::fmod(DBL(*GetInteger()),
                    pRight.GetFixedPoint()->GetComputed())); }
            else if (IsFixedPoint() && pRight.IsInteger())
                { return PPFP(std::fmod(GetFixedPoint()->GetComputed(),
                    DBL(*pRight.GetInteger()))); }
            else { return {}; }
        }

        inline auto operator+= (const PreprocessorValue& pRight)
            -> PreprocessorValue&
                { *this = *this + pRight; return *this; }
        inline auto operator-= (const PreprocessorValue& pRight)
            -> PreprocessorValue&
                { *this = *this - pRight; return *this; }
        inline auto operator*= (const PreprocessorValue& pRight)
            -> PreprocessorValue&
                { *this = *this * pRight; return *this; }
        inline auto operator/= (const PreprocessorValue& pRight)
            -> PreprocessorValue&
                { *this = *this / pRight; return *this; }
        inline auto operator%= (const PreprocessorValue& pRight)
            -> PreprocessorValue&
                { *this = *this % pRight; return *this; }

    public: // Operators - Binary Bitwise **************************************

        inline auto operator& (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI(*GetInteger() & *pRight.GetInteger()); }
            else { return {}; }
        }

        inline auto operator| (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI(*GetInteger() | *pRight.GetInteger()); }
            else { return {}; }
        }

        inline auto operator^ (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI(*GetInteger() ^ *pRight.GetInteger()); }
            else { return {}; }
        }

        inline auto operator<< (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI(*GetInteger() << *pRight.GetInteger()); }
            else { return {}; }
        }

        inline auto operator>> (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI(*GetInteger() >> *pRight.GetInteger()); }
            else { return {}; }
        }

        inline auto operator&= (const PreprocessorValue& pRight)
            -> PreprocessorValue&
                { *this = *this & pRight; return *this; }
        inline auto operator|= (const PreprocessorValue& pRight)
            -> PreprocessorValue&
                { *this = *this | pRight; return *this; }
        inline auto operator^= (const PreprocessorValue& pRight)
            -> PreprocessorValue&
                { *this = *this ^ pRight; return *this; }
        inline auto operator<<= (const PreprocessorValue& pRight)
            -> PreprocessorValue&
                { *this = *this << pRight; return *this; }
        inline auto operator>>= (const PreprocessorValue& pRight)
            -> PreprocessorValue&
                { *this = *this >> pRight; return *this; }

    public: // Operators - Binary Comparison ***********************************

        inline auto operator== (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI((*GetInteger() == *pRight.GetInteger()) ? 1 : 0); }
            else if (IsFixedPoint() && pRight.IsFixedPoint())
                { return PPI(APX(GetFixedPoint()->GetComputed(),
                    pRight.GetFixedPoint()->GetComputed()) ? 1 : 0); }
            else if (IsInteger() && pRight.IsFixedPoint())
                { return PPI(APX(*GetInteger(),
                    pRight.GetFixedPoint()->GetComputed()) ? 1 : 0); }
            else if (IsFixedPoint() && pRight.IsInteger())
                { return PPI(APX(GetFixedPoint()->GetComputed(),
                    *pRight.GetInteger()) ? 1 : 0); }
            else if (IsString() && pRight.IsString())
                { return PPI((*GetString() == *pRight.GetString()) ? 1 : 0); }
            else { return {}; }
        }

        inline auto operator!= (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI((*GetInteger() == *pRight.GetInteger()) ? 0 : 1); }
            else if (IsFixedPoint() && pRight.IsFixedPoint())
                { return PPI(APX(GetFixedPoint()->GetComputed(),
                    pRight.GetFixedPoint()->GetComputed()) ? 0 : 1); }
            else if (IsInteger() && pRight.IsFixedPoint())
                { return PPI(APX(*GetInteger(),
                    pRight.GetFixedPoint()->GetComputed()) ? 0 : 1); }
            else if (IsFixedPoint() && pRight.IsInteger())
                { return PPI(APX(GetFixedPoint()->GetComputed(),
                    *pRight.GetInteger()) ? 0 : 1); }
            else if (IsString() && pRight.IsString())
                { return PPI((*GetString() == *pRight.GetString()) ? 0 : 1); }
            else { return {}; }
        }

        inline auto operator< (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI((*GetInteger() < *pRight.GetInteger()) ? 1 : 0); }
            else if (IsFixedPoint() && pRight.IsFixedPoint())
                { return PPI((GetFixedPoint()->GetComputed() <
                    pRight.GetFixedPoint()->GetComputed()) ? 1 : 0); }
            else if (IsInteger() && pRight.IsFixedPoint())
                { return PPI((DBL(*GetInteger()) <
                    pRight.GetFixedPoint()->GetComputed()) ? 1 : 0); }
            else if (IsFixedPoint() && pRight.IsInteger())
                { return PPI((GetFixedPoint()->GetComputed() <
                    DBL(*pRight.GetInteger())) ? 1 : 0); }
            else if (IsString() && pRight.IsString())
                { return PPI((*GetString() < *pRight.GetString()) ? 1 : 0); }
            else { return {}; }
        }

        inline auto operator<= (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI((*GetInteger() <= *pRight.GetInteger()) ? 1 : 0); }
            else if (IsFixedPoint() && pRight.IsFixedPoint())
                { return PPI((GetFixedPoint()->GetComputed() <=
                    pRight.GetFixedPoint()->GetComputed()) ? 1 : 0); }
            else if (IsInteger() && pRight.IsFixedPoint())
                { return PPI((DBL(*GetInteger()) <=
                    pRight.GetFixedPoint()->GetComputed()) ? 1 : 0); }
            else if (IsFixedPoint() && pRight.IsInteger())
                { return PPI((GetFixedPoint()->GetComputed() <=
                    DBL(*pRight.GetInteger())) ? 1 : 0); }
            else if (IsString() && pRight.IsString())
                { return PPI((*GetString() <= *pRight.GetString()) ? 1 : 0); }
            else { return {}; }
        }

        inline auto operator> (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI((*GetInteger() > *pRight.GetInteger()) ? 1 : 0); }
            else if (IsFixedPoint() && pRight.IsFixedPoint())
                { return PPI((GetFixedPoint()->GetComputed() >
                    pRight.GetFixedPoint()->GetComputed()) ? 1 : 0); }
            else if (IsInteger() && pRight.IsFixedPoint())
                { return PPI((DBL(*GetInteger()) >
                    pRight.GetFixedPoint()->GetComputed()) ? 1 : 0); }
            else if (IsFixedPoint() && pRight.IsInteger())
                { return PPI((GetFixedPoint()->GetComputed() >
                    DBL(*pRight.GetInteger())) ? 1 : 0); }
            else if (IsString() && pRight.IsString())
                { return PPI((*GetString() > *pRight.GetString()) ? 1 : 0); }
            else { return {}; }
        }

        inline auto operator>= (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            if (IsInteger() && pRight.IsInteger())
                { return PPI((*GetInteger() >= *pRight.GetInteger()) ? 1 : 0); }
            else if (IsFixedPoint() && pRight.IsFixedPoint())
                { return PPI((GetFixedPoint()->GetComputed() >=
                    pRight.GetFixedPoint()->GetComputed()) ? 1 : 0); }
            else if (IsInteger() && pRight.IsFixedPoint())
                { return PPI((DBL(*GetInteger()) >=
                    pRight.GetFixedPoint()->GetComputed()) ? 1 : 0); }
            else if (IsFixedPoint() && pRight.IsInteger())
                { return PPI((GetFixedPoint()->GetComputed() >=
                    DBL(*pRight.GetInteger())) ? 1 : 0); }
            else if (IsString() && pRight.IsString())
                { return PPI((*GetString() >= *pRight.GetString()) ? 1 : 0); }
            else { return {}; }
        }

    public: // Operators - Binary Logical **************************************

        inline auto operator&& (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            return PPI((IsTruthy() && pRight.IsTruthy()) ? 1 : 0);
        }

        inline auto operator|| (const PreprocessorValue& pRight) const
            -> PreprocessorValue
        {
            return PPI((IsTruthy() || pRight.IsTruthy()) ? 1 : 0);
        }

    public: // Operators - Conversion ******************************************

        inline operator bool () const
            { return IsTruthy(); }
        inline operator PreprocessorInteger () const
            { return GetOr<PreprocessorInteger>(); }
        inline operator PreprocessorFixedPoint () const
            { return GetOr<PreprocessorFixedPoint>(); }
        inline operator PreprocessorString () const
            { return GetOr<PreprocessorString>(); }

    private: // Members ********************************************************

        std::variant<
            PreprocessorUndefined,
            PreprocessorInteger,
            PreprocessorFixedPoint,
            PreprocessorString
        > mVariant;

    };
}

// Standard Namespace Extension - Math Overloads *******************************

namespace stx
{
    template <typename T1, typename T2>
    inline constexpr auto pow (T1 base, T2 exp) -> T1
    {
        using namespace G10::ASM;

        if constexpr (std::is_same_v<T1, PreprocessorValue> &&
                      std::is_same_v<T2, PreprocessorValue>)
        {
            if (base.IsInteger() && exp.IsInteger())
                { return PPFP(std::pow(*base.GetInteger(), *exp.GetInteger())); }
            else if (base.IsFixedPoint() && exp.IsFixedPoint())
                { return PPFP(std::pow(base.GetFixedPoint()->GetComputed(),
                    exp.GetFixedPoint()->GetComputed())); }
            else if (base.IsInteger() && exp.IsFixedPoint())
                { return PPFP(std::pow(DBL(*base.GetInteger()),
                    exp.GetFixedPoint()->GetComputed())); }
            else if (base.IsFixedPoint() && exp.IsInteger())
                { return PPFP(std::pow(base.GetFixedPoint()->GetComputed(),
                    DBL(*exp.GetInteger()))); }
            else { return {}; }
        }
        else { return std::pow(base, exp); }

    }
}

#undef APX
#undef DBL
#undef PPS
#undef PPFP
#undef PPI
