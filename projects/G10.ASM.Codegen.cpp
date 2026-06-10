/**
 * @file    G10.ASM.Codegen.cpp
 * @brief   Contains implementations for the G10 Assembler's code generation
 *          unit, and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Codegen.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::ASM
{
    Codegen::Codegen (Diagnostic& pDiag) :
        mDiag { pDiag }
    {
        mStringBuffer.emplace_back('\0');
    }
}

// Public Methods - Input & Output *********************************************

namespace G10::ASM
{
    auto Codegen::Run (const SyntaxModule& pModule) -> bool
    {
        for (const auto& node : pModule.mNodes)
        {
            if (Dispatch(node) == false)
                { return false; }
        }

        return BuildOutput();
    }

    auto Codegen::GetOutput () -> std::vector<std::uint8_t>&
    {
        return mOutput;
    }

    auto Codegen::GetOutput () const -> const std::vector<std::uint8_t>&
    {
        return mOutput;
    }

    auto Codegen::SaveOutput (const fs::path& pPath) -> bool
    {
        const auto& resolved = NormalizePath(pPath);
        std::fstream file { resolved, std::ios::out | std::ios::binary };
        if (file.is_open() == false)
        {
            mDiag.ReportError("SaveOutput: Could not open binary file '{}' for writing.",
                resolved.string());
            return false;
        }

        file.write(reinterpret_cast<const char*>(mOutput.data()), mOutput.size());
        file.close();
        return true;
    }
}

// Public Methods - Options ****************************************************

namespace G10::ASM
{
    auto Codegen::SetIncludeDirectories (const std::vector<std::string>& pIncludeDirs) -> void
    {
        mIncludeDirs.clear();
        for (const auto& dir : pIncludeDirs)
        {
            mIncludeDirs.push_back(NormalizePath(dir));
        }
    }
}

// Private Methods - String Table **********************************************

namespace G10::ASM
{
    auto Codegen::AddString (const std::string& pString) -> std::uint32_t
    {
        auto it = mStringLookup.find(pString);
        if (it != mStringLookup.end())
        {
            return it->second;
        }

        auto offset = static_cast<std::uint32_t>(mStringBuffer.size());
        mStringBuffer.insert(mStringBuffer.end(), pString.begin(), pString.end());
        mStringBuffer.emplace_back('\0');
        mStringLookup[pString] = offset;
        mHeader.mStringCount++;
        mHeader.mStringBufferSize += static_cast<std::uint32_t>(pString.size()) + 1;
        return offset;
    }

    auto Codegen::GetStringBuffer () const -> std::span<const char>
        { return mStringBuffer; }
}

// Private Methods - Sections & Symbols ****************************************

namespace G10::ASM
{
    auto Codegen::SetSection (
        const std::string&  pDisplayName, 
        ObjectSectionType   pType,
        std::uint8_t        pInterruptNumber,
        std::uint32_t       pTargetAddress,
        std::uint16_t       pAlignment
    ) -> void
    {
        ObjectSectionContext ctx {};
        ctx.mHeader = ObjectSectionEntry {
            .mNameStringOffset  = AddString(pDisplayName),
            .mType              = pType,
            .mAlignmentBoundary = pAlignment,
            .mTargetAddress     = pTargetAddress,
            .mDataSize          = 0,
            .mDataOffset        = 0
        };

        if (pType == ObjectSectionType::Interrupt)
        {
            ctx.mHeader.mInterruptNumber = (pInterruptNumber & 0x1F);
            ctx.mHeader.mTargetAddress = CPU::kMemInterruptStartAddr +
                (ctx.mHeader.mInterruptNumber * 0x80);
        }

        if (pTargetAddress == stx::npos32)
        {
            auto last = GetLastSectionOfType(pType);
            if (last.has_value() == true)
            {
                ctx.mHeader.mTargetAddress = 
                    last->mHeader.mTargetAddress +
                    last->mHeader.mDataSize;
            }
            else switch (pType)
            {
                case ObjectSectionType::Metadata:
                    ctx.mHeader.mTargetAddress = 0;
                    break;
                case ObjectSectionType::Code:
                case ObjectSectionType::Data:
                    ctx.mHeader.mTargetAddress = 0x2000;
                    break;
                case ObjectSectionType::BSS:
                    ctx.mHeader.mTargetAddress = 0x80000000;
                    break;
            }
        }

        mSections.push_back(std::move(ctx));
        mHeader.mSectionCount++;
        mActiveSectionIndex = (mSections.size() - 1);
    }

    auto Codegen::ResolveSymbol (const std::string& pName) -> std::uint32_t
    {
        auto it = mSymbolNameIndices.find(pName);
        if (it != mSymbolNameIndices.end())
            { return it->second; }

        std::uint32_t index = static_cast<std::uint32_t>(mSymbols.size());

        mSymbols.emplace_back(ObjectSymbolEntry {
            .mNameStringOffset  = AddString(pName),
            .mType              = ObjectSymbolType::Import,
            .mAddressOffset     = 0,
            .mSectionIndex      = stx::npos32
        });

        mHeader.mSymbolCount++;
        mSymbolNameIndices[pName] = index;
        return index;
    }

    auto Codegen::AddRelocation (
        const std::uint32_t     pSymbolIndex,
        ObjectRelocationType    pType,
        std::uint8_t            pSize,
        std::uint32_t           pInitialValue
    ) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError("AddRelocation: No active section to add relocation for.");
            return false;
        }

        const auto& ctx = mSections[mActiveSectionIndex];
        mRelocations.emplace_back(ObjectRelocationEntry {
            .mPatchOffset       = ctx.mLocationCounter,
            .mSymbolIndex       = pSymbolIndex,
            .mSectionIndex      = mActiveSectionIndex,
            .mType              = pType,
            .mSize              = pSize
        });
        mHeader.mRelocationCount++;

        switch (pSize)
        {
            case 1: return EmitByte(pInitialValue & 0xFF);
            case 2: return EmitWord(pInitialValue & 0xFFFF);
            case 4: return EmitDoubleWord(pInitialValue);
            default:
                mDiag.ReportError("AddRelocation: Unsupported relocation size {}.", 
                    pSize);
                return false;
        }

        return true;
    }

    auto Codegen::AddRelocation (
        const std::string&      pSymbolName,
        ObjectRelocationType    pType,
        std::uint8_t            pSize,
        std::uint32_t           pInitialValue
    ) -> bool
    {
        if (pSymbolName.empty())
        {
            mDiag.ReportError("AddRelocation: Symbol name is empty.");
            return false;
        }

        const auto index = ResolveSymbol(pSymbolName);
        return AddRelocation(index, pType, pSize, pInitialValue);
    }
    
    auto Codegen::GetLastSectionOfType (ObjectSectionType pType)
        -> stx::optional_ref<ObjectSectionContext>
    {
        for (auto it = mSections.rbegin(); it != mSections.rend(); ++it)
        {
            auto& section = *it;

            // If `pType` is CODE or DATA, then our target type can be either
            // one of those two.
            if (pType == ObjectSectionType::Code || 
                pType == ObjectSectionType::Data)
            {
                if (section.mHeader.mType == ObjectSectionType::Code ||
                    section.mHeader.mType == ObjectSectionType::Data)
                {
                    return section;
                }
            }
            else if (section.mHeader.mType == pType)
            {
                return section;
            }
        }

        return std::nullopt;
    }
}

// Private Methods - Emission **************************************************

namespace G10::ASM
{
    auto Codegen::EmitByte (std::uint8_t pByte) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError("EmitByte: No active section to emit byte to.");
            return false;
        }

        auto& ctx = mSections[mActiveSectionIndex];
        if (ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            mDiag.ReportError("EmitByte: Cannot emit byte to BSS section.");
            return false;
        }

        stx::byte_view view { ctx.mData };
        view.push_byte(pByte);
        ctx.mHeader.mDataSize = ctx.mLocationCounter =
            static_cast<std::uint32_t>(ctx.mData.size());
        return true;
    }

    auto Codegen::EmitWord (std::uint16_t pWord) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError("EmitWord: No active section to emit word to.");
            return false;
        }

        auto& ctx = mSections[mActiveSectionIndex];
        if (ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            mDiag.ReportError("EmitWord: Cannot emit word to BSS section.");
            return false;
        }

        stx::byte_view view { ctx.mData };
        view.push_word_le(pWord);
        ctx.mHeader.mDataSize = ctx.mLocationCounter =
            static_cast<std::uint32_t>(ctx.mData.size());
        return true;
    }

    auto Codegen::EmitDoubleWord (std::uint32_t pDoubleWord) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError("EmitDoubleWord: No active section to emit double word to.");
            return false;
        }

        auto& ctx = mSections[mActiveSectionIndex];
        if (ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            mDiag.ReportError("EmitDoubleWord: Cannot emit double word to BSS section.");
            return false;
        }

        stx::byte_view view { ctx.mData };
        view.push_dword_le(pDoubleWord);
        ctx.mHeader.mDataSize = ctx.mLocationCounter =
            static_cast<std::uint32_t>(ctx.mData.size());
        return true;
    }

    auto Codegen::EmitLabel (const std::string& pLabel, std::uint32_t pAddress) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError("EmitLabel: No active section to emit label to.");
            return false;
        }
        
        if (const auto findIt = mSymbolNameIndices.find(pLabel); 
            findIt != mSymbolNameIndices.end())
        {
            const auto symbolIndex = findIt->second;
            if (symbolIndex >= mSymbols.size())
            {
                mDiag.ReportError("EmitLabel: Symbol '{}' resolves to an index out of bounds.", 
                    pLabel);
                return false;
            }

            auto& symbol = mSymbols[symbolIndex];
            if (symbol.mSectionIndex != stx::npos32)
            {
                mDiag.ReportError("EmitLabel: Redefinition of symbol '{}'.", pLabel);
                return false;
            }

            if (symbol.mType == ObjectSymbolType::Import)
                { symbol.mType = ObjectSymbolType::Local; }

            symbol.mAddressOffset = pAddress;
            symbol.mSectionIndex = mActiveSectionIndex;
        }
        else
        {
            mSymbols.emplace_back(ObjectSymbolEntry {
                .mNameStringOffset = AddString(pLabel),
                .mType = ObjectSymbolType::Local,
                .mAddressOffset = pAddress,
                .mSectionIndex = mActiveSectionIndex
            });
            
            mSymbolNameIndices[pLabel] =
                static_cast<std::uint32_t>(mSymbols.size() - 1);
        }

        return true;
    }

    auto Codegen::EmitString (const std::string& pString, bool pNoTerminator) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError("EmitString: No active section to emit string to.");
            return false;
        }

        auto& ctx = mSections[mActiveSectionIndex];
        if (ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            mDiag.ReportError("EmitString: Cannot emit string to BSS section.");
            return false;
        }

        stx::byte_view view { ctx.mData };
        
        if (pNoTerminator == true)
        {
            for (const auto c : pString)
                { view.push_byte(c); }
        }
        else
            { view.push_string(pString); }

        ctx.mHeader.mDataSize = ctx.mLocationCounter =
            static_cast<std::uint32_t>(ctx.mData.size());
        return true;
    }

    auto Codegen::EmitOpcode (std::uint16_t pOpcode) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError("EmitOpcode: No active section to emit opcode to.");
            return false;
        }

        auto& ctx = mSections[mActiveSectionIndex];
        if (ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            mDiag.ReportError("EmitOpcode: Cannot emit opcode to BSS section.");
            return false;
        }

        stx::byte_view view { ctx.mData };
        view.push_word_be(pOpcode);
        ctx.mHeader.mDataSize = ctx.mLocationCounter =
            static_cast<std::uint32_t>(ctx.mData.size());
        return true;
    }

    auto Codegen::EmitOpcode (std::uint8_t pOpcode, std::uint8_t pParamX, 
        std::uint8_t pParamY) -> bool
    {
        std::uint16_t full =
            (static_cast<std::uint16_t>(pOpcode) << 8) |
            (static_cast<std::uint16_t>(pParamX & 0xF) << 4) |
            (static_cast<std::uint16_t>(pParamY & 0xF));

        return EmitOpcode(full);
    }

    auto Codegen::ReserveBytes (std::uint32_t pCount) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError("ReserveBytes: No active section to reserve bytes in.");
            return false;
        }

        auto& ctx = mSections[mActiveSectionIndex];
        if (ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            // Reserves in BSS sections do not touch the `mData` vector at all.
            ctx.mHeader.mDataSize = (ctx.mLocationCounter += pCount);
        }
        else
        {
            ctx.mData.insert(ctx.mData.end(), pCount, 0);
            ctx.mHeader.mDataSize = ctx.mLocationCounter =
                static_cast<std::uint32_t>(ctx.mData.size());
        }

        return true;
    }

    auto Codegen::ReserveWords (std::uint32_t pCount) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError("ReserveWords: No active section to reserve words in.");
            return false;
        }

        auto& ctx = mSections[mActiveSectionIndex];
        if (ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            // Reserves in BSS sections do not touch the `mData` vector at all.
            ctx.mHeader.mDataSize = (ctx.mLocationCounter += (pCount * 2));
        }
        else
        {
            ctx.mData.insert(ctx.mData.end(), pCount * 2, 0);
            ctx.mHeader.mDataSize = ctx.mLocationCounter =
                static_cast<std::uint32_t>(ctx.mData.size());
        }

        return true;
    }

    auto Codegen::ReserveDoubleWords (std::uint32_t pCount) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError("ReserveDoubleWords: No active section to reserve double words in.");
            return false;
        }

        auto& ctx = mSections[mActiveSectionIndex];
        if (ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            // Reserves in BSS sections do not touch the `mData` vector at all.
            ctx.mHeader.mDataSize = (ctx.mLocationCounter += (pCount * 4));
        }
        else
        {
            ctx.mData.insert(ctx.mData.end(), pCount * 4, 0);
            ctx.mHeader.mDataSize = ctx.mLocationCounter =
                static_cast<std::uint32_t>(ctx.mData.size());
        }

        return true;
    }
}

// Private Methods - Build Output **********************************************

namespace G10::ASM
{
    template <typename T>
    static auto ComputeSize (const T& pValue = T {}) -> std::size_t
    {
        static std::vector<std::uint8_t> tmp {};
        static stx::byte_view view { tmp };
        tmp.clear();
        view.reset();
        view.push<T>(pValue);
        return tmp.size();
    }

    auto Codegen::BuildOutput () -> bool
    {
        // 1. Compute exact wire sizes.
        std::size_t headerSize = ComputeSize<ObjectHeader>(mHeader),
                    sectionEntrySize = ComputeSize<ObjectSectionEntry>(),
                    symbolEntrySize = ComputeSize<ObjectSymbolEntry>(),
                    relocationEntrySize = ComputeSize<ObjectRelocationEntry>();

        // 2. Compute table and data offsets.
        const std::uint32_t
            sectionTableOffset = static_cast<std::uint32_t>(headerSize),
            sectionTableSize = static_cast<std::uint32_t>(mSections.size() * sectionEntrySize),
            symbolTableOffset = sectionTableOffset + sectionTableSize,
            symbolTableSize = static_cast<std::uint32_t>(mSymbols.size() * symbolEntrySize),
            relocationTableOffset = symbolTableOffset + symbolTableSize,
            relocationTableSize = static_cast<std::uint32_t>(mRelocations.size() * relocationEntrySize),
            stringTableOffset = relocationTableOffset + relocationTableSize,
            stringTableSize = static_cast<std::uint32_t>(mStringBuffer.size()),
            dataBufferOffset = stringTableOffset + stringTableSize;
        std::uint32_t dataOffset = stringTableOffset + stringTableSize;

        // 3. Compute per-section data offsets (respect alignment). BSS sections use offset 0.
        std::vector<std::uint32_t> sectionDataOffsets(mSections.size(), 0);
        for (std::size_t i = 0; i < mSections.size(); ++i)
        {
            const auto& ctx = mSections[i];
            if (ctx.mHeader.mType == ObjectSectionType::BSS)
            {
                sectionDataOffsets[i] = 0;
                continue;
            }

            const std::uint32_t align = std::max(
                static_cast<std::uint32_t>(ctx.mHeader.mAlignmentBoundary), 1u);
            const std::uint32_t pad = (align - (dataOffset % align)) % align;
            dataOffset += pad;
            sectionDataOffsets[i] = dataOffset;
            dataOffset += ctx.mHeader.mDataSize;
        }

        // 4. Reserve output buffer and start writing.
        mOutput.clear();
        mOutput.reserve(static_cast<std::size_t>(dataOffset));
        stx::byte_view out{ mOutput };

        // 5. Prepare and write header (counts already in mHeader, fill offsets).
        ObjectHeader outHeader = mHeader;
        outHeader.mStringTableOffset = stringTableOffset;
        outHeader.mSectionTableOffset = sectionTableOffset;
        outHeader.mSectionCount = static_cast<std::uint32_t>(mSections.size());
        outHeader.mSymbolTableOffset = symbolTableOffset;
        outHeader.mSymbolCount = static_cast<std::uint32_t>(mSymbols.size());
        outHeader.mRelocationTableOffset = relocationTableOffset;
        outHeader.mRelocationCount = static_cast<std::uint32_t>(mRelocations.size());
        outHeader.mDataBufferOffset = dataBufferOffset;
        out.push<ObjectHeader>(outHeader);

        // 6. Write section table (adjust name offsets and data offsets).
        for (std::size_t i = 0; i < mSections.size(); ++i)
        {
            ObjectSectionEntry sectionEntry = mSections[i].mHeader;
            // sectionEntry.mNameStringOffset = stringTableOffset + sectionEntry.mNameStringOffset;
            sectionEntry.mDataOffset = sectionDataOffsets[i];
            out.push<ObjectSectionEntry>(sectionEntry);
        }

        // 7. Write symbol table (adjust name offsets).
        for (const auto& symbol : mSymbols)
        {
            ObjectSymbolEntry symbolEntry = symbol;
            // symbolEntry.mNameStringOffset = stringTableOffset + symbolEntry.mNameStringOffset;
            out.push<ObjectSymbolEntry>(symbolEntry);
        }

        // 8. Write relocation table.
        for (const auto& relocation : mRelocations)
            { out.push<ObjectRelocationEntry>(relocation); }

        // 9. Write string table as raw bytes.
        if (!mStringBuffer.empty())
        {
            std::vector<std::uint8_t> sbuf(mStringBuffer.begin(), mStringBuffer.end());
            out.push_buffer(sbuf);
        }

        // 10. Write section data blocks respecting computed offsets and padding.
        for (std::size_t i = 0; i < mSections.size(); ++i)
        {
            const auto& ctx = mSections[i];
            const std::uint32_t target = sectionDataOffsets[i];
            if (target == 0) // BSS - no data in file
                { continue; }

            if (mOutput.size() > target)
            {
                mDiag.ReportError("BuildOutput: section {} data would overlap previous data.", i);
                return false;
            }

            const std::size_t pad = static_cast<std::size_t>(target - static_cast<std::uint32_t>(mOutput.size()));
            if (pad > 0)
                { out.push_buffer(std::vector<std::uint8_t>(pad, 0)); }

            if (ctx.mData.empty() == false)
                { out.push_buffer(ctx.mData); }
        }

        return true;
    }
}
