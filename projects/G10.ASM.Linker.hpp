/**
 * @file    G10.ASM.Linker.hpp
 * @brief   Contains definitions for the G10 Assembler's linker component
 *          and related definitions.
 */

// Includes ********************************************************************

#include <G10.CPU.Program.hpp>
#include <G10.ASM.Object.hpp>

// Constants & Enumerations ****************************************************

namespace G10::ASM
{
    using   G10::CPU::kProgramMagicNumber,
            G10::CPU::kProgramMajorVersion,
            G10::CPU::kProgramMinorVersion,
            G10::CPU::kProgramPatchVersion,
            G10::CPU::kProgramVersion,
            G10::CPU::ProgramFlags,
            G10::CPU::ProgramSectionType;
}

// Structures ******************************************************************

namespace G10::ASM
{
    using   G10::CPU::ProgramHeader,
            G10::CPU::ProgramSectionEntry,
            G10::CPU::ProgramSymbolEntry;
}

// Classes *********************************************************************

namespace G10::ASM
{
    class G10_API Linker final
    {
    private: // Structures *****************************************************

        struct SourceSection final
        {
            std::string                 mObjectPath;
            std::uint32_t               mObjectSectionIndex { 0 };
            ObjectSectionEntry          mHeader {};
            std::vector<std::uint8_t>   mData {};        
        };

    public: // Constructors & Destructor ***************************************

        explicit Linker (Diagnostic& pDiag);

    public: // Methods *********************************************************

        auto Link (const std::vector<std::string>& pFiles) -> bool;
        auto SaveImage (const fs::path& pPath) -> bool;

    private: // Methods ********************************************************

        auto LoadObject (const std::string& pFilename) -> bool;
        auto BuildSourceSectionList () -> void;
        auto BuildProgramSectionEntries () -> bool;
        auto BuildExportMap () -> void;
        auto BuildSymbolTable () -> bool;
        auto AssignTargetAddresses () -> bool;
        auto ComputeLayout () -> bool;
        auto ApplyRelocations () -> bool;
        auto IdentifyEntryPoint () -> bool;

    private: // Members ********************************************************

        Diagnostic& mDiag;
        stx::dictionary<Object> mObjects {};

        ProgramHeader mHeader {};
        std::vector<ProgramSectionEntry> mSections {};
        std::vector<ProgramSymbolEntry> mSymbols {};
        stx::dual_vector<std::uint8_t> mSectionData {};
        std::vector<SourceSection> mSourceSections {};
        std::size_t mImageSize { 0 };

        std::map<
            std::pair<std::string, std::uint32_t>,
            std::uint32_t
        > mSectionIndexMap {};

        stx::dictionary<
            std::pair<std::string, std::uint32_t>
        > mExportMap {};

    };
}
