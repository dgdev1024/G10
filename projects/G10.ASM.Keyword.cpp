/**
 * @file    G10.ASM.Keyword.cpp
 * @brief   Contains implementations for the G10 Assembler's keyword class, 
 *          and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Keyword.hpp>

// Private - Static Members ****************************************************

namespace G10::ASM
{
    std::unordered_map<std::string_view, Keyword> Keyword::mLookup =
    {
        { "D0", Keyword { CPU::Register::D0 } },
        { "D1", Keyword { CPU::Register::D1 } },
        { "D2", Keyword { CPU::Register::D2 } },
        { "D3", Keyword { CPU::Register::D3 } },
        { "D4", Keyword { CPU::Register::D4 } },
        { "D5", Keyword { CPU::Register::D5 } },
        { "D6", Keyword { CPU::Register::D6 } },
        { "D7", Keyword { CPU::Register::D7 } },
        { "D8", Keyword { CPU::Register::D8 } },
        { "D9", Keyword { CPU::Register::D9 } },
        { "D10", Keyword { CPU::Register::D10 } },
        { "D11", Keyword { CPU::Register::D11 } },
        { "D12", Keyword { CPU::Register::D12 } },
        { "D13", Keyword { CPU::Register::D13 } },
        { "D14", Keyword { CPU::Register::D14 } },
        { "D15", Keyword { CPU::Register::D15 } },
        { "W0", Keyword { CPU::Register::W0 } },
        { "W1", Keyword { CPU::Register::W1 } },
        { "W2", Keyword { CPU::Register::W2 } },
        { "W3", Keyword { CPU::Register::W3 } },
        { "W4", Keyword { CPU::Register::W4 } },
        { "W5", Keyword { CPU::Register::W5 } },
        { "W6", Keyword { CPU::Register::W6 } },
        { "W7", Keyword { CPU::Register::W7 } },
        { "W8", Keyword { CPU::Register::W8 } },
        { "W9", Keyword { CPU::Register::W9 } },
        { "W10", Keyword { CPU::Register::W10 } },
        { "W11", Keyword { CPU::Register::W11 } },
        { "W12", Keyword { CPU::Register::W12 } },
        { "W13", Keyword { CPU::Register::W13 } },
        { "W14", Keyword { CPU::Register::W14 } },
        { "W15", Keyword { CPU::Register::W15 } },
        { "H0", Keyword { CPU::Register::H0 } },
        { "H1", Keyword { CPU::Register::H1 } },
        { "H2", Keyword { CPU::Register::H2 } },
        { "H3", Keyword { CPU::Register::H3 } },
        { "H4", Keyword { CPU::Register::H4 } },
        { "H5", Keyword { CPU::Register::H5 } },
        { "H6", Keyword { CPU::Register::H6 } },
        { "H7", Keyword { CPU::Register::H7 } },
        { "H8", Keyword { CPU::Register::H8 } },
        { "H9", Keyword { CPU::Register::H9 } },
        { "H10", Keyword { CPU::Register::H10 } },
        { "H11", Keyword { CPU::Register::H11 } },
        { "H12", Keyword { CPU::Register::H12 } },
        { "H13", Keyword { CPU::Register::H13 } },
        { "H14", Keyword { CPU::Register::H14 } },
        { "H15", Keyword { CPU::Register::H15 } },
        { "L0", Keyword { CPU::Register::L0 } },
        { "L1", Keyword { CPU::Register::L1 } },
        { "L2", Keyword { CPU::Register::L2 } },
        { "L3", Keyword { CPU::Register::L3 } },
        { "L4", Keyword { CPU::Register::L4 } },
        { "L5", Keyword { CPU::Register::L5 } },
        { "L6", Keyword { CPU::Register::L6 } },
        { "L7", Keyword { CPU::Register::L7 } },
        { "L8", Keyword { CPU::Register::L8 } },
        { "L9", Keyword { CPU::Register::L9 } },
        { "L10", Keyword { CPU::Register::L10 } },
        { "L11", Keyword { CPU::Register::L11 } },
        { "L12", Keyword { CPU::Register::L12 } },
        { "L13", Keyword { CPU::Register::L13 } },
        { "L14", Keyword { CPU::Register::L14 } },
        { "L15", Keyword { CPU::Register::L15 } },

        { "NC", Keyword { CPU::Condition::NC } },
        { "ZS", Keyword { CPU::Condition::ZS } },
        { "ZC", Keyword { CPU::Condition::ZC } },
        { "CS", Keyword { CPU::Condition::CS } },
        { "CC", Keyword { CPU::Condition::CC } },
        { "VS", Keyword { CPU::Condition::VS } },
        { "VC", Keyword { CPU::Condition::VC } },

        { "NOP", Keyword { CPU::InstructionType::NOP } },
        { "STOP", Keyword { CPU::InstructionType::STOP } },
        { "HALT", Keyword { CPU::InstructionType::HALT } },
        { "DI", Keyword { CPU::InstructionType::DI } },
        { "EI", Keyword { CPU::InstructionType::EI } },
        { "EII", Keyword { CPU::InstructionType::EII } },
        { "DAA", Keyword { CPU::InstructionType::DAA } },
        { "SCF", Keyword { CPU::InstructionType::SCF } },
        { "CCF", Keyword { CPU::InstructionType::CCF } },
        { "CLV", Keyword { CPU::InstructionType::CLV } },
        { "SEV", Keyword { CPU::InstructionType::SEV } },
        { "REX", Keyword { CPU::InstructionType::REX } },
        { "LEC", Keyword { CPU::InstructionType::LEC } },
        { "LD", Keyword { CPU::InstructionType::LD } },
        { "LDQ", Keyword { CPU::InstructionType::LDQ } },
        { "LDP", Keyword { CPU::InstructionType::LDP } },
        { "ST", Keyword { CPU::InstructionType::ST } },
        { "STQ", Keyword { CPU::InstructionType::STQ } },
        { "STP", Keyword { CPU::InstructionType::STP } },
        { "MV", Keyword { CPU::InstructionType::MV } },
        { "MWH", Keyword { CPU::InstructionType::MWH } },
        { "MWL", Keyword { CPU::InstructionType::MWL } },
        { "LSP", Keyword { CPU::InstructionType::LSP } },
        { "POP", Keyword { CPU::InstructionType::POP } },
        { "SSP", Keyword { CPU::InstructionType::SSP } },
        { "PUSH", Keyword { CPU::InstructionType::PUSH } },
        { "SPO", Keyword { CPU::InstructionType::SPO } },
        { "SPI", Keyword { CPU::InstructionType::SPI } },
        { "JMP", Keyword { CPU::InstructionType::JMP } },
        { "JPB", Keyword { CPU::InstructionType::JPB } },
        { "CALL", Keyword { CPU::InstructionType::CALL } },
        { "INT", Keyword { CPU::InstructionType::INT } },
        { "RET", Keyword { CPU::InstructionType::RET } },
        { "RETI", Keyword { CPU::InstructionType::RETI } },
        { "MFI", Keyword { CPU::InstructionType::MFI } },
        { "MFO", Keyword { CPU::InstructionType::MFO } },
        { "ADD", Keyword { CPU::InstructionType::ADD } },
        { "ADC", Keyword { CPU::InstructionType::ADC } },
        { "SUB", Keyword { CPU::InstructionType::SUB } },
        { "SBC", Keyword { CPU::InstructionType::SBC } },
        { "INC", Keyword { CPU::InstructionType::INC } },
        { "DEC", Keyword { CPU::InstructionType::DEC } },
        { "AND", Keyword { CPU::InstructionType::AND } },
        { "OR", Keyword { CPU::InstructionType::OR } },
        { "XOR", Keyword { CPU::InstructionType::XOR } },
        { "NOT", Keyword { CPU::InstructionType::NOT } },
        { "CMP", Keyword { CPU::InstructionType::CMP } },
        { "SLA", Keyword { CPU::InstructionType::SLA } },
        { "SRA", Keyword { CPU::InstructionType::SRA } },
        { "SRL", Keyword { CPU::InstructionType::SRL } },
        { "SWAP", Keyword { CPU::InstructionType::SWAP } },
        { "RLA", Keyword { CPU::InstructionType::RLA } },
        { "RL", Keyword { CPU::InstructionType::RL } },
        { "RLCA", Keyword { CPU::InstructionType::RLCA } },
        { "RLC", Keyword { CPU::InstructionType::RLC } },
        { "RRA", Keyword { CPU::InstructionType::RRA } },
        { "RR", Keyword { CPU::InstructionType::RR } },
        { "RRCA", Keyword { CPU::InstructionType::RRCA } },
        { "RRC", Keyword { CPU::InstructionType::RRC } },
        { "BIT", Keyword { CPU::InstructionType::BIT } },
        { "SET", Keyword { CPU::InstructionType::SET } },
        { "RES", Keyword { CPU::InstructionType::RES } },
        { "TOG", Keyword { CPU::InstructionType::TOG } },
        { "LDI", Keyword { CPU::InstructionType::LDI } },
        { "LDD", Keyword { CPU::InstructionType::LDD } },
        { "STI", Keyword { CPU::InstructionType::STI } },
        { "STD", Keyword { CPU::InstructionType::STD } },
        { "ASP", Keyword { CPU::InstructionType::ASP } },
        { "LASP", Keyword { CPU::InstructionType::LASP } },
        { "ISP", Keyword { CPU::InstructionType::ISP } },
        { "DSP", Keyword { CPU::InstructionType::DSP } },
        { "ASR", Keyword { CPU::InstructionType::ASR } },

        { "MOV", Keyword { CPU::InstructionType::LD } },
        { "IN", Keyword { CPU::InstructionType::LDP } },
        { "OUT", Keyword { CPU::InstructionType::STP } },
        { "JP", Keyword { CPU::InstructionType::JMP } },
        { "JR", Keyword { CPU::InstructionType::JPB } },
        { "RST", Keyword { CPU::InstructionType::INT } },
        { "CPL", Keyword { CPU::InstructionType::NOT } },
        { "SAL", Keyword { CPU::InstructionType::SLA } },
        { "SAR", Keyword { CPU::InstructionType::SRA } },
        { "SHR", Keyword { CPU::InstructionType::SRL } },
        { "ROL", Keyword { CPU::InstructionType::RLC } },
        { "ROR", Keyword { CPU::InstructionType::RRC } },
        { "RCL", Keyword { CPU::InstructionType::RL } },
        { "RCR", Keyword { CPU::InstructionType::RR } },

        { ".BYTE", Keyword { AssemblerDirective::BYTE } },
        { ".WORD", Keyword { AssemblerDirective::WORD } },
        { ".DWORD", Keyword { AssemblerDirective::DWORD } },
        { ".ASCIZ", Keyword { AssemblerDirective::ASCIZ } },
        { ".SPACE", Keyword { AssemblerDirective::SPACE } },
        { ".INCBIN", Keyword { AssemblerDirective::INCBIN } },
        { ".EXPORT", Keyword { AssemblerDirective::EXPORT } },
        { ".IMPORT", Keyword { AssemblerDirective::IMPORT } },
        { ".SECTION", Keyword { AssemblerDirective::SECTION } },
        { ".ORG", Keyword { AssemblerDirective::ORG } },
        { ".ALIGN", Keyword { AssemblerDirective::ALIGN } },

        { ".DB", Keyword { AssemblerDirective::BYTE } },
        { ".DW", Keyword { AssemblerDirective::WORD } },
        { ".DD", Keyword { AssemblerDirective::DWORD } },
        { ".DT", Keyword { AssemblerDirective::ASCIZ } },
        { ".STRING", Keyword { AssemblerDirective::ASCIZ } },
        { ".DS", Keyword { AssemblerDirective::SPACE } },
        { ".RESERVE", Keyword { AssemblerDirective::SPACE } },
        { ".GLOBAL", Keyword { AssemblerDirective::EXPORT } },
        { ".EXTERN", Keyword { AssemblerDirective::IMPORT } },

        { "_NARG", Keyword { PreprocessorBuiltinSymbol::_NARG } },
        { "_TARG", Keyword { PreprocessorBuiltinSymbol::_TARG } },
        { "_LINE", Keyword { PreprocessorBuiltinSymbol::_LINE } },
        { "_FILE", Keyword { PreprocessorBuiltinSymbol::_FILE } },
        { "_DATE", Keyword { PreprocessorBuiltinSymbol::_DATE } },
        { "_TIME", Keyword { PreprocessorBuiltinSymbol::_TIME } },
        { "_VERSION", Keyword { PreprocessorBuiltinSymbol::_VERSION } },
        { "_UNIQUE", Keyword { PreprocessorBuiltinSymbol::_UNIQUE } },
        { "_RANDOM32", Keyword { PreprocessorBuiltinSymbol::_RANDOM32 } },
        { "_RANDOM64", Keyword { PreprocessorBuiltinSymbol::_RANDOM64 } },

        { ".CONST", Keyword { PreprocessorDirective::CONST } },
        { ".LET", Keyword { PreprocessorDirective::LET } },
        { ".IF", Keyword { PreprocessorDirective::IF } },
        { ".IFDEF", Keyword { PreprocessorDirective::IFDEF } },
        { ".IFNDEF", Keyword { PreprocessorDirective::IFNDEF } },
        { ".ELSEIF", Keyword { PreprocessorDirective::ELSEIF } },
        { ".ELSE", Keyword { PreprocessorDirective::ELSE } },
        { ".ENDIF", Keyword { PreprocessorDirective::ENDIF } },
        { ".REPEAT", Keyword { PreprocessorDirective::REPEAT } },
        { ".ENDREPEAT", Keyword { PreprocessorDirective::ENDREPEAT } },
        { ".FOR", Keyword { PreprocessorDirective::FOR } },
        { ".ENDFOR", Keyword { PreprocessorDirective::ENDFOR } },
        { ".WHILE", Keyword { PreprocessorDirective::WHILE } },
        { ".ENDWHILE", Keyword { PreprocessorDirective::ENDWHILE } },
        { ".CONTINUE", Keyword { PreprocessorDirective::CONTINUE } },
        { ".BREAK", Keyword { PreprocessorDirective::BREAK } },
        { ".INCLUDE", Keyword { PreprocessorDirective::INCLUDE } },
        { ".ONCE", Keyword { PreprocessorDirective::ONCE } },
        { ".MACRO", Keyword { PreprocessorDirective::MACRO } },
        { ".ENDMACRO", Keyword { PreprocessorDirective::ENDMACRO } },
        { ".SHIFT", Keyword { PreprocessorDirective::SHIFT } },
        { ".RETURN", Keyword { PreprocessorDirective::RETURN } },
        { ".OPTION", Keyword { PreprocessorDirective::OPTION } },
        { ".PRINT", Keyword { PreprocessorDirective::PRINT } },
        { ".PRINTLN", Keyword { PreprocessorDirective::PRINTLN } },
        { ".INFO", Keyword { PreprocessorDirective::INFO } },
        { ".WARNING", Keyword { PreprocessorDirective::WARNING } },
        { ".ERROR", Keyword { PreprocessorDirective::ERROR } },
        { ".ASSERT", Keyword { PreprocessorDirective::ASSERT } },

        { ".DEF", Keyword { PreprocessorDirective::CONST } },
        { ".DEFINE", Keyword { PreprocessorDirective::CONST } },
        { ".VAR", Keyword { PreprocessorDirective::LET } },
        { ".SET", Keyword { PreprocessorDirective::LET } },
        { ".MUT", Keyword { PreprocessorDirective::LET } },
        { ".ELIF", Keyword { PreprocessorDirective::ELSEIF } },
        { ".ENDC", Keyword { PreprocessorDirective::ENDIF } },
        { ".REPT", Keyword { PreprocessorDirective::REPEAT } },
        { ".ENDR", Keyword { PreprocessorDirective::ENDREPEAT } },
        { ".ENDF", Keyword { PreprocessorDirective::ENDFOR } },
        { ".ENDW", Keyword { PreprocessorDirective::ENDWHILE } },
        { ".ENDM", Keyword { PreprocessorDirective::ENDMACRO } },
        { ".FAIL", Keyword { PreprocessorDirective::ERROR } },

        { "HIGH", Keyword { PreprocessorFunction::HIGH } },
        { "LOW", Keyword { PreprocessorFunction::LOW } },
        { "HIDWORD", Keyword { PreprocessorFunction::HIDWORD } },
        { "LODWORD", Keyword { PreprocessorFunction::LODWORD } },
        { "HIWORD", Keyword { PreprocessorFunction::HIWORD } },
        { "LOWORD", Keyword { PreprocessorFunction::LOWORD } },
        { "HIBYTE", Keyword { PreprocessorFunction::HIBYTE } },
        { "LOBYTE", Keyword { PreprocessorFunction::LOBYTE } },
        { "HINIBBLE", Keyword { PreprocessorFunction::HINIBBLE } },
        { "LONIBBLE", Keyword { PreprocessorFunction::LONIBBLE } },
        { "BITWIDTH", Keyword { PreprocessorFunction::BITWIDTH } },
        { "TZCOUNT", Keyword { PreprocessorFunction::TZCOUNT } },
        { "FINT", Keyword { PreprocessorFunction::FINT } },
        { "FFRAC", Keyword { PreprocessorFunction::FFRAC } },
        { "FADD", Keyword { PreprocessorFunction::FADD } },
        { "FSUB", Keyword { PreprocessorFunction::FSUB } },
        { "FDIV", Keyword { PreprocessorFunction::FDIV } },
        { "FMUL", Keyword { PreprocessorFunction::FMUL } },
        { "FMOD", Keyword { PreprocessorFunction::FMOD } },
        { "FPOW", Keyword { PreprocessorFunction::FPOW } },
        { "FSQRT", Keyword { PreprocessorFunction::FSQRT } },
        { "FROOT", Keyword { PreprocessorFunction::FROOT } },
        { "FLOG", Keyword { PreprocessorFunction::FLOG } },
        { "FLN", Keyword { PreprocessorFunction::FLN } },
        { "FROUND", Keyword { PreprocessorFunction::FROUND } },
        { "FCEIL", Keyword { PreprocessorFunction::FCEIL } },
        { "FFLOOR", Keyword { PreprocessorFunction::FFLOOR } },
        { "FRADT", Keyword { PreprocessorFunction::FRADT } },
        { "FDEGT", Keyword { PreprocessorFunction::FDEGT } },
        { "FSIN", Keyword { PreprocessorFunction::FSIN } },
        { "FCOS", Keyword { PreprocessorFunction::FCOS } },
        { "FTAN", Keyword { PreprocessorFunction::FTAN } },
        { "FASIN", Keyword { PreprocessorFunction::FASIN } },
        { "FACOS", Keyword { PreprocessorFunction::FACOS } },
        { "FATAN", Keyword { PreprocessorFunction::FATAN } },
        { "FATAN2", Keyword { PreprocessorFunction::FATAN2 } },
        { "STRCAT", Keyword { PreprocessorFunction::STRCAT } },
        { "STRNCAT", Keyword { PreprocessorFunction::STRNCAT } },
        { "STRUPR", Keyword { PreprocessorFunction::STRUPR } },
        { "STRLWR", Keyword { PreprocessorFunction::STRLWR } },
        { "STRSLICE", Keyword { PreprocessorFunction::STRSLICE } },
        { "STRREPLACE", Keyword { PreprocessorFunction::STRREPLACE } },
        { "STRFMT", Keyword { PreprocessorFunction::STRFMT } },
        { "STRLEN", Keyword { PreprocessorFunction::STRLEN } },
        { "STRCMP", Keyword { PreprocessorFunction::STRCMP } },
        { "STRNCMP", Keyword { PreprocessorFunction::STRNCMP } },
        { "STRFIND", Keyword { PreprocessorFunction::STRFIND } },
        { "STRRFIND", Keyword { PreprocessorFunction::STRRFIND } },
        { "STRNFIND", Keyword { PreprocessorFunction::STRNFIND } },
        { "STRNRFIND", Keyword { PreprocessorFunction::STRNRFIND } },
        { "BYTELEN", Keyword { PreprocessorFunction::BYTELEN } },
        { "STRBYTE", Keyword { PreprocessorFunction::STRBYTE } },
        { "DEFINED", Keyword { PreprocessorFunction::DEFINED } },
        { "ISCONST", Keyword { PreprocessorFunction::ISCONST } },
        { "ISMACRO", Keyword { PreprocessorFunction::ISMACRO } },
        { "ISSYMBOL", Keyword { PreprocessorFunction::ISSYMBOL } },

        { "METADATA", Keyword { SectionName::METADATA } },
        { "INT0", Keyword { SectionName::INT0 } },
        { "INT1", Keyword { SectionName::INT1 } },
        { "INT2", Keyword { SectionName::INT2 } },
        { "INT3", Keyword { SectionName::INT3 } },
        { "INT4", Keyword { SectionName::INT4 } },
        { "INT5", Keyword { SectionName::INT5 } },
        { "INT6", Keyword { SectionName::INT6 } },
        { "INT7", Keyword { SectionName::INT7 } },
        { "INT8", Keyword { SectionName::INT8 } },
        { "INT9", Keyword { SectionName::INT9 } },
        { "INT10", Keyword { SectionName::INT10 } },
        { "INT11", Keyword { SectionName::INT11 } },
        { "INT12", Keyword { SectionName::INT12 } },
        { "INT13", Keyword { SectionName::INT13 } },
        { "INT14", Keyword { SectionName::INT14 } },
        { "INT15", Keyword { SectionName::INT15 } },
        { "INT16", Keyword { SectionName::INT16 } },
        { "INT17", Keyword { SectionName::INT17 } },
        { "INT18", Keyword { SectionName::INT18 } },
        { "INT19", Keyword { SectionName::INT19 } },
        { "INT20", Keyword { SectionName::INT20 } },
        { "INT21", Keyword { SectionName::INT21 } },
        { "INT22", Keyword { SectionName::INT22 } },
        { "INT23", Keyword { SectionName::INT23 } },
        { "INT24", Keyword { SectionName::INT24 } },
        { "INT25", Keyword { SectionName::INT25 } },
        { "INT26", Keyword { SectionName::INT26 } },
        { "INT27", Keyword { SectionName::INT27 } },
        { "INT28", Keyword { SectionName::INT28 } },
        { "INT29", Keyword { SectionName::INT29 } },
        { "INT30", Keyword { SectionName::INT30 } },
        { "INT31", Keyword { SectionName::INT31 } },
        { "CODE", Keyword { SectionName::CODE } },
        { "DATA", Keyword { SectionName::DATA } },
        { "BSS", Keyword { SectionName::BSS } },

        { "TEXT", Keyword { SectionName::CODE } },
        { "RODATA", Keyword { SectionName::DATA } },
        { "RAM", Keyword { SectionName::BSS } },

        { ".FILE", Keyword { Hint::FILE } },
        { ".LINE", Keyword { Hint::LINE } }
    };
}

// Public - Constructors & Destructor ******************************************

namespace G10::ASM
{
    Keyword::Keyword (const CPU::Register pType) :
        mGroup  { KeywordGroup::Register },
        mType   { pType }
    {}

    Keyword::Keyword (const CPU::Condition pType) :
        mGroup  { KeywordGroup::Condition },
        mType   { pType }
    {}

    Keyword::Keyword (const CPU::InstructionType pType) :
        mGroup  { KeywordGroup::InstructionType },
        mType   { pType }
    {}

    Keyword::Keyword (const PreprocessorDirective pType) :
        mGroup  { KeywordGroup::PreprocessorDirective },
        mType   { pType }
    {}

    Keyword::Keyword (const SectionName pType) :
        mGroup  { KeywordGroup::SectionName },
        mType   { pType }
    {}

    Keyword::Keyword (const PreprocessorFunction pType) :
        mGroup  { KeywordGroup::PreprocessorFunction },
        mType   { pType }
    {}

    Keyword::Keyword (const AssemblerDirective pType) :
        mGroup  { KeywordGroup::AssemblerDirective },
        mType   { pType }
    {}

    Keyword::Keyword (const PreprocessorBuiltinSymbol pType) :
        mGroup  { KeywordGroup::PreprocessorBuiltinSymbol },
        mType   { pType }
    {}

    Keyword::Keyword (const Hint pType) :
        mGroup  { KeywordGroup::Hint },
        mType   { pType }
    {}
}

// Public Static Methods *******************************************************

namespace G10::ASM
{
    auto Keyword::Lookup (std::string str) -> stx::optional_ref<const Keyword>
    {
        std::transform(str.begin(), str.end(), str.begin(),
            [] (char c) { return std::toupper(c); });
        auto findIt = mLookup.find(str);
        if (findIt != mLookup.end())
            { return findIt->second; }

        return std::nullopt;
    }
}

// Public Methods **************************************************************

namespace G10::ASM
{
    auto Keyword::GetGroup () const -> KeywordGroup
    {
        return mGroup;
    }

    auto Keyword::StringifyType () const -> std::string_view
    {
        return std::visit(stx::overload {
            [] (CPU::Register pType) 
            { 
                switch (pType)
                {
                    case CPU::Register::D0: return "D0";
                    case CPU::Register::D1: return "D1";
                    case CPU::Register::D2: return "D2";
                    case CPU::Register::D3: return "D3";
                    case CPU::Register::D4: return "D4";
                    case CPU::Register::D5: return "D5";
                    case CPU::Register::D6: return "D6";
                    case CPU::Register::D7: return "D7";
                    case CPU::Register::D8: return "D8";
                    case CPU::Register::D9: return "D9";
                    case CPU::Register::D10: return "D10";
                    case CPU::Register::D11: return "D11";
                    case CPU::Register::D12: return "D12";
                    case CPU::Register::D13: return "D13";
                    case CPU::Register::D14: return "D14";
                    case CPU::Register::D15: return "D15";
                    case CPU::Register::W0: return "W0";
                    case CPU::Register::W1: return "W1";
                    case CPU::Register::W2: return "W2";
                    case CPU::Register::W3: return "W3";
                    case CPU::Register::W4: return "W4";
                    case CPU::Register::W5: return "W5";
                    case CPU::Register::W6: return "W6";
                    case CPU::Register::W7: return "W7";
                    case CPU::Register::W8: return "W8";
                    case CPU::Register::W9: return "W9";
                    case CPU::Register::W10: return "W10";
                    case CPU::Register::W11: return "W11";
                    case CPU::Register::W12: return "W12";
                    case CPU::Register::W13: return "W13";
                    case CPU::Register::W14: return "W14";
                    case CPU::Register::W15: return "W15";
                    case CPU::Register::H0: return "H0";
                    case CPU::Register::H1: return "H1";
                    case CPU::Register::H2: return "H2";
                    case CPU::Register::H3: return "H3";
                    case CPU::Register::H4: return "H4";
                    case CPU::Register::H5: return "H5";
                    case CPU::Register::H6: return "H6";
                    case CPU::Register::H7: return "H7";
                    case CPU::Register::H8: return "H8";
                    case CPU::Register::H9: return "H9";
                    case CPU::Register::H10: return "H10";
                    case CPU::Register::H11: return "H11";
                    case CPU::Register::H12: return "H12";
                    case CPU::Register::H13: return "H13";
                    case CPU::Register::H14: return "H14";
                    case CPU::Register::H15: return "H15";
                    case CPU::Register::L0: return "L0";
                    case CPU::Register::L1: return "L1";
                    case CPU::Register::L2: return "L2";
                    case CPU::Register::L3: return "L3";
                    case CPU::Register::L4: return "L4";
                    case CPU::Register::L5: return "L5";
                    case CPU::Register::L6: return "L6";
                    case CPU::Register::L7: return "L7";
                    case CPU::Register::L8: return "L8";
                    case CPU::Register::L9: return "L9";
                    case CPU::Register::L10: return "L10";
                    case CPU::Register::L11: return "L11";
                    case CPU::Register::L12: return "L12";
                    case CPU::Register::L13: return "L13";
                    case CPU::Register::L14: return "L14";
                    case CPU::Register::L15: return "L15"; 
                    default: return "";
                } 
            },
            [] (CPU::Condition pType) 
            { 
                switch (pType)
                {
                    case CPU::Condition::NC: return "NC";
                    case CPU::Condition::ZS: return "ZS";
                    case CPU::Condition::ZC: return "ZC";
                    case CPU::Condition::CS: return "CS";
                    case CPU::Condition::CC: return "CC";
                    case CPU::Condition::VS: return "VS";
                    case CPU::Condition::VC: return "VC";
                    default: return "";
                }
            },
            [] (CPU::InstructionType pType) 
            { 
                switch (pType)
                {
                    case CPU::InstructionType::NOP: return "NOP";
                    case CPU::InstructionType::STOP: return "STOP";
                    case CPU::InstructionType::HALT: return "HALT";
                    case CPU::InstructionType::DI: return "DI";
                    case CPU::InstructionType::EI: return "EI";
                    case CPU::InstructionType::EII: return "EII";
                    case CPU::InstructionType::DAA: return "DAA";
                    case CPU::InstructionType::SCF: return "SCF";
                    case CPU::InstructionType::CCF: return "CCF";
                    case CPU::InstructionType::CLV: return "CLV";
                    case CPU::InstructionType::SEV: return "SEV";
                    case CPU::InstructionType::REX: return "REX";
                    case CPU::InstructionType::LEC: return "LEC";
                    case CPU::InstructionType::LD: return "LD";
                    case CPU::InstructionType::LDQ: return "LDQ";
                    case CPU::InstructionType::LDP: return "LDP";
                    case CPU::InstructionType::ST: return "ST";
                    case CPU::InstructionType::STQ: return "STQ";
                    case CPU::InstructionType::STP: return "STP";
                    case CPU::InstructionType::MV: return "MV";
                    case CPU::InstructionType::MWH: return "MWH";
                    case CPU::InstructionType::MWL: return "MWL";
                    case CPU::InstructionType::LSP: return "LSP";
                    case CPU::InstructionType::POP: return "POP";
                    case CPU::InstructionType::SSP: return "SSP";
                    case CPU::InstructionType::PUSH: return "PUSH";
                    case CPU::InstructionType::SPO: return "SPO";
                    case CPU::InstructionType::SPI: return "SPI";
                    case CPU::InstructionType::JMP: return "JMP";
                    case CPU::InstructionType::JPB: return "JPB";
                    case CPU::InstructionType::CALL: return "CALL";
                    case CPU::InstructionType::INT: return "INT";
                    case CPU::InstructionType::RET: return "RET";
                    case CPU::InstructionType::RETI: return "RETI";
                    case CPU::InstructionType::MFI: return "MFI";
                    case CPU::InstructionType::MFO: return "MFO";
                    case CPU::InstructionType::ADD: return "ADD";
                    case CPU::InstructionType::ADC: return "ADC";
                    case CPU::InstructionType::SUB: return "SUB";
                    case CPU::InstructionType::SBC: return "SBC";
                    case CPU::InstructionType::INC: return "INC";
                    case CPU::InstructionType::DEC: return "DEC";
                    case CPU::InstructionType::AND: return "AND";
                    case CPU::InstructionType::OR: return "OR";
                    case CPU::InstructionType::XOR: return "XOR";
                    case CPU::InstructionType::NOT: return "NOT";
                    case CPU::InstructionType::CMP: return "CMP";
                    case CPU::InstructionType::SLA: return "SLA";
                    case CPU::InstructionType::SRA: return "SRA";
                    case CPU::InstructionType::SRL: return "SRL";
                    case CPU::InstructionType::SWAP: return "SWAP";
                    case CPU::InstructionType::RLA: return "RLA";
                    case CPU::InstructionType::RL: return "RL";
                    case CPU::InstructionType::RLCA: return "RLCA";
                    case CPU::InstructionType::RLC: return "RLC";
                    case CPU::InstructionType::RRA: return "RRA";
                    case CPU::InstructionType::RR: return "RR";
                    case CPU::InstructionType::RRCA: return "RRCA";
                    case CPU::InstructionType::RRC: return "RRC";
                    case CPU::InstructionType::BIT: return "BIT";
                    case CPU::InstructionType::SET: return "SET";
                    case CPU::InstructionType::RES: return "RES";
                    case CPU::InstructionType::TOG: return "TOG";
                    case CPU::InstructionType::LDI: return "LDI";
                    case CPU::InstructionType::LDD: return "LDD";
                    case CPU::InstructionType::STI: return "STI";
                    case CPU::InstructionType::STD: return "STD";
                    case CPU::InstructionType::ASP: return "ASP";
                    case CPU::InstructionType::LASP: return "LASP";
                    case CPU::InstructionType::ISP: return "ISP";
                    case CPU::InstructionType::DSP: return "DSP";
                    case CPU::InstructionType::ASR: return "ASR";
                    default: return "";
                }
            },
            [] (PreprocessorDirective pType) 
            { 
                switch (pType)
                {
                    case PreprocessorDirective::CONST: return ".CONST";
                    case PreprocessorDirective::LET: return ".LET";
                    case PreprocessorDirective::IF: return ".IF";
                    case PreprocessorDirective::IFDEF: return ".IFDEF";
                    case PreprocessorDirective::IFNDEF: return ".IFNDEF";
                    case PreprocessorDirective::ELSEIF: return ".ELSEIF";
                    case PreprocessorDirective::ELSE: return ".ELSE";
                    case PreprocessorDirective::ENDIF: return ".ENDIF";
                    case PreprocessorDirective::REPEAT: return ".REPEAT";
                    case PreprocessorDirective::ENDREPEAT: return ".ENDREPEAT";
                    case PreprocessorDirective::FOR: return ".FOR";
                    case PreprocessorDirective::ENDFOR: return ".ENDFOR";
                    case PreprocessorDirective::WHILE: return ".WHILE";
                    case PreprocessorDirective::ENDWHILE: return ".ENDWHILE";
                    case PreprocessorDirective::CONTINUE: return ".CONTINUE";
                    case PreprocessorDirective::BREAK: return ".BREAK";
                    case PreprocessorDirective::INCLUDE: return ".INCLUDE";
                    case PreprocessorDirective::ONCE: return ".ONCE";
                    case PreprocessorDirective::MACRO: return ".MACRO";
                    case PreprocessorDirective::ENDMACRO: return ".ENDMACRO";
                    case PreprocessorDirective::SHIFT: return ".SHIFT";
                    case PreprocessorDirective::RETURN: return ".RETURN";
                    case PreprocessorDirective::OPTION: return ".OPTION";
                    case PreprocessorDirective::PRINT: return ".PRINT";
                    case PreprocessorDirective::PRINTLN: return ".PRINTLN";
                    case PreprocessorDirective::INFO: return ".INFO";
                    case PreprocessorDirective::WARNING: return ".WARNING";
                    case PreprocessorDirective::ERROR: return ".ERROR";
                    case PreprocessorDirective::ASSERT: return ".ASSERT";
                    default: return "";
                } 
            },
            [] (SectionName pType) 
            { 
                switch (pType)
                {
                    case SectionName::METADATA: return "METADATA";
                    case SectionName::INT0: return "INT0";
                    case SectionName::INT1: return "INT1";
                    case SectionName::INT2: return "INT2";
                    case SectionName::INT3: return "INT3";
                    case SectionName::INT4: return "INT4";
                    case SectionName::INT5: return "INT5";
                    case SectionName::INT6: return "INT6";
                    case SectionName::INT7: return "INT7";
                    case SectionName::INT8: return "INT8";
                    case SectionName::INT9: return "INT9";
                    case SectionName::INT10: return "INT10";
                    case SectionName::INT11: return "INT11";
                    case SectionName::INT12: return "INT12";
                    case SectionName::INT13: return "INT13";
                    case SectionName::INT14: return "INT14";
                    case SectionName::INT15: return "INT15";
                    case SectionName::INT16: return "INT16";
                    case SectionName::INT17: return "INT17";
                    case SectionName::INT18: return "INT18";
                    case SectionName::INT19: return "INT19";
                    case SectionName::INT20: return "INT20";
                    case SectionName::INT21: return "INT21";
                    case SectionName::INT22: return "INT22";
                    case SectionName::INT23: return "INT23";
                    case SectionName::INT24: return "INT24";
                    case SectionName::INT25: return "INT25";
                    case SectionName::INT26: return "INT26";
                    case SectionName::INT27: return "INT27";
                    case SectionName::INT28: return "INT28";
                    case SectionName::INT29: return "INT29";
                    case SectionName::INT30: return "INT30";
                    case SectionName::INT31: return "INT31";
                    case SectionName::CODE: return "CODE";
                    case SectionName::DATA: return "DATA";
                    case SectionName::BSS: return "BSS";
                    default: return "";
                }
            },
            [] (PreprocessorFunction pType) 
            { 
                switch (pType)
                {
                    case PreprocessorFunction::HIGH: return "HIGH";
                    case PreprocessorFunction::LOW: return "LOW";
                    case PreprocessorFunction::HIDWORD: return "HIDWORD";
                    case PreprocessorFunction::LODWORD: return "LODWORD";
                    case PreprocessorFunction::HIWORD: return "HIWORD";
                    case PreprocessorFunction::LOWORD: return "LOWORD";
                    case PreprocessorFunction::HIBYTE: return "HIBYTE";
                    case PreprocessorFunction::LOBYTE: return "LOBYTE";
                    case PreprocessorFunction::HINIBBLE: return "HINIBBLE";
                    case PreprocessorFunction::LONIBBLE: return "LONIBBLE";
                    case PreprocessorFunction::BITWIDTH: return "BITWIDTH";
                    case PreprocessorFunction::TZCOUNT: return "TZCOUNT";
                    case PreprocessorFunction::FINT: return "FINT";
                    case PreprocessorFunction::FFRAC: return "FFRAC";
                    case PreprocessorFunction::FADD: return "FADD";
                    case PreprocessorFunction::FSUB: return "FSUB";
                    case PreprocessorFunction::FDIV: return "FDIV";
                    case PreprocessorFunction::FMUL: return "FMUL";
                    case PreprocessorFunction::FMOD: return "FMOD";
                    case PreprocessorFunction::FPOW: return "FPOW";
                    case PreprocessorFunction::FSQRT: return "FSQRT";
                    case PreprocessorFunction::FROOT: return "FROOT";
                    case PreprocessorFunction::FLOG: return "FLOG";
                    case PreprocessorFunction::FLN: return "FLN";
                    case PreprocessorFunction::FROUND: return "FROUND";
                    case PreprocessorFunction::FCEIL: return "FCEIL";
                    case PreprocessorFunction::FFLOOR: return "FFLOOR";
                    case PreprocessorFunction::FRADT: return "FRADT";
                    case PreprocessorFunction::FDEGT: return "FDEGT";
                    case PreprocessorFunction::FSIN: return "FSIN";
                    case PreprocessorFunction::FCOS: return "FCOS";
                    case PreprocessorFunction::FTAN: return "FTAN";
                    case PreprocessorFunction::FASIN: return "FASIN";
                    case PreprocessorFunction::FACOS: return "FACOS";
                    case PreprocessorFunction::FATAN: return "FATAN";
                    case PreprocessorFunction::FATAN2: return "FATAN2";
                    case PreprocessorFunction::STRCAT: return "STRCAT";
                    case PreprocessorFunction::STRNCAT: return "STRNCAT";
                    case PreprocessorFunction::STRUPR: return "STRUPR";
                    case PreprocessorFunction::STRLWR: return "STRLWR";
                    case PreprocessorFunction::STRSLICE: return "STRSLICE";
                    case PreprocessorFunction::STRREPLACE: return "STRREPLACE";
                    case PreprocessorFunction::STRFMT: return "STRFMT";
                    case PreprocessorFunction::STRLEN: return "STRLEN";
                    case PreprocessorFunction::STRCMP: return "STRCMP";
                    case PreprocessorFunction::STRNCMP: return "STRNCMP";
                    case PreprocessorFunction::STRFIND: return "STRFIND";
                    case PreprocessorFunction::STRRFIND: return "STRRFIND";
                    case PreprocessorFunction::STRNFIND: return "STRNFIND";
                    case PreprocessorFunction::STRNRFIND: return "STRNRFIND";
                    case PreprocessorFunction::BYTELEN: return "BYTELEN";
                    case PreprocessorFunction::STRBYTE: return "STRBYTE";
                    case PreprocessorFunction::DEFINED: return "DEFINED";
                    case PreprocessorFunction::ISCONST: return "ISCONST";
                    case PreprocessorFunction::ISMACRO: return "ISMACRO";
                    case PreprocessorFunction::ISSYMBOL: return "ISSYMBOL";
                    default: return "";
                }
            },
            [] (AssemblerDirective pType) 
            { 
                switch (pType)
                {
                    case AssemblerDirective::BYTE: return ".BYTE";
                    case AssemblerDirective::WORD: return ".WORD";
                    case AssemblerDirective::DWORD: return ".DWORD";
                    case AssemblerDirective::ASCIZ: return ".ASCIZ";
                    case AssemblerDirective::SPACE: return ".SPACE";
                    case AssemblerDirective::INCBIN: return ".INCBIN";
                    case AssemblerDirective::EXPORT: return ".EXPORT";
                    case AssemblerDirective::IMPORT: return ".IMPORT";
                    case AssemblerDirective::SECTION: return ".SECTION";
                    case AssemblerDirective::ORG: return ".ORG";
                    case AssemblerDirective::ALIGN: return ".ALIGN";
                    default: return "";
                }
            },
            [] (PreprocessorBuiltinSymbol pType) 
            { 
                switch (pType)
                {
                    case PreprocessorBuiltinSymbol::_NARG: return "_NARG";
                    case PreprocessorBuiltinSymbol::_TARG: return "_TARG";
                    case PreprocessorBuiltinSymbol::_LINE: return "_LINE";
                    case PreprocessorBuiltinSymbol::_FILE: return "_FILE";
                    case PreprocessorBuiltinSymbol::_DATE: return "_DATE";
                    case PreprocessorBuiltinSymbol::_TIME: return "_TIME";
                    case PreprocessorBuiltinSymbol::_VERSION: return "_VERSION";
                    case PreprocessorBuiltinSymbol::_UNIQUE: return "_UNIQUE";
                    case PreprocessorBuiltinSymbol::_RANDOM32: return "_RANDOM32";
                    case PreprocessorBuiltinSymbol::_RANDOM64: return "_RANDOM64";
                    default: return "";
                }
            },
            [] (Hint pType)
            {
                switch (pType)
                {
                    case Hint::FILE: return ".FILE";
                    case Hint::LINE: return ".LINE";
                    default: return "";
                }
            }
        }, mType);
    }

    auto Keyword::StringifyGroup () const -> std::string_view
    {
        switch (mGroup)
        {
            case KeywordGroup::Register: 
                return "Register";
            case KeywordGroup::Condition: 
                return "Condition";
            case KeywordGroup::InstructionType: 
                return "InstructionType";
            case KeywordGroup::PreprocessorDirective: 
                return "PreprocessorDirective";
            case KeywordGroup::SectionName: 
                return "SectionName";
            case KeywordGroup::PreprocessorFunction: 
                return "PreprocessorFunction";
            case KeywordGroup::AssemblerDirective: 
                return "AssemblerDirective";
            case KeywordGroup::PreprocessorBuiltinSymbol: 
                return "PreprocessorBuiltinSymbol";
            case KeywordGroup::Hint: 
                return "Hint";
            default: return "Unknown";
        }
    }
}
