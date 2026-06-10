/**
 * @file    G10.CPU.Program.cpp
 * @brief   Contains implementations for the G10 CPU's executable program,
 *          and related definitions.
 */

// Includes ********************************************************************

#include <G10.CPU.Program.hpp>

// Static - Byte View Template Specializations *********************************

namespace stx
{
    using namespace G10::CPU;

    template <>
    auto byte_view::read<ProgramHeader> () -> ProgramHeader
    {
        ProgramHeader header {};
        header.mMagicNumber = read_dword_be();
        header.mVersion = read_dword_le();
        header.mFlags = static_cast<ProgramFlags>(read_dword_le());
        header.mEntryPoint = read_dword_le();
        header.mSectionTableOffset = read_dword_le();
        header.mSectionCount = read_dword_le();
        header.mSymbolTableOffset = read_dword_le();
        header.mSymbolCount = read_dword_le();
        header.mDataBufferOffset = read_dword_le();

        return header;
    }

    template <>
    auto byte_view::read<ProgramSectionEntry> () -> ProgramSectionEntry
    {
        ProgramSectionEntry entry {};
        entry.mType = static_cast<ProgramSectionType>(read_byte());
        entry.mInterruptNumber = read_byte();
        entry.mAlignmentBoundary = read_word_le();
        entry.mTargetAddress = read_dword_le();
        entry.mDataSize = read_dword_le();
        entry.mDataOffset = read_dword_le();

        return entry;
    }

    template <>
    auto byte_view::read<ProgramSymbolEntry> () -> ProgramSymbolEntry
    {
        ProgramSymbolEntry entry {};
        entry.mAddressOffset = read_dword_le();
        entry.mSectionIndex = read_dword_le();

        return entry;
    }

    template <>
    auto byte_view::write<ProgramHeader> (const ProgramHeader& header) -> void
    {
        write_dword_be(header.mMagicNumber);
        write_dword_le(header.mVersion);
        write_dword_le(static_cast<uint32_t>(header.mFlags));
        write_dword_le(header.mEntryPoint);
        write_dword_le(header.mSectionTableOffset);
        write_dword_le(header.mSectionCount);
        write_dword_le(header.mSymbolTableOffset);
        write_dword_le(header.mSymbolCount);
        write_dword_le(header.mDataBufferOffset);
    }

    template <>
    auto byte_view::write<ProgramSectionEntry> (const ProgramSectionEntry& entry) -> void
    {
        write_byte(std::to_underlying(entry.mType));
        write_byte(entry.mInterruptNumber);
        write_word_le(entry.mAlignmentBoundary);
        write_dword_le(entry.mTargetAddress);
        write_dword_le(entry.mDataSize);
        write_dword_le(entry.mDataOffset);
    }

    template <>
    auto byte_view::write<ProgramSymbolEntry> (const ProgramSymbolEntry& entry) -> void
    {
        write_dword_le(entry.mAddressOffset);
        write_dword_le(entry.mSectionIndex);
    }

    template <>
    auto byte_view::push<ProgramHeader> (const ProgramHeader& header) -> void
    {
        write_dword_be(header.mMagicNumber);
        write_dword_le(header.mVersion);
        write_dword_le(static_cast<uint32_t>(header.mFlags));
        write_dword_le(header.mEntryPoint);
        write_dword_le(header.mSectionTableOffset);
        write_dword_le(header.mSectionCount);
        write_dword_le(header.mSymbolTableOffset);
        write_dword_le(header.mSymbolCount);
        write_dword_le(header.mDataBufferOffset);
    }

    template <>
    auto byte_view::push<ProgramSectionEntry> (const ProgramSectionEntry& entry) -> void
    {
        write_byte(std::to_underlying(entry.mType));
        write_byte(entry.mInterruptNumber);
        write_word_le(entry.mAlignmentBoundary);
        write_dword_le(entry.mTargetAddress);
        write_dword_le(entry.mDataSize);
        write_dword_le(entry.mDataOffset);
    }

