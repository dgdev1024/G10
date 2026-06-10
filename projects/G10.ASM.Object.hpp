/**
 * @file    G10.ASM.Object.hpp
 * @brief   Contains declarations for the G10 Assembler's object file class,
 *          and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.ASM.Codegen.hpp>

// Classes *********************************************************************

namespace G10::ASM
{
    class G10_API Object final
    {
        friend class Codegen;

    public: // Constructors & Destructor ***************************************
    
        Object ();

    public: // Methods - Loading and Serialization *****************************
    
        auto LoadFile (const fs::path& pPath) -> stx::expect_good;
        auto LoadBuffer (std::vector<std::uint8_t>& pBuffer) -> stx::expect_good;

    public: // Methods *********************************************************

        auto LookupString (std::uint32_t pIndex) const
            -> stx::optional_ref<const std::string>;
        auto LookupStringByOffset (std::uint32_t pOffset) const
            -> stx::optional_ref<const std::string>;

    public: // Methods - Accessors *********************************************

        auto GetHeader () const -> const ObjectHeader&
            { return mHeader; }
        auto GetSectionTable () const -> const std::vector<ObjectSectionEntry>&
            { return mSectionTable; }
        auto GetSectionTable ()  -> std::vector<ObjectSectionEntry>&
            { return mSectionTable; }
        auto GetSymbolTable () const -> const std::vector<ObjectSymbolEntry>&
            { return mSymbolTable; }
        auto GetSymbolTable ()  -> std::vector<ObjectSymbolEntry>&
            { return mSymbolTable; }
        auto GetRelocationTable () const -> const std::vector<ObjectRelocationEntry>&
            { return mRelocationTable; }
        auto GetRelocationTable ()  -> std::vector<ObjectRelocationEntry>&
            { return mRelocationTable; }
        auto GetStringTable () const -> const std::vector<std::string>&
            { return mStringTable; }
        auto GetStringTable ()  -> std::vector<std::string>&
            { return mStringTable; }
        auto GetDataTable () const -> const stx::dual_vector<std::uint8_t>&
            { return mDataTable; }
        auto GetDataTable ()  -> stx::dual_vector<std::uint8_t>&
            { return mDataTable; }

    private: // Methods ********************************************************
    


    private: // Members ********************************************************
    
        ObjectHeader mHeader {};
        std::vector<ObjectSectionEntry> mSectionTable {};
        std::vector<ObjectSymbolEntry> mSymbolTable {};
        std::vector<ObjectRelocationEntry> mRelocationTable {};
        std::vector<std::string> mStringTable {};
        stx::dual_vector<std::uint8_t> mDataTable {};

    };
}
