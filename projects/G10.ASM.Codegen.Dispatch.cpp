/**
 * @file    G10.ASM.Codegen.Dispatch.cpp
 * @brief   Contains implementations for the G10 Assembler Parser's
 *          dispatch methods, and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Codegen.hpp>

// Private Methods - Dispatch **************************************************

namespace G10::ASM
{
    auto Codegen::Dispatch (const std::shared_ptr<SyntaxNode>& pNode) -> bool
    {
        if (pNode == nullptr)
            { return false; }

        if (const auto ptr = stx::to<ByteDirectiveNode>(pNode))
            { return DispatchByteDirective(*ptr); }
        else if (const auto ptr = stx::to<WordDirectiveNode>(pNode))
            { return DispatchWordDirective(*ptr); }
        else if (const auto ptr = stx::to<DoubleWordDirectiveNode>(pNode))
            { return DispatchDoubleWordDirective(*ptr); }
        else if (const auto ptr = stx::to<StringDirectiveNode>(pNode))
            { return DispatchStringDirective(*ptr); }
        else if (const auto ptr = stx::to<SpaceDirectiveNode>(pNode))
            { return DispatchSpaceDirective(*ptr); }
        else if (const auto ptr = stx::to<IncbinDirectiveNode>(pNode))
            { return DispatchIncbinDirective(*ptr); }
        else if (const auto ptr = stx::to<ExportDirectiveNode>(pNode))
            { return DispatchExportDirective(*ptr); }
        else if (const auto ptr = stx::to<ImportDirectiveNode>(pNode))
            { return DispatchImportDirective(*ptr); }
        else if (const auto ptr = stx::to<SectionDirectiveNode>(pNode))
            { return DispatchSectionDirective(*ptr); }
        else if (const auto ptr = stx::to<OrgDirectiveNode>(pNode))
            { return DispatchOrgDirective(*ptr); }
        else if (const auto ptr = stx::to<AlignDirectiveNode>(pNode))
            { return DispatchAlignDirective(*ptr); }
        else if (const auto ptr = stx::to<LabelStatementNode>(pNode))
            { return DispatchLabelStatement(*ptr); }
        else if (const auto ptr = stx::to<InstructionStatementNode>(pNode))
            { return DispatchInstructionStatement(*ptr); }

        mDiag.ReportError(pNode->mLocation, "Un-implemented syntax node type.");
        return false;
    }

    auto Codegen::DispatchByteDirective (const ByteDirectiveNode& pNode) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError(pNode.mLocation, "'.BYTE' directive: No active section.");
            return false;
        }

        const auto& ctx = mSections[mActiveSectionIndex];

        if (ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            for (const auto& operand : pNode.mOperands)
            {
                if (operand == nullptr)
                {
                    mDiag.ReportError(pNode.mLocation, "'.BYTE' directive: Null operand.");
                    return false;
                }

                if (const auto i = EvaluateIntegerExpression(operand))
                {
                    if (ReserveBytes(*i) == false)
                        { return false; }
                }
            }
        }
        else if (
            ctx.mHeader.mType == ObjectSectionType::Data ||
            ctx.mHeader.mType == ObjectSectionType::Metadata
        )
        {
            for (const auto& operand : pNode.mOperands)
            {
                if (operand == nullptr)
                {
                    mDiag.ReportError(pNode.mLocation, "'.BYTE' directive: Null operand.");
                    return false;
                }

                if (const auto s = EvaluateStringExpression(operand))
                {
                    if (EmitString(*s, true) == false) 
                        { return false; }
                }
                else if (const auto i = EvaluateIntegerExpression(operand))
                {
                    if (EmitByte(*i & 0xFF) == false)
                        { return false; }
                }
                else if (const auto lx = stx::to<LabelExpressionNode>(operand))
                {
                    if (AddRelocation(lx->mSymbol, ObjectRelocationType::Absolute, 1) == false)
                        { return false; }
                }
                else
                {
                    mDiag.ReportError(pNode.mLocation, "'.BYTE' directive: Invalid operand.");
                    return false;
                }
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.BYTE' directive: Invalid section type.");
            return false;
        }

        return true;
    }

    auto Codegen::DispatchWordDirective (const WordDirectiveNode& pNode) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError(pNode.mLocation, "'.WORD' directive: No active section.");
            return false;
        }

        const auto& ctx = mSections[mActiveSectionIndex];
        if (ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            for (const auto& operand : pNode.mOperands)
            {
                if (operand == nullptr)
                {
                    mDiag.ReportError(pNode.mLocation, "'.WORD' directive: Null operand.");
                    return false;
                }

                if (const auto i = EvaluateIntegerExpression(operand))
                {
                    if (ReserveWords(*i) == false)
                        { return false; }
                }
            }
        }
        else if (
            ctx.mHeader.mType == ObjectSectionType::Data ||
            ctx.mHeader.mType == ObjectSectionType::Metadata
        )
        {
            for (const auto& operand : pNode.mOperands)
            {
                if (operand == nullptr)
                {
                    mDiag.ReportError(pNode.mLocation, "'.WORD' directive: Null operand.");
                    return false;
                }
    
                if (const auto i = EvaluateIntegerExpression(operand))
                {
                    if (EmitWord(*i & 0xFFFF) == false)
                        { return false; }
                }
                else if (const auto lx = stx::to<LabelExpressionNode>(operand))
                {
                    if (AddRelocation(lx->mSymbol, ObjectRelocationType::Absolute, 2) == false)
                        { return false; }
                }
                else
                {
                    mDiag.ReportError(pNode.mLocation, "'.WORD' directive: Invalid operand.");
                    return false;
                }
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.WORD' directive: Invalid section type.");
            return false;
        }

        return true;
    }

    auto Codegen::DispatchDoubleWordDirective (const DoubleWordDirectiveNode& pNode) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError(pNode.mLocation, "'.DWORD' directive: No active section.");
            return false;
        }

        const auto& ctx = mSections[mActiveSectionIndex];
        if (ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            for (const auto& operand : pNode.mOperands)
            {
                if (operand == nullptr)
                {
                    mDiag.ReportError(pNode.mLocation, "'.DWORD' directive: Null operand.");
                    return false;
                }

                if (const auto i = EvaluateIntegerExpression(operand))
                {
                    if (ReserveDoubleWords(*i) == false)
                        { return false; }
                }
            }
        }
        else if (
            ctx.mHeader.mType == ObjectSectionType::Data ||
            ctx.mHeader.mType == ObjectSectionType::Metadata
        )
        {
            for (const auto& operand : pNode.mOperands)
            {
                if (operand == nullptr)
                {
                    mDiag.ReportError(pNode.mLocation, "'.DWORD' directive: Null operand.");
                    return false;
                }
    
                if (const auto i = EvaluateIntegerExpression(operand))
                {
                    if (EmitDoubleWord(*i & 0xFFFFFFFF) == false)
                        { return false; }
                }
                else if (const auto lx = stx::to<LabelExpressionNode>(operand))
                {
                    if (AddRelocation(lx->mSymbol, ObjectRelocationType::Absolute, 4) == false)
                        { return false; }
                }
                else
                {
                    mDiag.ReportError(pNode.mLocation, "'.DWORD' directive: Invalid operand.");
                    return false;
                }
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.DWORD' directive: Invalid section type.");
            return false;
        }

        return true;
    }

    auto Codegen::DispatchStringDirective (const StringDirectiveNode& pNode) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError(pNode.mLocation, "'.STRING' directive: No active section.");
            return false;
        }

        const auto& ctx = mSections[mActiveSectionIndex];
        if (ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.STRING' directive: Not allowed in BSS section.");
            return false;
        }
        else if (
            ctx.mHeader.mType == ObjectSectionType::Code ||
            ctx.mHeader.mType == ObjectSectionType::Interrupt
        )
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.STRING' directive: Not allowed in Code or Interrupt section.");
            return false;
        }

        for (const auto& operand : pNode.mOperands)
        {
            if (operand == nullptr)
            {
                mDiag.ReportError(pNode.mLocation, "'.STRING' directive: Null operand.");
                return false;
            }

            if (const auto s = EvaluateStringExpression(operand))
            {
                if (EmitString(*s) == false)
                    { return false; }
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.STRING' directive: Invalid operand.");
                return false;
            }
        }

        return true;
    }

    auto Codegen::DispatchSpaceDirective (const SpaceDirectiveNode& pNode) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError(pNode.mLocation, "'.SPACE' directive: No active section.");
            return false;
        }
        
        std::uint32_t count = 0;
        for (const auto& operand : pNode.mOperands)
        {
            if (operand == nullptr)
            {
                mDiag.ReportError(pNode.mLocation, "'.SPACE' directive: Null operand.");
                return false;
            }

            if (const auto i = EvaluateIntegerExpression(operand))
            {
                count += *i;
            }
            else
            {
                mDiag.ReportError(pNode.mLocation, "'.SPACE' directive: Invalid operand.");
                return false;
            }
        }

        if (count == 0)
            { count = 1; }

        return ReserveBytes(count);
    }

    auto Codegen::DispatchIncbinDirective (const IncbinDirectiveNode& pNode) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError(pNode.mLocation, "'.INCBIN' directive: No active section.");
            return false;
        }

        auto& ctx = mSections[mActiveSectionIndex];
        if (ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.INCBIN' directive: Not allowed in BSS section.");
            return false;
        }
        else if (
            ctx.mHeader.mType == ObjectSectionType::Code ||
            ctx.mHeader.mType == ObjectSectionType::Interrupt
        )
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.INCBIN' directive: Not allowed in Code or Interrupt section.");
            return false;
        }

        auto filename = EvaluateStringExpression(pNode.mFilename);
        if (filename.has_value() == false)
        {
            mDiag.ReportError(pNode.mLocation, "'.INCBIN' directive: Filename must be a string.");
            return false;
        }

        auto resolved = NormalizePath(fs::path(*filename));
        bool exists = fs::exists(resolved);
        if (exists == false)
        {
            for (const auto& dir : mIncludeDirs)
            {
                auto candidate = NormalizePath(dir / fs::path(*filename));
                if (fs::exists(candidate))
                {
                    resolved = candidate;
                    exists = true;
                    break;
                }
            }

            if (exists == false)
            {
                mDiag.ReportError(pNode.mLocation, "'.INCBIN' directive: File not found.");
                return false;
            }
        }

        std::uint64_t offset = 0, size = stx::npos64;
        if (pNode.mOffset)
        {
            if (const auto i = EvaluateIntegerExpression(pNode.mOffset))
                { offset = *i; }
            else
            {
                mDiag.ReportError(pNode.mLocation, 
                    "'.INCBIN' directive: Invalid offset.");
                return false;
            }
        }

        if (pNode.mSize)
        {
            if (const auto i = EvaluateIntegerExpression(pNode.mSize))
                { size = *i; }
            else
            {
                mDiag.ReportError(pNode.mLocation, 
                    "'.INCBIN' directive: Invalid size.");
                return false;
            }
        }

        auto buffer = stx::load_binary(resolved, offset, size);
        if (buffer.has_value() == false)
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.INCBIN' directive: Failed to load binary file.");
            return false;
        }

        ctx.mData.insert(ctx.mData.end(), buffer->begin(), buffer->end());
        ctx.mHeader.mDataSize = ctx.mLocationCounter =
            static_cast<std::uint32_t>(ctx.mData.size());
        return true;
    }

    auto Codegen::DispatchExportDirective (const ExportDirectiveNode& pNode) -> bool
    {
        for (const auto& label : pNode.mLabels)
        {   
            if (label == nullptr)
            {
                mDiag.ReportError(pNode.mLocation, "'.EXPORT' directive: Null label.");
                return false;
            }

            const auto index = ResolveSymbol(label->mSymbol);
            if (index < mSymbols.size())
                { mSymbols[index].mType = ObjectSymbolType::Export; }
        }

        return true;
    }

    auto Codegen::DispatchImportDirective (const ImportDirectiveNode& pNode) -> bool
    {
        for (const auto& label : pNode.mLabels)
        {   
            if (label == nullptr)
            {
                mDiag.ReportError(pNode.mLocation, "'.IMPORT' directive: Null label.");
                return false;
            }

            const auto index = ResolveSymbol(label->mSymbol);
            if (index < mSymbols.size())
                { mSymbols[index].mType = ObjectSymbolType::Import; }
        }

        return true;
    }

    auto Codegen::DispatchSectionDirective (const SectionDirectiveNode& pNode) -> bool
    {
        std::string displayName {};
        if (const auto dn = stx::to<StringExpressionNode>(pNode.mDisplayName))
        {
            displayName = dn->mValue;
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.SECTION' directive: Invalid display name.");
            return false;
        }

        std::uint32_t targetAddress = stx::npos32;
        if (pNode.mFixedAddress)
        {
            if (const auto i = EvaluatePointerExpression(pNode.mFixedAddress);
                i && std::holds_alternative<std::uint32_t>(*i))
                { targetAddress = std::get<std::uint32_t>(*i); }
            else
            {
                mDiag.ReportError(pNode.mLocation, 
                    "'.SECTION' directive: Invalid fixed address.");
                return false;
            }
        }

        ObjectSectionType sectionType {};
        std::uint8_t interruptLine = 0;
        if (pNode.mSectionName)
        {
            switch (pNode.mSectionName->mSectionName)
            {
                case SectionName::METADATA: sectionType = ObjectSectionType::Metadata; break;
                case SectionName::CODE: sectionType = ObjectSectionType::Code; break;
                case SectionName::DATA: sectionType = ObjectSectionType::Data; break;
                case SectionName::BSS: sectionType = ObjectSectionType::BSS; break;
                default:
                    auto under = std::to_underlying(pNode.mSectionName->mSectionName);
                    if (under >= std::to_underlying(SectionName::INT0) &&
                        under <= std::to_underlying(SectionName::INT31))
                    {
                        sectionType = ObjectSectionType::Interrupt;
                        interruptLine = static_cast<std::uint8_t>(under - 
                            std::to_underlying(SectionName::INT0));
                    }
                    break;
            }
        }
        else
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.SECTION' directive: Missing section name.");
            return false;
        }

        const auto nameOffset = AddString(displayName);
        for (std::uint32_t i = 0; i < mSections.size(); ++i)
        {
            if (mSections[i].mHeader.mNameStringOffset == nameOffset)
            {
                mActiveSectionIndex = i;
                return true;
            }
        }

        SetSection(displayName, sectionType, interruptLine, targetAddress);
        return true;
    }

    auto Codegen::DispatchOrgDirective (const OrgDirectiveNode& pNode) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.ORG' directive: No active section.");
            return false;
        }

        auto& ctx = mSections[mActiveSectionIndex];
        std::uint32_t address { 0 };
        if (const auto i = EvaluateIntegerExpression(pNode.mAddress))
            { address = *i; }
        else
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.ORG' directive: Missing or invalid address.");
            return false;
        }
        
        std::uint32_t resolved { 0 };
        if (ctx.mHeader.mTargetAddress != stx::npos32)
        {
            if (address < ctx.mHeader.mTargetAddress)
            {
                mDiag.ReportError(pNode.mLocation, 
                    "'.ORG' directive: Address is before target address.");
                return false;
            }

            resolved = address - ctx.mHeader.mTargetAddress;
        }
        else
        {
            resolved = address;
        }

        if (resolved < ctx.mLocationCounter)
        {
            mDiag.ReportWarning(pNode.mLocation, 
                "'.ORG' directive: Moving address before current location counter.");
            
            ctx.mLocationCounter = resolved;
            if (ctx.mData.size() > resolved)
                { ctx.mData.resize(resolved); }
        }
        else if (resolved > ctx.mLocationCounter)
        {
            const auto padding = resolved - ctx.mLocationCounter;
            return ReserveBytes(padding);
        }

        return true;
    }

    auto Codegen::DispatchAlignDirective (const AlignDirectiveNode& pNode) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.ALIGN' directive: No active section.");
            return false;
        }

        if (!pNode.mBoundary)
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.ALIGN' directive: Missing alignment boundary.");
            return false;
        }

        auto val = EvaluateIntegerExpression(pNode.mBoundary);
        if (val.has_value() == false)
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.ALIGN' directive: Invalid alignment boundary.");
            return false;
        }
        else if (*val == 0)
        {
            mDiag.ReportError(pNode.mLocation, 
                "'.ALIGN' directive: Alignment boundary cannot be zero.");
            return false;
        }

        auto& ctx = mSections[mActiveSectionIndex];
        std::uint32_t padding = (*val - (ctx.mLocationCounter % *val)) % *val;
        if (padding > 0)
            { return ReserveBytes(padding); }

        return true;
    }

    auto Codegen::DispatchLabelStatement (const LabelStatementNode& pNode) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError(pNode.mLocation, 
                "Label definition: No active section.");
            return false;
        }

        auto& ctx = mSections[mActiveSectionIndex];
        return EmitLabel(pNode.mLabelExpr.mSymbol, ctx.mLocationCounter);
    }

    auto Codegen::DispatchInstructionStatement (const InstructionStatementNode& pNode) -> bool
    {
        if (mActiveSectionIndex == stx::npos32)
        {
            mDiag.ReportError(pNode.mLocation, 
                "Instruction statement: No active section.");
            return false;
        }
        
        auto& ctx = mSections[mActiveSectionIndex];
        if (ctx.mHeader.mType == ObjectSectionType::Metadata ||
            ctx.mHeader.mType == ObjectSectionType::Data ||
            ctx.mHeader.mType == ObjectSectionType::BSS)
        {
            mDiag.ReportError(pNode.mLocation, 
                "Instruction statement: Not allowed outside of code section.");
            return false;
        }

        switch (pNode.mType)
        {
            case CPU::InstructionType::NOP: return DispatchNOP(pNode, ctx);
            case CPU::InstructionType::STOP: return DispatchSTOP(pNode, ctx);
            case CPU::InstructionType::HALT: return DispatchHALT(pNode, ctx);
            case CPU::InstructionType::DI: return DispatchDI(pNode, ctx);
            case CPU::InstructionType::EI: return DispatchEI(pNode, ctx);
            case CPU::InstructionType::EII: return DispatchEII(pNode, ctx);
            case CPU::InstructionType::DAA: return DispatchDAA(pNode, ctx);
            case CPU::InstructionType::SCF: return DispatchSCF(pNode, ctx);
            case CPU::InstructionType::CCF: return DispatchCCF(pNode, ctx);
            case CPU::InstructionType::CLV: return DispatchCLV(pNode, ctx);
            case CPU::InstructionType::SEV: return DispatchSEV(pNode, ctx);
            case CPU::InstructionType::REX: return DispatchREX(pNode, ctx);
            case CPU::InstructionType::LEC: return DispatchLEC(pNode, ctx);
            case CPU::InstructionType::LD: return DispatchLD(pNode, ctx);
            case CPU::InstructionType::LDQ: return DispatchLDQ(pNode, ctx);
            case CPU::InstructionType::LDP: return DispatchLDP(pNode, ctx);
            case CPU::InstructionType::ST: return DispatchST(pNode, ctx);
            case CPU::InstructionType::STQ: return DispatchSTQ(pNode, ctx);
            case CPU::InstructionType::STP: return DispatchSTP(pNode, ctx);
            case CPU::InstructionType::MV: return DispatchMV(pNode, ctx);
            case CPU::InstructionType::MWH: return DispatchMWH(pNode, ctx);
            case CPU::InstructionType::MWL: return DispatchMWL(pNode, ctx);
            case CPU::InstructionType::LSP: return DispatchLSP(pNode, ctx);
            case CPU::InstructionType::POP: return DispatchPOP(pNode, ctx);
            case CPU::InstructionType::SSP: return DispatchSSP(pNode, ctx);
            case CPU::InstructionType::PUSH: return DispatchPUSH(pNode, ctx);
            case CPU::InstructionType::SPO: return DispatchSPO(pNode, ctx);
            case CPU::InstructionType::SPI: return DispatchSPI(pNode, ctx);
            case CPU::InstructionType::JMP: return DispatchJMP(pNode, ctx);
            case CPU::InstructionType::JPB: return DispatchJPB(pNode, ctx);
            case CPU::InstructionType::CALL: return DispatchCALL(pNode, ctx);
            case CPU::InstructionType::INT: return DispatchINT(pNode, ctx);
            case CPU::InstructionType::RET: return DispatchRET(pNode, ctx);
            case CPU::InstructionType::RETI: return DispatchRETI(pNode, ctx);
            case CPU::InstructionType::MFI: return DispatchMFI(pNode, ctx);
            case CPU::InstructionType::MFO: return DispatchMFO(pNode, ctx);
            case CPU::InstructionType::ADD: return DispatchADD(pNode, ctx);
            case CPU::InstructionType::ADC: return DispatchADC(pNode, ctx);
            case CPU::InstructionType::SUB: return DispatchSUB(pNode, ctx);
            case CPU::InstructionType::SBC: return DispatchSBC(pNode, ctx);
            case CPU::InstructionType::INC: return DispatchINC(pNode, ctx);
            case CPU::InstructionType::DEC: return DispatchDEC(pNode, ctx);
            case CPU::InstructionType::AND: return DispatchAND(pNode, ctx);
            case CPU::InstructionType::OR: return DispatchOR(pNode, ctx);
            case CPU::InstructionType::XOR: return DispatchXOR(pNode, ctx);
            case CPU::InstructionType::NOT: return DispatchNOT(pNode, ctx);
            case CPU::InstructionType::CMP: return DispatchCMP(pNode, ctx);
            case CPU::InstructionType::SLA: return DispatchSLA(pNode, ctx);
            case CPU::InstructionType::SRA: return DispatchSRA(pNode, ctx);
            case CPU::InstructionType::SRL: return DispatchSRL(pNode, ctx);
            case CPU::InstructionType::SWAP: return DispatchSWAP(pNode, ctx);
            case CPU::InstructionType::RLA: return DispatchRLA(pNode, ctx);
            case CPU::InstructionType::RL: return DispatchRL(pNode, ctx);
            case CPU::InstructionType::RLCA: return DispatchRLCA(pNode, ctx);
            case CPU::InstructionType::RLC: return DispatchRLC(pNode, ctx);
            case CPU::InstructionType::RRA: return DispatchRRA(pNode, ctx);
            case CPU::InstructionType::RR: return DispatchRR(pNode, ctx);
            case CPU::InstructionType::RRCA: return DispatchRRCA(pNode, ctx);
            case CPU::InstructionType::RRC: return DispatchRRC(pNode, ctx);
            case CPU::InstructionType::BIT: return DispatchBIT(pNode, ctx);
            case CPU::InstructionType::SET: return DispatchSET(pNode, ctx);
            case CPU::InstructionType::RES: return DispatchRES(pNode, ctx);
            case CPU::InstructionType::TOG: return DispatchTOG(pNode, ctx);
            case CPU::InstructionType::LDI: return DispatchLDI(pNode, ctx);
            case CPU::InstructionType::LDD: return DispatchLDD(pNode, ctx);
            case CPU::InstructionType::STI: return DispatchSTI(pNode, ctx);
            case CPU::InstructionType::STD: return DispatchSTD(pNode, ctx);
            case CPU::InstructionType::ASP: return DispatchASP(pNode, ctx);
            case CPU::InstructionType::LASP: return DispatchLASP(pNode, ctx);
            case CPU::InstructionType::ISP: return DispatchISP(pNode, ctx);
            case CPU::InstructionType::DSP: return DispatchDSP(pNode, ctx);
            case CPU::InstructionType::ASR: return DispatchASR(pNode, ctx);
            default:
                mDiag.ReportError(pNode.mLocation, 
                    "Invalid instruction type '{}'", pNode.mName);
                return false;
        }
    }
}
