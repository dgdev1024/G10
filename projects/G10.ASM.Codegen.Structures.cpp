/**
 * @file    G10.ASM.Codegen.Structures.cpp
 * @brief   Contains implementations of byte view specialization methods
 *          for the code generation unit's structures, and related definitions.
 */

#include <G10.ASM.Codegen.hpp>

// Static - Byte View Specializations ******************************************

namespace stx
{
    using namespace G10::ASM;

    template <>
    auto byte_view::read<ObjectHeader> () -> ObjectHeader
    {
        ObjectHeader header {};
        header.mMagicNumber = read_dword_be();
        header.mVersion = read_dword_le();
        header.mFlags = static_cast<ObjectFlags>(read_dword_le());
        header.mSectionTableOffset = read_dword_le();
        header.mSectionCount = read_dword_le();
        header.mSymbolTableOffset = read_dword_le();
        header.mSymbolCount = read_dword_le();
        header.mRelocationTableOffset = read_dword_le();
        header.mRelocationCount = read_dword_le();
        header.mStringTableOffset = read_dword_le();
        header.mStringBufferSize = read_dword_le();
        header.mStringCount = read_dword_le();
        header.mDataBufferOffset = read_dword_le();
        return header;
    }

    template <>
    auto byte_view::read<ObjectSectionEntry> () -> ObjectSectionEntry
    {
        ObjectSectionEntry entry {};
        entry.mNameStringOffset = read_dword_le();
        entry.mType = static_cast<ObjectSectionType>(read_byte());
        entry.mInterruptNumber = read_byte();
        entry.mAlignmentBoundary = read_word_le();
        entry.mTargetAddress = read_dword_le();
        entry.mDataSize = read_dword_le();
        entry.mDataOffset = read_dword_le();
        return entry;
    }

    template <>
    auto byte_view::read<ObjectSymbolEntry> () -> ObjectSymbolEntry
    {
        ObjectSymbolEntry entry {};
        entry.mNameStringOffset = read_dword_le();
        entry.mType = static_cast<ObjectSymbolType>(read_byte());
        entry.mAddressOffset = read_dword_le();
        entry.mSectionIndex = read_dword_le();
        return entry;
    }

    template <>
    auto byte_view::read<ObjectRelocationEntry> () -> ObjectRelocationEntry
    {
        ObjectRelocationEntry entry {};
        entry.mPatchOffset = read_dword_le();
        entry.mSymbolIndex = read_dword_le();
        entry.mSectionIndex = read_dword_le();
        entry.mType = static_cast<ObjectRelocationType>(read_byte());
        entry.mSize = read_byte();
        return entry;
    }

    template <>
    auto byte_view::write<ObjectHeader> (const ObjectHeader& header) -> void
    {
        write_dword_be(header.mMagicNumber);
        write_dword_le(header.mVersion);
        write_dword_le(std::to_underlying(header.mFlags));
        write_dword_le(header.mSectionTableOffset);
        write_dword_le(header.mSectionCount);
        write_dword_le(header.mSymbolTableOffset);
        write_dword_le(header.mSymbolCount);
        write_dword_le(header.mRelocationTableOffset);
        write_dword_le(header.mRelocationCount);
        write_dword_le(header.mStringTableOffset);
        write_dword_le(header.mStringBufferSize);
        write_dword_le(header.mStringCount);
        write_dword_le(header.mDataBufferOffset);
    }

    template <>
    auto byte_view::write<ObjectSectionEntry> (const ObjectSectionEntry& entry) -> void
    {
        write_dword_le(entry.mNameStringOffset);
        write_byte(std::to_underlying(entry.mType));
        write_byte(entry.mInterruptNumber);
        write_word_le(entry.mAlignmentBoundary);
        write_dword_le(entry.mTargetAddress);
        write_dword_le(entry.mDataSize);
        write_dword_le(entry.mDataOffset);
    }

    template <>
    auto byte_view::write<ObjectSymbolEntry> (const ObjectSymbolEntry& entry) -> void
    {
        write_dword_le(entry.mNameStringOffset);
        write_byte(std::to_underlying(entry.mType));
        write_dword_le(entry.mAddressOffset);
        write_dword_le(entry.mSectionIndex);
    }

    template <>
    auto byte_view::write<ObjectRelocationEntry> (const ObjectRelocationEntry& entry) -> void
    {
        write_dword_le(entry.mPatchOffset);
        write_dword_le(entry.mSymbolIndex);
        write_dword_le(entry.mSectionIndex);
        write_byte(std::to_underlying(entry.mType));
        write_byte(entry.mSize);
    }

    template <>
    auto byte_view::push<ObjectHeader> (const ObjectHeader& header) -> void
    {
        push_dword_be(header.mMagicNumber);
        push_dword_le(header.mVersion);
        push_dword_le(std::to_underlying(header.mFlags));
        push_dword_le(header.mSectionTableOffset);
        push_dword_le(header.mSectionCount);
        push_dword_le(header.mSymbolTableOffset);
        push_dword_le(header.mSymbolCount);
        push_dword_le(header.mRelocationTableOffset);
        push_dword_le(header.mRelocationCount);
        push_dword_le(header.mStringTableOffset);
        push_dword_le(header.mStringBufferSize);
        push_dword_le(header.mStringCount);
        push_dword_le(header.mDataBufferOffset);
    }

    template <>
    auto byte_view::push<ObjectSectionEntry> (const ObjectSectionEntry& entry) -> void
    {
        push_dword_le(entry.mNameStringOffset);
        push_byte(std::to_underlying(entry.mType));
        push_byte(entry.mInterruptNumber);
        push_word_le(entry.mAlignmentBoundary);
        push_dword_le(entry.mTargetAddress);
        push_dword_le(entry.mDataSize);
        push_dword_le(entry.mDataOffset);
    }

    template <>
    auto byte_view::push<ObjectSymbolEntry> (const ObjectSymbolEntry& entry) -> void
    {
        push_dword_le(entry.mNameStringOffset);
        push_byte(std::to_underlying(entry.mType));
        push_dword_le(entry.mAddressOffset);
        push_dword_le(entry.mSectionIndex);
    }

    template <>
    auto byte_view::push<ObjectRelocationEntry> (const ObjectRelocationEntry& entry) -> void
    {
        push_dword_le(entry.mPatchOffset);
        push_dword_le(entry.mSymbolIndex);
        push_dword_le(entry.mSectionIndex);
        push_byte(std::to_underlying(entry.mType));
        push_byte(entry.mSize);
    }
}
