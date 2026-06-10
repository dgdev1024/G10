/**
 * @file    G10.ASM.Codegen.hpp
 * @brief   Contains declarations for the G10 Assembler's code generation
 *          unit, and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.ASM.Common.hpp>
#include <G10.ASM.Diagnostic.hpp>
#include <G10.ASM.Syntax.hpp>

// Constants & Enumerations ****************************************************

namespace G10::ASM
{
    constexpr std::uint32_t
        kObjectMagicNumber  = 0x4731304F;       // ASCII "G10O"
    constexpr std::uint32_t
        kObjectMajorVersion = 1,
        kObjectMinorVersion = 0,
        kObjectPatchVersion = 0,
        kObjectVersion =
            (kObjectMajorVersion * 10000) +
            (kObjectMinorVersion * 100) +
            kObjectPatchVersion;

    enum class ObjectFlags : std::uint32_t
    {
        None                = 0
    };
    G10_BITFIELD_ENUM(ObjectFlags)

    enum class ObjectSectionType : std::uint8_t
    {
        Metadata,
        Interrupt,
        Code,
        Data,
        BSS
    };

    enum class ObjectSymbolType : std::uint8_t
    {
        Local,
        Export,
        Import
    };

    enum class ObjectRelocationType : std::uint8_t
    {
        Absolute,
        Relative
    };
}

// Structures ******************************************************************

namespace G10::ASM
{
    struct ObjectHeader final
    {
        std::uint32_t       mMagicNumber { kObjectMagicNumber };
        std::uint32_t       mVersion { kObjectVersion };
        ObjectFlags         mFlags { };
        std::uint32_t       mSectionTableOffset { 0 };
        std::uint32_t       mSectionCount { 0 };
        std::uint32_t       mSymbolTableOffset { 0 };
        std::uint32_t       mSymbolCount { 0 };
        std::uint32_t       mRelocationTableOffset { 0 };
        std::uint32_t       mRelocationCount { 0 };
        std::uint32_t       mStringTableOffset { 0 };
        std::uint32_t       mStringBufferSize { 0 };
        std::uint32_t       mStringCount { 0 };
        std::uint32_t       mDataBufferOffset { 0 };
    };

    struct ObjectSectionEntry final
    {
        std::uint32_t       mNameStringOffset { 0 };
        ObjectSectionType   mType;
        std::uint8_t        mInterruptNumber { 0 };
        std::uint16_t       mAlignmentBoundary { 0 };
        std::uint32_t       mTargetAddress { 0 };
        std::uint32_t       mDataSize { 0 };
        std::uint32_t       mDataOffset { 0 };
    };

    struct ObjectSymbolEntry final
    {
        std::uint32_t       mNameStringOffset { 0 };
        ObjectSymbolType    mType;
        std::uint32_t       mAddressOffset { 0 };
        std::uint32_t       mSectionIndex { 0 };
    };

    struct ObjectRelocationEntry final
    {
        std::uint32_t           mPatchOffset { 0 };
        std::uint32_t           mSymbolIndex { 0 };
        std::uint32_t           mSectionIndex { 0 };
        ObjectRelocationType    mType;
        std::uint8_t            mSize;
    };
}

// Classes *********************************************************************

namespace G10::ASM
{
    class G10_API Codegen final
    {
        friend class Object;

    private: // Structures *****************************************************

        struct ObjectSectionContext final
        {
            ObjectSectionEntry          mHeader {}; 
            std::vector<std::uint8_t>   mData {};               // Non-BSS: Section's data contents.
            std::uint32_t               mLocationCounter { 0 };
        };

    public: // Constructors & Destructor ***************************************

        Codegen (Diagnostic& pDiag);

    public: // Methods - Input & Output ****************************************

        auto Run (const SyntaxModule& pModule) -> bool;
        auto GetOutput () -> std::vector<std::uint8_t>&;
        auto GetOutput () const -> const std::vector<std::uint8_t>&;
        auto SaveOutput (const fs::path& pPath) -> bool;

    public: // Methods - Options ***********************************************

        auto SetIncludeDirectories (const std::vector<std::string>& pIncludeDirs) -> void;

    private: // Methods - Dispatch *********************************************

        auto Dispatch (const std::shared_ptr<SyntaxNode>& pNode) -> bool;
        auto DispatchByteDirective (const ByteDirectiveNode& pNode) -> bool;
        auto DispatchWordDirective (const WordDirectiveNode& pNode) -> bool;
        auto DispatchDoubleWordDirective (const DoubleWordDirectiveNode& pNode) -> bool;
        auto DispatchStringDirective (const StringDirectiveNode& pNode) -> bool;
        auto DispatchSpaceDirective (const SpaceDirectiveNode& pNode) -> bool;
        auto DispatchIncbinDirective (const IncbinDirectiveNode& pNode) -> bool;
        auto DispatchExportDirective (const ExportDirectiveNode& pNode) -> bool;
        auto DispatchImportDirective (const ImportDirectiveNode& pNode) -> bool;
        auto DispatchSectionDirective (const SectionDirectiveNode& pNode) -> bool;
        auto DispatchOrgDirective (const OrgDirectiveNode& pNode) -> bool;
        auto DispatchAlignDirective (const AlignDirectiveNode& pNode) -> bool;
        auto DispatchLabelStatement (const LabelStatementNode& pNode) -> bool;
        auto DispatchInstructionStatement (const InstructionStatementNode& pNode) -> bool;

    private: // Methods - Instruction Dispatch *********************************

        auto DispatchNOP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSTOP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchHALT (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchDI (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchEI (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchEII (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchDAA (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSCF (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchCCF (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchCLV (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSEV (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchREX (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchLEC (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchLD (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchLDQ (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchLDP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchST (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSTQ (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSTP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchMV (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchMWH (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchMWL (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchLSP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchPOP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSSP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchPUSH (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSPO (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSPI (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchJMP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchJPB (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchCALL (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchINT (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchRET (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchRETI (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchMFI (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchMFO (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchADD (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchADC (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSUB (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSBC (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchINC (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchDEC (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchAND (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchOR (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchXOR (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchNOT (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchCMP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSLA (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSRA (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSRL (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSWAP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchRLA (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchRL (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchRLCA (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchRLC (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchRRA (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchRR (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchRRCA (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchRRC (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchBIT (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSET (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchRES (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchTOG (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchLDI (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchLDD (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSTI (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchSTD (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchASP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchLASP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchISP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchDSP (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;
        auto DispatchASR (const InstructionStatementNode& pNode, ObjectSectionContext& pCtx) -> bool;

    private: // Methods - Evaluate *********************************************

        auto EvaluateIntegerExpression (const std::shared_ptr<ExpressionNode>& pNode)
            -> std::optional<std::uint32_t>;
        auto EvaluateStringExpression (const std::shared_ptr<ExpressionNode>& pNode)
            -> std::optional<std::string>;
        auto EvaluateRegisterExpression (const std::shared_ptr<ExpressionNode>& pNode)
            -> stx::optional_pair<std::string, CPU::Register>;
        auto EvaluateConditionExpression (const std::shared_ptr<ExpressionNode>& pNode)
            -> stx::optional_pair<std::string, CPU::Condition>;
        auto EvaluateLabelExpression (const std::shared_ptr<ExpressionNode>& pNode)
            -> stx::optional_pair<std::string, std::uint32_t>;
        auto EvaluateSectionNameExpression (const std::shared_ptr<ExpressionNode>& pNode)
            -> std::optional<SectionName>;
        auto EvaluateBinaryExpression (const std::shared_ptr<ExpressionNode>& pNode)
            -> std::optional<std::uint32_t>;
        auto EvaluatePointerExpression (const std::shared_ptr<ExpressionNode>& pNode)
            -> stx::optional_var<std::uint32_t, 
                std::pair<std::string, CPU::Register>, std::string>;

    private: // Methods - String Table *****************************************

        auto AddString (const std::string& pString) -> std::uint32_t;
        auto GetStringBuffer () const -> std::span<const char>;

    private: // Methods - Sections & Symbols ***********************************

        auto SetSection (
            const std::string&  pDisplayName, 
            ObjectSectionType   pType,
            std::uint8_t        pInterruptNumber = 0,
            std::uint32_t       pTargetAddress = stx::npos32,
            std::uint16_t       pAlignment = 1
        ) -> void;

        auto ResolveSymbol (const std::string& pName) -> std::uint32_t;

        auto AddRelocation (
            const std::uint32_t     pSymbolIndex,
            ObjectRelocationType    pType,
            std::uint8_t            pSize,
            std::uint32_t           pInitialValue = 0
        ) -> bool;

        auto AddRelocation (
            const std::string&      pSymbolName,
            ObjectRelocationType    pType,
            std::uint8_t            pSize,
            std::uint32_t           pInitialValue = 0
        ) -> bool;

        auto GetLastSectionOfType (ObjectSectionType pType)
            -> stx::optional_ref<ObjectSectionContext>;

    private: // Methods - Emission *********************************************

        auto EmitByte (std::uint8_t pByte) -> bool;
        auto EmitWord (std::uint16_t pWord) -> bool;
        auto EmitDoubleWord (std::uint32_t pDoubleWord) -> bool;
        auto EmitLabel (const std::string& pLabel, std::uint32_t pAddress) -> bool;
        auto EmitString (const std::string& pString, bool pNoTerminator = false) -> bool;
        auto EmitOpcode (std::uint16_t pOpcode) -> bool;
        auto EmitOpcode (std::uint8_t pOpcode, std::uint8_t pParamX, std::uint8_t pParamY) -> bool;

        auto ReserveBytes (std::uint32_t pCount) -> bool;
        auto ReserveWords (std::uint32_t pCount) -> bool;
        auto ReserveDoubleWords (std::uint32_t pCount) -> bool;

    private: // Methods - Build ************************************************

        auto BuildOutput () -> bool;

    private: // Members ********************************************************

        Diagnostic& mDiag;

        std::vector<fs::path>                           mIncludeDirs {};

        std::vector<char>                               mStringBuffer;
        std::unordered_map<std::string, std::uint32_t>  mStringLookup;

        std::uint32_t                                   mActiveSectionIndex { stx::npos32 };
        ObjectHeader                                    mHeader {};
        std::vector<ObjectSectionContext>               mSections {};
        std::vector<ObjectSymbolEntry>                  mSymbols {};
        std::vector<ObjectRelocationEntry>              mRelocations {};
        std::unordered_map<std::string, std::uint32_t>  mSymbolNameIndices {};
        std::vector<std::uint8_t>                       mOutput {};
        
    };
}
