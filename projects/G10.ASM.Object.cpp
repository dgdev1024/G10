/**
 * @file    G10.ASM.Object.cpp
 * @brief   Contains implementations for the G10 Assembler's object file class,
 *          and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Object.hpp>

// Static - Helper Functions ***************************************************

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

}

// Public - Constructors & Destructor ******************************************

namespace G10::ASM
{

    Object::Object ()
        { mStringTable.push_back(""); }

}

// Public Methods **************************************************************

namespace G10::ASM
{

    auto Object::LoadFile (const fs::path& pPath) -> stx::expect_good
    {
        auto resolved = NormalizePath(pPath);
        if (fs::exists(resolved) == false)
        {
            return stx::fmt_unexpected(
                "LoadFile: Object file '{}' not found.",
                resolved.string()
            );
        }

        auto file_size = fs::file_size(resolved);
        std::fstream file { pPath, std::ios::binary | std::ios::in };
        if (file.is_open() == false)
        {
            return stx::fmt_unexpected(
                "LoadFile: Could not open object file '{}' for reading.",
                resolved.string()
            );
        }

        std::vector<std::uint8_t> buffer;
        buffer.resize(file_size);
        file.read(reinterpret_cast<char*>(buffer.data()), file_size);
        file.close();

        return LoadBuffer(buffer);
    }
    
    auto Object::LoadBuffer (std::vector<std::uint8_t>& pBuffer) -> stx::expect_good
    {
        // 1. Compute exact wire sies.
        std::size_t
            headerSize = ComputeSize<ObjectHeader>(mHeader),
            sectionEntrySize = ComputeSize<ObjectSectionEntry>(),
            symbolEntrySize = ComputeSize<ObjectSymbolEntry>(),
            relocationEntrySize = ComputeSize<ObjectRelocationEntry>();
            
        // 2. Begin by reading the object file's header.
        stx::byte_view view { pBuffer };
        mHeader = view.read<ObjectHeader>();

        // 3. Next, read the section table.
        mSectionTable.resize(mHeader.mSectionCount);
        for (std::size_t i = 0; i < mHeader.mSectionCount; ++i)
            { mSectionTable[i] = view.read<ObjectSectionEntry>(); }

        // 4. Next, read the symbol table.
        mSymbolTable.resize(mHeader.mSymbolCount);
        for (std::size_t i = 0; i < mHeader.mSymbolCount; ++i)
            { mSymbolTable[i] = view.read<ObjectSymbolEntry>(); }

        // 5. Next, read the relocation table.
        mRelocationTable.resize(mHeader.mRelocationCount);
        for (std::size_t i = 0; i < mHeader.mRelocationCount; ++i)
            { mRelocationTable[i] = view.read<ObjectRelocationEntry>(); }

        // 6. Read the string table.
        mStringTable.resize(mHeader.mStringCount + 1);  // Plus one to account for the index-zero null string.
        for (std::size_t i = 0; i <= mHeader.mStringCount; ++i)
            { mStringTable[i] = view.read_string(); }

        // 7. Read the data table.
        mDataTable.clear();
        for (const auto& section : mSectionTable)
        {
            std::vector<std::uint8_t> buffer {};
            if (section.mType != ObjectSectionType::BSS && section.mDataSize > 0)
            {
                buffer = view.read_buffer(section.mDataSize);
            }
            mDataTable.push_back(std::move(buffer));
        }

        return {};
    }

}

// Public Methods **************************************************************

namespace G10::ASM
{

    auto Object::LookupString (std::uint32_t pIndex) const 
        -> stx::optional_ref<const std::string>
    {
        if (pIndex >= mStringTable.size())
            { return std::nullopt; }

        return mStringTable[pIndex];
    }

    auto Object::LookupStringByOffset (std::uint32_t pOffset) const
        -> stx::optional_ref<const std::string>
    {
        std::uint32_t cursor = 0;
        for (const auto& str : mStringTable)
        {
            if (cursor == pOffset)
                { return str; }
                
            cursor += str.size() + 1;  // +1 for the null terminator
        }

        return std::nullopt;
    }

}

// Private Methods *************************************************************

namespace G10::ASM
{
    


}
