/**
 * @file    G10.CPU.Program.hpp
 * @brief   Contains definitions for the G10 CPU's executable program,
 *          and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.Common.hpp>

// Constants & Enumerations ****************************************************

namespace G10::CPU
{
    constexpr std::uint32_t
        kProgramMagicNumber = 0x47313058;       // ASCII "G10X"
    constexpr std::uint32_t
        kProgramMajorVersion = 1,
        kProgramMinorVersion = 0,
        kProgramPatchVersion = 0,
        kProgramVersion =
            (kProgramMajorVersion * 10000) +
            (kProgramMinorVersion * 100) +
            kProgramPatchVersion;

    enum class ProgramFlags : std::uint32_t
    {
        None                = 0
    };
    G10_BITFIELD_ENUM(ProgramFlags)

    enum class ProgramSectionType : std::uint8_t
    {
        Metadata,
        Interrupt,
        Code,
        Data,
        BSS
    };
}

// Structures ******************************************************************

namespace G10::CPU
{
    struct ProgramHeader final
    {
        std::uint32_t       mMagicNumber { kProgramMagicNumber };
        std::uint32_t       mVersion { kProgramVersion };
        ProgramFlags        mFlags { };
        std::uint32_t       mEntryPoint { 0x2000 };
        std::uint32_t       mSectionTableOffset { 0 };
        std::uint32_t       mSectionCount { 0 };
        std::uint32_t       mSymbolTableOffset { 0 };
        std::uint32_t       mSymbolCount { 0 };
        std::uint32_t       mDataBufferOffset { 0 };
    };

    struct ProgramSectionEntry final
    {
        ProgramSectionType  mType {};
        std::uint8_t        mInterruptNumber { 0 };
        std::uint16_t       mAlignmentBoundary { 0 };
        std::uint32_t       mTargetAddress { 0 };
        std::uint32_t       mDataSize { 0 };
        std::uint32_t       mDataOffset { 0 };
    };

    struct ProgramSymbolEntry final
    {
        std::uint32_t       mAddressOffset { 0 };
        std::uint32_t       mSectionIndex { 0 };
    };
}

// Classes *********************************************************************

namespace G10::CPU
{
    class G10_API Program final
    {
    public: // Constructors & Destructor ***************************************

    

    public: // Methods *********************************************************

        auto Clear () -> void;
        auto LoadFile (const fs::path& pPath) -> stx::expect_good;
        auto LoadBuffer (std::vector<std::uint8_t>& pBuffer) -> stx::expect_good;

    public: // Methods - Bus Access ********************************************

        auto Read (std::uint32_t pAddress, std::uint8_t& pData) const -> bool;

    public: // Methods - Accessors *********************************************

        inline auto GetHeader () const -> const ProgramHeader&
            { return mHeader; }
        inline auto GetSectionTable () const -> const std::vector<ProgramSectionEntry>&
            { return mSectionTable; }
        inline auto GetSymbolTable () const -> const std::vector<ProgramSymbolEntry>&
            { return mSymbolTable; }
        inline auto GetDataTable () const -> const stx::dual_vector<std::uint8_t>&
            { return mDataTable; }
        inline auto GetMetadataBuffer () -> std::vector<std::uint8_t>&
            { return mMetadataBuffer; }
        inline auto GetEntryPoint () const -> std::uint32_t
            { return mHeader.mEntryPoint; }

    private: // Methods ********************************************************



    private: // Members ********************************************************

        ProgramHeader mHeader {};
        std::vector<ProgramSectionEntry> mSectionTable {};
        std::vector<ProgramSymbolEntry> mSymbolTable {};
        stx::dual_vector<std::uint8_t> mDataTable {};
        std::vector<std::uint8_t> mMetadataBuffer {};

        mutable const ProgramSectionEntry* mLastSection { nullptr };
        mutable const std::vector<std::uint8_t>* mLastData { nullptr };

    };
}
