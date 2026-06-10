/**
 * @file    G10.ASM.Linker.cpp
 * @brief   Contains implementations for the G10 Assembler's linker component
 *          and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Linker.hpp>

// Static Constants ************************************************************

namespace G10::ASM
{
    static const std::unordered_map<SectionName, std::uint32_t> kBases = {
        { SectionName::METADATA,    0x00000000 },
        { SectionName::INT0,        0x00001000 },
        { SectionName::INT1,        0x00001080 },
        { SectionName::INT2,        0x00001100 },
        { SectionName::INT3,        0x00001180 },
        { SectionName::INT4,        0x00001200 },
        { SectionName::INT5,        0x00001280 },
        { SectionName::INT6,        0x00001300 },
        { SectionName::INT7,        0x00001380 },
        { SectionName::INT8,        0x00001400 },
        { SectionName::INT9,        0x00001480 },
        { SectionName::INT10,       0x00001500 },
        { SectionName::INT11,       0x00001580 },
        { SectionName::INT12,       0x00001600 },
        { SectionName::INT13,       0x00001680 },
        { SectionName::INT14,       0x00001700 },
        { SectionName::INT15,       0x00001780 },
        { SectionName::INT16,       0x00001800 },
        { SectionName::INT17,       0x00001880 },
        { SectionName::INT18,       0x00001900 },
        { SectionName::INT19,       0x00001980 },
        { SectionName::INT20,       0x00001A00 },
        { SectionName::INT21,       0x00001A80 },
        { SectionName::INT22,       0x00001B00 },
        { SectionName::INT23,       0x00001B80 },
        { SectionName::INT24,       0x00001C00 },
        { SectionName::INT25,       0x00001C80 },
        { SectionName::INT26,       0x00001D00 },
        { SectionName::INT27,       0x00001D80 },
        { SectionName::INT28,       0x00001E00 },
        { SectionName::INT29,       0x00001E80 },
        { SectionName::INT30,       0x00001F00 },
        { SectionName::INT31,       0x00001F80 },
        { SectionName::CODE,        0x00002000 }, // `SectionName::DATA` also shares this address space.
        { SectionName::BSS,         0x80000000 }
    };
}

// Static Functions ************************************************************

namespace G10::ASM
{
    static auto ResolveSectionName (const ProgramSectionEntry& pEntry)
        -> SectionName
    {
        switch (pEntry.mType)
        {
            case ProgramSectionType::Metadata:  return SectionName::METADATA;
            case ProgramSectionType::Code:      return SectionName::CODE;
            case ProgramSectionType::Data:      return SectionName::CODE;
            case ProgramSectionType::BSS:       return SectionName::BSS;
            case ProgramSectionType::Interrupt:
                switch (pEntry.mInterruptNumber & 0x1F)
                {
                    case 0: return SectionName::INT0;
                    case 1: return SectionName::INT1;
                    case 2: return SectionName::INT2;
                    case 3: return SectionName::INT3;
                    case 4: return SectionName::INT4;
                    case 5: return SectionName::INT5;
                    case 6: return SectionName::INT6;
                    case 7: return SectionName::INT7;
                    case 8: return SectionName::INT8;
                    case 9: return SectionName::INT9;
                    case 10: return SectionName::INT10;
                    case 11: return SectionName::INT11;
                    case 12: return SectionName::INT12;
                    case 13: return SectionName::INT13;
                    case 14: return SectionName::INT14;
                    case 15: return SectionName::INT15;
                    case 16: return SectionName::INT16;
                    case 17: return SectionName::INT17;
                    case 18: return SectionName::INT18;
                    case 19: return SectionName::INT19;
                    case 20: return SectionName::INT20;
                    case 21: return SectionName::INT21;
                    case 22: return SectionName::INT22;
                    case 23: return SectionName::INT23;
                    case 24: return SectionName::INT24;
                    case 25: return SectionName::INT25;
                    case 26: return SectionName::INT26;
                    case 27: return SectionName::INT27;
                    case 28: return SectionName::INT28;
                    case 29: return SectionName::INT29;
                    case 30: return SectionName::INT30;
                    case 31: return SectionName::INT31;
                    default: return SectionName::INT0;
                } break;
            default:
                return SectionName::CODE;
        }
    }
}

// Public - Constructors & Destructor ******************************************

namespace G10::ASM
{
    Linker::Linker (Diagnostic& pDiag) :
        mDiag   { pDiag }
    {

    }
}

// Public Methods **************************************************************

namespace G10::ASM
{
    auto Linker::Link (const std::vector<std::string>& pFiles) -> bool
    {
        for (const auto& filename : pFiles)
        {
            if (LoadObject(filename) == false)
                { return false; }
        }

        BuildSourceSectionList();
        if (BuildProgramSectionEntries() == false)
            { return false; }

        BuildExportMap();
        return
            BuildSymbolTable() &&
            AssignTargetAddresses() &&
            ComputeLayout() &&
            ApplyRelocations() &&
            IdentifyEntryPoint();
    }

    auto Linker::SaveImage (const fs::path& pPath) -> bool
    {
        // Open file.
        std::fstream file { pPath, std::ios::out | std::ios::binary };
        if (file.is_open() == false)
        {
            mDiag.ReportError("Failed to open image file '{}' for writing.", 
                pPath.string());
            return false;
        }

        std::vector<std::uint8_t> image {};
        stx::byte_view view { image };
        image.resize(mImageSize, 0x00);

        // Program Header
        view.push<ProgramHeader>(mHeader);

        // Section Table
        for (const auto& section : mSections)
            { view.push<ProgramSectionEntry>(section); }

        // Symbol Table
        for (const auto& symbol : mSymbols)
            { view.push<ProgramSymbolEntry>(symbol); }

        // Section Data
        for (std::size_t i = 0; i < mSections.size(); ++i)
        {
            const auto& section = mSections[i];
            const auto& sectionData = mSectionData[i];
            if (section.mType == ProgramSectionType::BSS ||
                section.mDataOffset == 0)
            {
                // BSS section.
                continue;
            }

            if (sectionData.size() + section.mDataOffset > mImageSize)
            {
                mDiag.ReportError("Section {} data overflows image.", i);
                return false;
            }

            view.set_write_ptr(section.mDataOffset);
            view.write_buffer(sectionData);
        }

        file.write(reinterpret_cast<const char*>(image.data()), image.size());
        return true;
    }
}

// Private Methods *************************************************************

namespace G10::ASM
{
    auto Linker::LoadObject (const std::string& pFilename) -> bool
    {
        auto resolved = NormalizePath(pFilename);
        if (fs::exists(resolved) == false)
        {
            mDiag.ReportError("Object file '{}' not found.", pFilename);
            return false;
        }

        if (mObjects.contains(resolved.string()))
        {
            return true;
        }

        Object obj;
        auto loadFile = obj.LoadFile(resolved);
        if (loadFile.has_value() == false)
        {
            mDiag.ReportError("Failed to load object file '{}'.", pFilename);
            mDiag.ReportInfo("'{}'", loadFile.error());
            return false;
        }
        
        mObjects[resolved.string()] = std::move(obj);
        return true;
    }

    auto Linker::BuildSourceSectionList () -> void
    {
        // Build a quick list of object paths, in the same order as the provided
        // `pObjects` iteration.
        mSourceSections.clear();
        mSourceSections.reserve(mObjects.size() * 2);
        for (const auto& [path, obj] : mObjects)
        {
            const auto& sections = obj.GetSectionTable();
            const auto& data = obj.GetDataTable();
            for (std::size_t i = 0; i < sections.size(); ++i)
            {
                auto& emplace = mSourceSections.emplace_back(SourceSection {
                    .mObjectPath = path,
                    .mObjectSectionIndex = static_cast<std::uint32_t>(i),
                    .mHeader = sections[i]
                });

                if (i < data.size())
                    { emplace.mData = data[i]; }
            }
        }
    }

    auto Linker::BuildProgramSectionEntries () -> bool
    {
        std::unordered_set<std::uint8_t> usedInterrupts {};

        mSectionIndexMap.clear();
        mSections.clear();
        mSections.reserve(mSourceSections.size());
        for (std::size_t i = 0; i < mSourceSections.size(); ++i)
        {
            const auto& section = mSourceSections[i];

            ProgramSectionEntry entry {};
            entry.mType = static_cast<ProgramSectionType>(section.mHeader.mType);
            entry.mInterruptNumber = section.mHeader.mInterruptNumber;

            if (entry.mType == ProgramSectionType::Interrupt)
            {
                if (usedInterrupts.contains(entry.mInterruptNumber))
                {
                    SourceLocation loc { section.mObjectPath, 0, 0 };
                    mDiag.ReportError(loc, 
                        "Encountered duplicate interrupt section #{}.",
                        entry.mInterruptNumber);
                    return false;
                }

                usedInterrupts.insert(entry.mInterruptNumber);
            }

            entry.mAlignmentBoundary = section.mHeader.mAlignmentBoundary;
            entry.mTargetAddress = section.mHeader.mTargetAddress;
            entry.mDataSize = section.mHeader.mDataSize;
            entry.mDataOffset = 0;

            mSections.push_back(std::move(entry));
            mSectionIndexMap[{section.mObjectPath, section.mObjectSectionIndex}] 
                = static_cast<std::uint32_t>(i);
        }

        return true;
    }

    auto Linker::BuildExportMap () -> void
    {
        mExportMap.clear();
        for (const auto& [path, obj] : mObjects)
        {
            const auto& symbolTable = obj.GetSymbolTable();
            for (std::size_t i = 0; i < symbolTable.size(); ++i)
            {
                const auto& symbol = symbolTable[i];
                if (symbol.mType == ObjectSymbolType::Export)
                {
                    auto name = obj.LookupStringByOffset(symbol.mNameStringOffset);
                    if (name.has_value() == true)
                    { 
                        mExportMap[*name] = 
                            { path, static_cast<std::uint32_t>(i) }; 
                    }
                }
            }
        }
    }

    auto Linker::BuildSymbolTable () -> bool
    {
        for (const auto& [path, obj] : mObjects)
        {
            const auto& symbolTable = obj.GetSymbolTable();
            for (std::size_t i = 0; i < symbolTable.size(); ++i)
            {
                const auto& symbol = symbolTable[i];
                ProgramSymbolEntry entry {};

                if (symbol.mType == ObjectSymbolType::Import)
                {
                    SourceLocation loc { path, 0, 0 };
                    auto name = obj.LookupStringByOffset(symbol.mNameStringOffset);
                    if (name.has_value() == false)
                    {
                        mDiag.ReportError(loc, "Encountered un-named import symbol.");
                        mDiag.ReportInfo("While building symbol table.");
                        mDiag.ReportInfo("Looking up name string offset: {}", symbol.mNameStringOffset);
                        return false;
                    }

                    const auto findIt = mExportMap.find(*name);
                    if (findIt == mExportMap.end())
                    {
                        mDiag.ReportError(loc, 
                            "Encountered unresolved import symbol '{}'.", *name);
                        mDiag.ReportInfo("While building symbol table.");
                        return false;
                    }

                    const auto& [objectPath, symbolIndex] = findIt->second;
                    const auto& definedObject = mObjects[objectPath];
                    const auto& definedSymbol = 
                        definedObject.GetSymbolTable()[symbolIndex];

                    // Map to the program section which contains our definition.
                    entry.mSectionIndex = 
                        mSectionIndexMap[{objectPath, symbolIndex}];
                    entry.mAddressOffset = definedSymbol.mAddressOffset;
                }
                else
                {
                    // This is a local or exported symbol.
                    entry.mSectionIndex = 
                        mSectionIndexMap[{path, symbol.mSectionIndex}];
                    entry.mAddressOffset = symbol.mAddressOffset;
                }

                mSymbols.push_back(std::move(entry));
            }
        }

        return true;
    }

    auto Linker::AssignTargetAddresses () -> bool
    {
        std::unordered_map<SectionName, std::uint32_t> cursors = kBases;
        for (auto& section : mSections)
        {
            auto sectionName = ResolveSectionName(section);
            auto& cursor = cursors[sectionName];

            if (section.mTargetAddress != 0xFFFFFFFF)
            {
                // Respect existing address.
                cursor = section.mTargetAddress;
                continue;
            }

            std::uint32_t
                boundary = static_cast<std::uint32_t>(section.mAlignmentBoundary),
                align = std::max(boundary, 1u),
                padding = (align - (cursor % align)) % align;

            if (sectionName == SectionName::BSS)
            {
                std::uint64_t check = static_cast<std::uint64_t>(cursor);
                if (check + padding + section.mDataSize > 0xFFFFFFFF)
                {
                    mDiag.ReportError("Detected BSS section overflow.");
                    mDiag.ReportInfo("While assigning target addresses.");
                    return false;
                }
            }
            else
            {
                auto nextSection = static_cast<SectionName>(
                    std::to_underlying(sectionName) + 1
                );
                if (nextSection == SectionName::DATA)
                    { nextSection = SectionName::BSS; }

                if (cursor + padding + section.mDataSize >= kBases.at(nextSection))
                {
                    mDiag.ReportError("Detected section overflow.");
                    mDiag.ReportInfo("While assigning target addresses.");
                    return false;
                }
            }

            cursor += padding;
            section.mTargetAddress = cursor;
            cursor += section.mDataSize;
        }

        // Iterate and fix overlaps by repeatedly scanning ordered sections.
        // Strategy: sort sections by start address; when an overlap is found,
        // move the non-explicit section forward (after the anchor) while
        // respecting alignment and region bounds. If both are explicit, fail.
        if (!mSections.empty())
        {
            bool changed = true;
            while (changed)
            {
                changed = false;
                std::vector<std::size_t> order(mSections.size());
                for (std::size_t i = 0; i < order.size(); ++i) order[i] = i;
                std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b)
                {
                    return mSections[a].mTargetAddress < mSections[b].mTargetAddress;
                });

                for (std::size_t oi = 0; oi < order.size(); ++oi)
                {
                    const auto i = order[oi];
                    const std::uint32_t iStart = mSections[i].mTargetAddress;
                    const std::uint32_t iEnd = iStart + mSections[i].mDataSize;

                    for (std::size_t oj = oi + 1; oj < order.size(); ++oj)
                    {
                        const auto j = order[oj];
                        const std::uint32_t jStart = mSections[j].mTargetAddress;
                        const std::uint32_t jEnd = jStart + mSections[j].mDataSize;

                        if (jStart >= iEnd) // no overlap (and, because ordered, no further overlaps with i)
                            break;

                        // overlap detected between i and j
                        const bool iSpecified = (i < mSourceSections.size() &&
                            mSourceSections[i].mHeader.mTargetAddress != stx::npos32);
                        const bool jSpecified = (j < mSourceSections.size() &&
                            mSourceSections[j].mHeader.mTargetAddress != stx::npos32);

                        SourceLocation loc { mSourceSections[j].mObjectPath, 0, 0 };
                        if (iSpecified && jSpecified)
                        {
                            // Both sections requested explicit addresses — warn that
                            // we'll adjust the later one to avoid overlap. (`-Wextra` only).
                            mDiag.ReportExtraWarning(loc,
                                "Overlapping explicit section addresses: sections "
                                "{0} and {1} overlap; adjusting later section {1}.",
                                i, j);
                        }

                        // Choose anchor (the section we won't move) and the mover.
                        std::size_t anchor = i;
                        std::size_t mover = j;
                        if (!iSpecified && jSpecified)
                        {
                            anchor = j; mover = i;
                        }

                        // Compute new start for mover: align anchor end up to mover alignment.
                        const std::uint32_t anchorEnd = 
                            mSections[anchor].mTargetAddress + 
                            mSections[anchor].mDataSize;
                        const std::uint32_t boundary = 
                            static_cast<std::uint32_t>(
                                mSections[mover].mAlignmentBoundary);
                        const std::uint32_t align = std::max(boundary, 1u);
                        const std::uint32_t pad = 
                            (align - (anchorEnd % align)) % align;
                        const std::uint64_t newStart64 = 
                            static_cast<std::uint64_t>(anchorEnd) + pad;

                        // Region bounds check for mover
                        const auto moverName = ResolveSectionName(mSections[mover]);
                        if (moverName == SectionName::BSS)
                        {
                            if (newStart64 + mSections[mover].mDataSize > 0xFFFFFFFFull)
                            {
                                mDiag.ReportError(loc, 
                                    "Cannot relocate section {}: "
                                    "BSS overflow would occur.", mover);
                                mDiag.ReportInfo("While assigning target addresses.");
                                return false;
                            }
                        }
                        else
                        {
                            auto nextSection = 
                                static_cast<SectionName>(std::to_underlying(moverName) + 1);
                            if (nextSection == SectionName::DATA) nextSection = SectionName::BSS;
                            const std::uint32_t limit = kBases.at(nextSection);
                            if (newStart64 + mSections[mover].mDataSize >= limit)
                            {
                                mDiag.ReportError(loc, 
                                    "Cannot relocate section {}: "
                                    "would overflow its region.", mover);
                                mDiag.ReportInfo("While assigning target addresses.");
                                return false;
                            }
                        }
                        
                        mSections[mover].mTargetAddress = static_cast<std::uint32_t>(newStart64);
                        changed = true;
                        break; // restart scanning after resort
                    }

                    if (changed) break;
                }
            }
        }

        return true;
    }

    auto Linker::ComputeLayout () -> bool
    {
        const std::uint32_t 
            headerSize = sizeof(ProgramHeader),
            sectionEntrySize = sizeof(ProgramSectionEntry),
            symbolEntrySize = sizeof(ProgramSymbolEntry);

        const std::uint32_t 
            sectionTableOffset = headerSize,
            sectionTableSize = static_cast<std::uint32_t>(mSections.size() * sectionEntrySize),
            symbolTableOffset = sectionTableOffset + sectionTableSize,
            symbolTableSize = static_cast<std::uint32_t>(mSymbols.size() * symbolEntrySize),
            dataBufferOffset = symbolTableOffset + symbolTableSize;
        std::uint32_t dataOffset = dataBufferOffset;

        // Prepare per-section data buffers
        mSectionData.clear();
        mSectionData.resize(mSections.size());

        for (std::size_t i = 0; i < mSections.size(); ++i)
        {
            auto& section = mSections[i];
            auto& sectionData = mSectionData[i];

            // Copy the section bytes from the corresponding source section.
            // `mSourceSections` was built in the same order as `mSections`.
            if (
                section.mType == ProgramSectionType::BSS || 
                section.mDataSize == 0)
            {
                sectionData.clear();
                section.mDataOffset = 0;
                continue;
            }

            std::uint32_t align = 
                std::max(static_cast<std::uint32_t>(section.mAlignmentBoundary), 1u);
            std::uint32_t pad = (align - (dataOffset % align)) % align;
            dataOffset += pad;
            section.mDataOffset = dataOffset;

            if (i < mSourceSections.size())
                { sectionData = mSourceSections[i].mData; }
            else
                { sectionData.clear(); }

            // Ensure the vector is the declared size (pad with zeros if necessary)
            if (sectionData.size() < section.mDataSize)
                { sectionData.resize(section.mDataSize, 0); }

            dataOffset += section.mDataSize;
        }

        // Prepare the program header, and fill it with the offsets and counts
        mHeader.mMagicNumber = kProgramMagicNumber;
        mHeader.mVersion = kProgramVersion;
        mHeader.mFlags = ProgramFlags::None;
        mHeader.mSectionTableOffset = sectionTableOffset;
        mHeader.mSectionCount = static_cast<std::uint32_t>(mSections.size());
        mHeader.mSymbolTableOffset = symbolTableOffset;
        mHeader.mSymbolCount = static_cast<std::uint32_t>(mSymbols.size());
        mHeader.mDataBufferOffset = dataBufferOffset;
        mImageSize = dataOffset;
        
        return true;
    }

    auto Linker::ApplyRelocations () -> bool
    {
        for (const auto& [path, obj] : mObjects)
        {
            SourceLocation loc { path, 0, 0 };
            const auto& relocTable = obj.GetRelocationTable();
            const auto& symbolTable = obj.GetSymbolTable();
            for (const auto& reloc : relocTable)
            {
                // Identify the patch location's section idnex.
                const auto patchKey = std::make_pair(path, reloc.mSectionIndex);
                auto findIt = mSectionIndexMap.find(patchKey);
                if (findIt == mSectionIndexMap.end())
                {
                    mDiag.ReportError(loc,
                        "Relocation refers to unknown section.");
                    mDiag.ReportInfo("While applying relocations.");
                    return false;
                }
                
                const auto patchIndex = findIt->second;
                if (patchIndex >= mSectionData.size())
                {
                    mDiag.ReportError(loc,
                        "Relocation patch index {} is out of bounds.", patchIndex);
                    mDiag.ReportInfo("While applying relocations.");
                    return false;
                }

                // Resolve the symbol's program section and address offset.
                if (reloc.mSymbolIndex >= symbolTable.size())
                {
                    mDiag.ReportError(loc, 
                        "Relocation references invalid symbol index {}.", 
                        reloc.mSymbolIndex);
                        mDiag.ReportInfo("While applying relocations.");
                    return false;
                }

                const auto& symbol = symbolTable[reloc.mSymbolIndex];
                std::uint32_t 
                    targetIndex = stx::npos32,
                    targetAddressOffset = 0;
                if (symbol.mType == ObjectSymbolType::Import)
                {
                    auto name = obj.LookupStringByOffset(symbol.mNameStringOffset);
                    if (name.has_value() == false)
                    {
                        mDiag.ReportError(loc, 
                            "Relocation references unnamed import.");
                        mDiag.ReportInfo("While applying relocations.");
                        return false;
                    }

                    const auto findExport = mExportMap.find(*name);
                    if (findExport == mExportMap.end())
                    {
                        mDiag.ReportError(loc, 
                            "Relocation references unresolved import '{}'.", 
                            *name);
                        mDiag.ReportInfo("While applying relocations.");
                        return false;
                    }

                    const auto& [objectPath, symbolIndex] = findExport->second;
                    const auto& defObj = mObjects.at(objectPath);
                    const auto& defSym = defObj.GetSymbolTable()[symbolIndex];

                    // Map the definition's section index to a program section index.
                    auto mapIt = 
                    mSectionIndexMap.find({ objectPath, defSym.mSectionIndex });
                    if (mapIt == mSectionIndexMap.end())
                    {
                        mDiag.ReportError(loc, 
                            "Relocation: definition section not present for '{}'.", 
                            *name);
                        return false;
                    }

                    targetIndex = mapIt->second;
                    targetAddressOffset = defSym.mAddressOffset;
                }   
                else
                {
                    targetIndex = mSectionIndexMap[{path, symbol.mSectionIndex}];
                    targetAddressOffset = symbol.mAddressOffset;
                }     

                if (targetIndex >= mSections.size())
                {
                    mDiag.ReportError(loc, 
                        "Relocation: target section index {} out of range.", 
                        targetIndex);
                    mDiag.ReportInfo("While applying relocations.");
                    return false;
                }

                // Compute addresses
                const std::uint32_t 
                    symbolAbsolute = mSections[targetIndex].mTargetAddress + targetAddressOffset,
                    patchLocationAddr = mSections[patchIndex].mTargetAddress + reloc.mPatchOffset;

                std::uint64_t value = 0;
                if (reloc.mType == ObjectRelocationType::Absolute)
                {
                    value = symbolAbsolute;
                }
                else // Relative
                {
                    // displacement = symbol - (patch + size)
                    value = static_cast<std::uint32_t>(symbolAbsolute - 
                        (patchLocationAddr + reloc.mSize));
                }

                // Bounds check
                if (reloc.mPatchOffset + reloc.mSize > mSectionData[patchIndex].size())
                {
                    mDiag.ReportError(loc, 
                        "Relocation patch would overflow section data.");
                    mDiag.ReportInfo("While applying relocations.");
                    return false;
                }

                // Write little-endian value into the section data
                stx::byte_view view { mSectionData[patchIndex] };
                view.set_write_ptr(reloc.mPatchOffset);
                switch (reloc.mSize)
                {
                    case 1: view.write_byte(value & 0xFF); break;
                    case 2: view.write_word_le(value & 0xFFFF); break;
                    case 4: view.write_dword_le(value & 0xFFFFFFFF); break;
                }
            }
        }

        return true;
    }

    auto Linker::IdentifyEntryPoint () -> bool
    {
        bool foundEntryPoint = false;
        const std::array<std::string, 3> candidates = { 
            "main", 
            "start", 
            "_start" 
        };

        for (const auto& name : candidates)
        {
            const auto findIt = mExportMap.find(name);
            if (findIt == mExportMap.end())
                { continue; }

            const auto& [objectPath, symbolIndex] = findIt->second;
            const auto objIt = mObjects.find(objectPath);
            if (objIt == mObjects.end())
            {
                mDiag.ReportError("Entry point resolution: exported symbol '{}' references "
                    "missing object '{}'.", name, objectPath);
                mDiag.ReportInfo("While identifying program entry point.");
                return false;
            }

            const auto& defObj = objIt->second;
            const auto& defSymbols = defObj.GetSymbolTable();
            if (symbolIndex >= defSymbols.size())
            {
                mDiag.ReportError("Entry point resolution: exported symbol '{}' references "
                    "invalid symbol index {}.", name, symbolIndex);
                mDiag.ReportInfo("While identifying program entry point.");
                return false;
            }

            const auto& defSym = defSymbols[symbolIndex];

            // Map object's section index to program section index
            const auto mapIt = mSectionIndexMap.find({ objectPath, defSym.mSectionIndex });
            if (mapIt == mSectionIndexMap.end())
            {
                mDiag.ReportError("Entry point resolution: symbol '{}' definition "
                    "section not present in program image.", name);
                mDiag.ReportInfo("While identifying program entry point.");
                return false;
            }

            const auto programSectionIndex = mapIt->second;
            if (programSectionIndex >= mSections.size())
            {
                mDiag.ReportError("Entry point resolution: program section "
                    "index {} out of range for '{}'.", programSectionIndex, name);
                mDiag.ReportInfo("While identifying program entry point.");
                return false;
            }

            // Compute absolute entry address
            const std::uint32_t entryAddress = 
                mSections[programSectionIndex].mTargetAddress + defSym.mAddressOffset;
            mHeader.mEntryPoint = entryAddress;
            foundEntryPoint = true;
            break;
        }

        if (!foundEntryPoint)
        {
            mDiag.ReportError("Entry point resolution: no valid entry point found.");
            return false;
        }
        
        return true;
    }
}