    template <>
    auto byte_view::push<ProgramSymbolEntry> (const ProgramSymbolEntry& entry) -> void
    {
        write_dword_le(entry.mAddressOffset);
        write_dword_le(entry.mSectionIndex);
    }
}

// Public - Constructors & Destructor ******************************************

namespace G10::CPU
{

}

// Public Methods **************************************************************

namespace G10::CPU
{
    auto Program::Clear () -> void
    {
        mLastData = nullptr;
        mLastSection = nullptr;

        mHeader = {};
        mSectionTable.clear();
        mSymbolTable.clear();
        mDataTable.clear();
        mMetadataBuffer.clear();
    }

    auto Program::LoadFile (const fs::path& pPath) -> stx::expect_good
    {
        auto resolved = fs::absolute(pPath).lexically_normal();
        if (fs::exists(resolved) == false)
        {
            return stx::fmt_unexpected(
                "LoadFile: Program file '{}' not found.",
                resolved.string()
            );
        }

        auto file_size = fs::file_size(resolved);
        std::fstream file { pPath, std::ios::binary | std::ios::in };
        if (file.is_open() == false)
        {
            return stx::fmt_unexpected(
                "LoadFile: Could not open program file '{}' for reading.",
                resolved.string()
            );
        }

        std::vector<std::uint8_t> buffer;
        buffer.resize(file_size);
        file.read(reinterpret_cast<char*>(buffer.data()), file_size);
        file.close();

        debug("Loading Program File '{}'...", pPath.string());
        return LoadBuffer(buffer);
    }

    auto Program::LoadBuffer (std::vector<std::uint8_t>& pBuffer) -> stx::expect_good
    {
        // 1. Begin by reading the program file's header.
        stx::byte_view view { pBuffer };
        mHeader = view.read<ProgramHeader>();

        // 2. Next, read the section table.
        mSectionTable.resize(mHeader.mSectionCount);
        for (std::size_t i = 0; i < mHeader.mSectionCount; ++i)
            { mSectionTable[i] = view.read<ProgramSectionEntry>(); }

        // 3. Next, read the symbol table.
        mSymbolTable.resize(mHeader.mSymbolCount);
        for (std::size_t i = 0; i < mHeader.mSymbolCount; ++i)
            { mSymbolTable[i] = view.read<ProgramSymbolEntry>(); }

        // 4. Read the data table.
        // - Flatten the buffers in the data table into a flat vector.
        // - While we're here, also prepare the metadata buffer.
        mDataTable.clear();
        std::vector<std::uint8_t> metadataBuffer(0x1000, 0x00);
        for (const auto& section : mSectionTable)
        {
            std::vector<std::uint8_t> buffer {};
            if (section.mType != ProgramSectionType::BSS && section.mDataSize > 0)
            {
                buffer = view.read_buffer(section.mDataSize);
            }

            if (section.mType == ProgramSectionType::Metadata)
            {
                std::copy(buffer.begin(), buffer.end(), metadataBuffer.begin() + section.mTargetAddress);
            }

            mDataTable.push_back(std::move(buffer));
        }

        mMetadataBuffer = std::move(metadataBuffer);
        return {};
    }
}

// Public Methods - Bus Access *************************************************

namespace G10::CPU
{
    auto Program::Read (std::uint32_t pAddress, std::uint8_t& pData) const -> bool
    {
        if (mLastSection != nullptr &&
            mLastData != nullptr &&
            pAddress >= mLastSection->mTargetAddress &&
            pAddress < mLastSection->mTargetAddress + mLastSection->mDataSize)
        {
            pData = (*mLastData)[pAddress - mLastSection->mTargetAddress];
            return true;
        }

        for (std::size_t i = 0; i < mSectionTable.size(); ++i)
        {
            const auto& section = mSectionTable[i];
            const auto& data = mDataTable[i];
            const auto end = section.mTargetAddress + section.mDataSize;
            if (pAddress >= section.mTargetAddress && pAddress < end)
            {
                mLastSection = &section;
                mLastData = &data;
                pData = data[pAddress - section.mTargetAddress];
                return true;
            }
        }

        return false;
    }
}

// Private Methods *************************************************************

namespace G10::CPU
{
    
}
