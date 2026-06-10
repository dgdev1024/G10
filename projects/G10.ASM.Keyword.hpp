/**
 * @file    G10.ASM.Keyword.hpp
 * @brief   Contains declarations for the G10 Assembler's keyword class, 
 *          and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.ASM.Common.hpp>
#include <G10.CPU.Common.hpp>

// Types ***********************************************************************

namespace G10::ASM
{
    using KeywordType = std::variant<
        CPU::Register,
        CPU::Condition,
        CPU::InstructionType,
        PreprocessorDirective,
        SectionName,
        PreprocessorFunction,
        AssemblerDirective,
        PreprocessorBuiltinSymbol,
        Hint
    >;

    template <typename T>
    concept KeywordTypename = (
        std::same_as<T, CPU::Register> ||
        std::same_as<T, CPU::Condition> ||
        std::same_as<T, CPU::InstructionType> ||
        std::same_as<T, PreprocessorDirective> ||
        std::same_as<T, SectionName> ||
        std::same_as<T, PreprocessorFunction> ||
        std::same_as<T, AssemblerDirective> ||
        std::same_as<T, PreprocessorBuiltinSymbol> ||
        std::same_as<T, Hint>
    );
}

// Constants & Enumerations ****************************************************

namespace G10::ASM
{
    enum class KeywordGroup : std::uint8_t
    {
        Register,
        Condition,
        InstructionType,
        PreprocessorDirective,
        SectionName,
        PreprocessorFunction,
        AssemblerDirective,
        PreprocessorBuiltinSymbol,
        Hint
    };
}

// Classes *********************************************************************

namespace G10::ASM
{
    class G10_API Keyword final
    {
    public: // Types ***********************************************************

        using Ref = std::reference_wrapper<const Keyword>;

    public: // Constructors & Destructor ***************************************

        explicit Keyword (const CPU::Register pType);
        explicit Keyword (const CPU::Condition pType);
        explicit Keyword (const CPU::InstructionType pType);
        explicit Keyword (const PreprocessorDirective pType);
        explicit Keyword (const SectionName pType);
        explicit Keyword (const PreprocessorFunction pType);
        explicit Keyword (const AssemblerDirective pType);
        explicit Keyword (const PreprocessorBuiltinSymbol pType);
        explicit Keyword (const Hint pType);

    public: // Static Methods **************************************************

        static auto Lookup (std::string str) -> stx::optional_ref<const Keyword>;

    public: // Methods *********************************************************

        template <KeywordTypename T>
        inline constexpr auto Is () const -> bool
            { return std::holds_alternative<T>(mType); }

        template <KeywordTypename T>
        inline constexpr auto GetTypeIf () const -> const T*
            { return std::get_if<T>(&mType); }

        template <KeywordTypename T>
        inline auto GetType () const -> T
        {
            return *std::get_if<T>(&mType);
        }

        auto GetGroup () const -> KeywordGroup;
        auto StringifyType () const -> std::string_view;
        auto StringifyGroup () const -> std::string_view;

    private: // Members ********************************************************

        static std::unordered_map<std::string_view, Keyword> mLookup;
        KeywordGroup mGroup {};
        KeywordType mType {};

    };
}
