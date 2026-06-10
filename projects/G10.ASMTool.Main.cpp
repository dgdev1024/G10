/**
 * @file    G10.ASMTool.Main.cpp
 * @brief   Contains the primary entry point for the G10 CPU Assembler & Linker
 *          Tool, and related definitions.
 */

// Includes ********************************************************************

#include <print>
#include <G10.ASM.Lexer.hpp>
#include <G10.ASM.Preprocessor.hpp>
#include <G10.ASM.Parser.hpp>
#include <G10.ASM.Codegen.hpp>
#include <G10.ASM.Object.hpp>
#include <G10.ASM.Linker.hpp>
#include <G10.CPU.Program.hpp>

// Static Variables ************************************************************

namespace G10::ASM::Tool
{
    static std::string                  sProgramName { "" };
    static bool                         sShowVersion { false };
    static bool                         sShowHelp { false };
    static bool                         sVerbose { false };
    static bool                         sWextra { false };
    static bool                         sWerror { false };
    static bool                         sSkipPreprocess { false };
    static bool                         sLex { false };
    static bool                         sPreprocessOnly { false };
    static bool                         sOutputPreprocess { false };
    static bool                         sParseOnly { false };
    static bool                         sExamineObject { false };
    static bool                         sExamineProgram { false };
    static std::size_t                  sInterpolationDepth { kDefaultInterpolationDepth };
    static std::size_t                  sLoopDepth { kDefaultLoopDepth };
    static std::size_t                  sIncludeDepth { kDefaultIncludeDepth };
    static std::size_t                  sRecursionDepth { kDefaultRecursionDepth };
    static std::string                  sInputFile { "" };
    static std::string                  sOutputFile { "" };
    static std::vector<std::string>     sLinkFiles {};
    static std::vector<std::string>     sIncludeDirs {};
    static bool                         sTest { false };
}

// Static Functions ************************************************************

namespace G10::ASM::Tool
{
    static auto ParseArguments (int argc, const char** argv) -> bool
    {
        auto ParseNumericArgument = [&] (int& i, const std::string& pKey,
            std::size_t& pValue) -> bool
        {
            if (i >= argc || std::string { argv[i] }.starts_with('-'))
            {
                std::println(stderr,
                    "Error: Missing numeric value after '{}'.", pKey);
                return false;
            }

            try
            {
                pValue = std::stoull(argv[i + 1], nullptr, 10);
                return true;
            }
            catch (...)
            {
                std::println("Invalid value to numeric argument '{}'.", pKey);
                return false;
            }
        };

        sProgramName = argv[0];
        for (int i = 1; i < argc; )
        {
            std::string arg { argv[i++] };
            if (arg == "-h" || arg == "--help")
                { sShowHelp = true; }
            else if (arg == "-v" || arg == "--version")
                { sShowVersion = true; }
            else if (arg == "-V" || arg == "--verbose")
                { sVerbose = true; }
            else if (arg == "-Wextra" || arg == "--wextra")
                { sWextra = true; }
            else if (arg == "-Werror" || arg == "--werror")
                { sWerror = true; }
            else if (arg == "-s" || arg == "--skip-preprocess")
                { sSkipPreprocess = true; }
            else if (arg == "-l" || arg == "--lex")
                { sLex = true; }
            else if (arg == "-p" || arg == "--preprocess")
                { sPreprocessOnly = true; }
            else if (arg == "-r" || arg == "--parse")
                { sParseOnly = true; }
            else if (arg == "--output-preprocess")
                { sOutputPreprocess = true; }
            else if (arg == "-x" || arg == "--examine-object")
                { sExamineObject = true; }
            else if (arg == "-y" || arg == "--examine-program")
                { sExamineProgram = true; }
            else if (arg == "-t" || arg == "--test")
                { sTest = true; }
            else if (arg == "--max-interpolation-depth" && 
                    ParseNumericArgument(i, arg, sInterpolationDepth) == false)
                { return false; }
            else if (arg == "--max-loop-depth" && 
                    ParseNumericArgument(i, arg, sLoopDepth) == false)
                { return false; }
            else if (arg == "--max-include-depth" && 
                    ParseNumericArgument(i, arg, sIncludeDepth) == false)
                { return false; }
            else if (arg == "--max-recursion-depth" && 
                    ParseNumericArgument(i, arg, sRecursionDepth) == false)
                { return false; }

            else if (arg == "-i" || arg == "--input-file")
            {
                if (i >= argc || std::string { argv[i] }.starts_with('-'))
                {
                    std::println(stderr,
                        "Error: Missing input file after '{}'.", arg);
                    return false;
                }

                sInputFile = argv[i++];
            }
            else if (arg == "-o" || arg == "--output-file")
            {
                if (i >= argc || std::string { argv[i] }.starts_with('-'))
                {
                    std::println(stderr,
                        "Error: Missing output file after '{}'.", arg);
                    return false;
                }

                sOutputFile = argv[i++];
            }
            else if (arg == "-L" || arg == "--link-file")
            {
                while (i < argc && std::string { argv[i] }.starts_with('-') == false)
                {
                    sLinkFiles.push_back(std::string { argv[i++] });
                }

                if (sLinkFiles.empty() == true)
                {
                    std::println(stderr,
                        "Error: Missing link files after '{}'.", arg);
                    return false;
                }
            }
            else if (arg == "-I" || arg == "--include-dirs")
            {
                while (i < argc && std::string { argv[i] }.starts_with('-') == false)
                {
                    sIncludeDirs.push_back(std::string { argv[i++] });
                }

                if (sIncludeDirs.empty() == true)
                {
                    std::println(stderr,
                        "Error: Missing include directory after '{}'.", arg);
                    return false;
                }
            }
        }

        if (sShowHelp == true || sShowVersion == true || sTest == true)
            { return true; }

        if (sSkipPreprocess == true && (sPreprocessOnly == true || sOutputPreprocess == true))
        {
            std::println(stderr,
                "Error: '--skip-preprocess' is mutually exclusive with "
                "'--preprocess' and '--output-preprocess'.");
            return false;
        }

        if (sInputFile.empty() == true && sLinkFiles.empty() == true)
        {
            std::println(stderr, 
                "Error: Missing required argument: -i <file>, --input-file <file>.");
            return false;
        }

        if (
            sLex == true || 
            sPreprocessOnly == true || 
            sParseOnly == true || 
            sExamineObject == true ||
            sExamineProgram == true
        )
        {
            return true;
        }

        if (sOutputFile.empty() == true)
        {
            std::println(stderr,
                "Error: Missing required argument: -o <file>, --output-file <file>.");
            return false;
        }

        return true;
    }

    static auto DisplayVersion () -> void
    {
        std::println("G10.ASM - G10 Virtual CPU Assembler & Linker Tool");
        std::println("Version: {}.{}.{}", kMajorVersion, kMinorVersion,
            kPatchVersion);
        std::println("By: Dennis W. Griffin <dgdev1024@gmail.com>\n");
    }

    static auto DisplayHelp () -> void
    {
        std::println("Usage: {} [OPTIONS]\n", sProgramName);
        std::println(
            "Options:\n"
            "  -i <file>, --input-file <file>   Required. Input source file to assemble.\n"
            "  -o <file>, --output-file <file>  Required. Output file to generate.\n"
            "  -L <files>, --link-files <files> Link object files into an output executable.\n"
            "  -I <dirs>, --include-dirs <dirs> Include directories for preprocessing and binary inclusion.\n"
            "                                   Ignored if '--lex', '--preprocess' or '--parse'\n"
            "                                   is also specified.\n"
            "  --verbose                        Enable verbose diagnostic output.\n"
            "  --wextra                         Enable extra, verbose warnings.\n"
            "  --werror                         Treat all warnings as errors.\n"
            "  --skip-preprocess                Skips first-pass lexing and preprocessing.\n"
            "                                   Mutually exclusive with '--preprocess' and\n"
            "                                   '--output-preprocess'.\n"
            "  --lex                            Prints the result of first-pass lexing.\n"
            "                                   If '--skip-preprocess' or '--preprocess' is also\n"
            "                                   specified, prints the result of second-pass\n"
            "                                   lexing, instead.\n"
            "  --preprocess                     Performs up to the preprocessing step only.\n"
            "                                   Mutually exclusive with '--skip-preprocess'.\n"
            "  --parse                          Parses the input file and displays the syntax tree.\n"
            "  --output-preprocess              Prints the preprocessed source code string.\n"
            "                                   Mutually exclusive with '--skip-preprocess'.\n"
            "  -x, --examine-object             Examines an assembled object file.\n"
            "  -y, --examine-program            Examines an assembled program file.\n"
            "                                   If specified, the input file is expected to be an object file.\n"
            "  --max-interpolation-depth <N>    Sets the maximum interpolated expression depth\n"
            "                                   allowed by the preprocessor.\n"
            "  --max-loop-depth <N>             Sets the maximum looping depth\n"
            "                                   allowed by the preprocessor.\n"
            "  --max-include-depth <N>          Sets the maximum include file depth\n"
            "                                   allowed by the preprocessor.\n"
            "  --max-loop-depth <N>             Sets the maximum looping depth\n"
            "                                   allowed by the preprocessor.\n"
            "  -v, --version                    Display version information, then exit.\n"
            "  -h, --help                       Display this help message, then exit.\n"
        );
    }

    static auto DisplayReport (const DiagnosticReport& pReport) -> void
    {
        auto stream = (
            pReport.mLevel == DiagnosticLevel::Verbose ||
            pReport.mLevel == DiagnosticLevel::Info
        ) ? stdout : stderr;

        std::print(stream, " - {}", pReport.mLocation.ToString());
        switch (pReport.mLevel)
        {
            case DiagnosticLevel::Verbose:
            case DiagnosticLevel::Info:
                std::println(stream, "Info: {}", pReport.mMessage);
                break;
            case DiagnosticLevel::ExtraWarning:
            case DiagnosticLevel::Warning:
                std::println(stream, "Warning: {}", pReport.mMessage);
                break;
            case DiagnosticLevel::Error:
                std::println(stream, "Error: {}", pReport.mMessage);
                break;
        }
    }

    static auto ReportDiagnostic (const Diagnostic& diag) -> void
    {
        auto    warningCount = diag.GetWarningCount(),
                werrorCount = diag.GetWerrorCount(),
                errorCount = diag.GetErrorCount();

        if (warningCount == 0 && werrorCount == 0 && errorCount == 0)
            { return; }

        std::print(stderr, "\n");
        if (werrorCount > 0)
            { std::println(stderr, "Note: All warnings being treated as errors."); }
        if (errorCount > 0)
            { std::println(stderr, "{} error(s) generated.", errorCount); }
        if (warningCount > 0)
            { std::println(stderr, "{} warning(s) generated.", warningCount); }
    }

    static auto ReportLex (const Lexer& lex) -> void
    {
        const auto& tokens = lex.GetTokens();
        for (const auto& token : tokens)
        {
            std::print(" - {}: {}", token.mLocation.ToString(),
                token.StringifyType());

            switch (token.mType)
            {
                case TokenType::Identifier:
                case TokenType::Parameter:
                case TokenType::StringLiteral:
                    std::print(" = '{}'", token.Stringify().value_or(""));
                    break;
                case TokenType::BinaryLiteral:
                case TokenType::OctalLiteral:
                case TokenType::DecimalLiteral:
                case TokenType::HexadecimalLiteral:
                    std::print(" = '{}' ({})", token.Stringify().value_or(""),
                        token.mInteger.value_or(0));
                    break;
                case TokenType::FloatingPointLiteral:
                    std::print(" = '{}' ({})", token.Stringify().value_or(""),
                        token.mFloat.value_or(0.0));
                    break;
                default: break;
            }

            std::println();
        }
    }

    static auto ReportPreprocess (const Preprocessor& pp) -> void
    {
        auto output = pp.GetOutput();
        output.erase(
            std::unique(output.begin(), output.end(),
                [] (char a, char b) -> bool { return a == '\n' && b == '\n'; }
            ), output.end()
        );

        std::println("{}", output);
    }

    static auto ReportParse (const Parser& parser) -> void
    {
        std::println("{}", parser.GetOutput().Stringify());
    }

    static auto ExamineObject () -> bool
    {
        Object obj;
        if (auto good = obj.LoadFile(sInputFile); !good)
        {
            std::println(stderr, "{}", good.error());
            return false;
        }

        const auto& header = obj.GetHeader();
        const auto& sectionTable = obj.GetSectionTable();
        const auto& symbolTable = obj.GetSymbolTable();
        const auto& relocationTable = obj.GetRelocationTable();
        const auto& stringTable = obj.GetStringTable();
        const auto& dataTable = obj.GetDataTable();

        std::println("\nObject Header Details:");
        std::println(" - Magic: 0x{:08X}", header.mMagicNumber);
        std::println(" - Version: {}", header.mVersion);
        std::println(" - Section Table Offset: 0x{:08X}", header.mSectionTableOffset);
        std::println(" - Section Count: {}", header.mSectionCount);
        std::println(" - Symbol Table Offset: 0x{:08X}", header.mSymbolTableOffset);
        std::println(" - Symbol Count: {}", header.mSymbolCount);
        std::println(" - Relocation Table Offset: 0x{:08X}", header.mRelocationTableOffset);
        std::println(" - Relocation Count: {}", header.mRelocationCount);
        std::println(" - String Table Offset: 0x{:08X}", header.mStringTableOffset);
        std::println(" - String Buffer Size: {}", header.mStringBufferSize);
        std::println(" - String Count: {}", header.mStringCount);
        std::println(" - Data Buffer Offset: 0x{:08X}", header.mDataBufferOffset);

        std::println("\nObject Section Table:");
        for (std::size_t i = 0; i < sectionTable.size(); ++i)
        {
            const auto& section = sectionTable[i];
            std::println(" - Section {}:", i);
            std::println("   - Name String Offset: 0x{:08X}", section.mNameStringOffset);
            std::print  ("   - Type: ");
            switch (section.mType)
            {
                case ObjectSectionType::Metadata: std::println("Metadata"); break;
                case ObjectSectionType::Interrupt: std::println("Interrupt"); break;
                case ObjectSectionType::Code: std::println("Code"); break;
                case ObjectSectionType::Data: std::println("Data"); break;
                case ObjectSectionType::BSS: std::println("BSS"); break;
            }
            if (section.mType == ObjectSectionType::Interrupt)
                { std::println("   - Interrupt Number: {}", section.mInterruptNumber); }
            std::println("   - Alignment Boundary: {}", section.mAlignmentBoundary);
            std::println("   - Target Address: 0x{:08X}", section.mTargetAddress);
            std::println("   - Data Offset: 0x{:08X}", section.mDataOffset);
            std::println("   - Data Size: {}", section.mDataSize);
        }

        std::println("\nObject Symbol Table:");
        for (std::size_t i = 0; i < symbolTable.size(); ++i)
        {
            const auto& symbol = symbolTable[i];
            std::println(" - Symbol {}:", i);
            std::println("   - Name String Offset: 0x{:08X}", symbol.mNameStringOffset);
            std::print  ("   - Type: ");
            switch (symbol.mType)
            {
                case ObjectSymbolType::Local: std::println("Local"); break;
                case ObjectSymbolType::Import: std::println("Import"); break;
                case ObjectSymbolType::Export: std::println("Export"); break;
            }
            std::println("   - Address Offset: 0x{:08X}", symbol.mAddressOffset);
            std::println("   - Section Index: {}", symbol.mSectionIndex);
        }

        std::println("\nObject Relocation Table:");
        for (std::size_t i = 0; i < relocationTable.size(); ++i)
        {
            const auto& relocation = relocationTable[i];
            std::println(" - Relocation {}:", i);
            std::println("   - Patch Offset: 0x{:08X}", relocation.mPatchOffset);
            std::println("   - Symbol Index: {}", relocation.mSymbolIndex);
            std::println("   - Section Index: {}", relocation.mSectionIndex);
            std::print  ("   - Type: ");
            switch (relocation.mType)
            {
                case ObjectRelocationType::Absolute: std::println("Absolute"); break;
                case ObjectRelocationType::Relative: std::println("Relative"); break;
            }
            std::println("   - Patch Size: {} bytes", relocation.mSize);
        }

        std::println("\nObject String Table:");
        std::println(" - 0. \"\" (empty string, always present)");
        for (std::size_t i = 1; i < stringTable.size(); ++i)
        {
            std::println(" - {}. \"{}\"", i, stringTable[i]);
        }

        std::println("\nObject Data Table:");
        for (std::size_t i = 0; i < dataTable.size(); ++i)
        {
            const auto& data = dataTable[i];

            std::print(" - Data {}:", i);
            for (std::size_t j = 0; j < data.size(); ++j)
            {
                if (j % 16 == 0)
                    { std::print("\n    "); }
                std::print("0x{:02X} ", data[j]);
            }

            if (data.empty() == true) { std::println(); }
            else { std::println("\n"); }
        }

        return true;
    }

    static auto ExamineProgram () -> bool
    {
        G10::CPU::Program prg;
        if (auto good = prg.LoadFile(sInputFile); !good)
        {
            std::println(stderr, "{}", good.error());
            return false;
        }

        const auto& header = prg.GetHeader();
        const auto& sectionTable = prg.GetSectionTable();
        const auto& symbolTable = prg.GetSymbolTable();
        const auto& dataTable = prg.GetDataTable();

        std::println("\nProgram Header Details:");
        std::println(" - Magic: 0x{:08X}", header.mMagicNumber);
        std::println(" - Version: {}", header.mVersion);
        std::println(" - Entry Point: 0x{:08X}", header.mEntryPoint);
        std::println(" - Section Table Offset: 0x{:08X}", header.mSectionTableOffset);
        std::println(" - Section Count: {}", header.mSectionCount);
        std::println(" - Symbol Table Offset: 0x{:08X}", header.mSymbolTableOffset);
        std::println(" - Symbol Count: {}", header.mSymbolCount);
        std::println(" - Data Buffer Offset: 0x{:08X}", header.mDataBufferOffset);

        std::println("\nProgram Section Table:");
        for (std::size_t i = 0; i < sectionTable.size(); ++i)
        {
            const auto& section = sectionTable[i];
            std::println(" - Section {}:", i);
            std::print  ("   - Type: ");
            switch (section.mType)
            {
                case ProgramSectionType::Metadata: std::println("Metadata"); break;
                case ProgramSectionType::Interrupt: std::println("Interrupt"); break;
                case ProgramSectionType::Code: std::println("Code"); break;
                case ProgramSectionType::Data: std::println("Data"); break;
                case ProgramSectionType::BSS: std::println("BSS"); break;
            }
            if (section.mType == ProgramSectionType::Interrupt)
                { std::println("   - Interrupt Number: {}", section.mInterruptNumber); }
            std::println("   - Alignment Boundary: {}", section.mAlignmentBoundary);
            std::println("   - Target Address: 0x{:08X}", section.mTargetAddress);
            std::println("   - Data Offset: 0x{:08X}", section.mDataOffset);
            std::println("   - Data Size: {}", section.mDataSize);
        }

        std::println("\nProgram Data Table:");
        for (std::size_t i = 0; i < dataTable.size(); ++i)
        {
            const auto& data = dataTable[i];

            std::print(" - Data {}:", i);
            for (std::size_t j = 0; j < data.size(); ++j)
            {
                if (j % 16 == 0)
                    { std::print("\n    "); }
                std::print("0x{:02X} ", data[j]);
            }

            if (data.empty() == true) { std::println(); }
            else { std::println("\n"); }
        }

        return true;
    }

    static auto Test () -> int
    {
        return 0;
    }

    static auto Run (int argc, const char** argv) -> int
    {
        if (ParseArguments(argc, argv) == false)
            { DisplayHelp(); return 1; }
        if (sShowHelp == true) { DisplayVersion(); DisplayHelp(); return 0; }
        if (sShowVersion == true) { DisplayVersion(); return 0; }

        if (sTest == true)
        {
            return Test();
        }
        else if (sExamineObject == true)
        {
            return ExamineObject() ? 0 : 1;
        }
        else if (sExamineProgram == true)
        {
            return ExamineProgram() ? 0 : 1;
        }

        Diagnostic diag;
        diag.SetCallback(DisplayReport);
        diag.SetVerboseEnabled(sVerbose);
        diag.SetExtraWarningsEnabled(sWextra);
        diag.SetWarningsAreErrors(sWerror);

        if (sLinkFiles.empty() == false)
        {
            Linker linker { diag };
            return (
                linker.Link(sLinkFiles) &&
                linker.SaveImage(sOutputFile)
            ) ? 0 : 1;
        }

        Lexer lex { diag };

        if (sSkipPreprocess == true)
        {
            if (lex.LexFile(sInputFile, true) == false)
            {
                ReportDiagnostic(diag);
                return 1;
            }
            else if (sLex == true)
            {
                ReportLex(lex);
                return 0;
            }
        }
        else
        {
            Preprocessor pp { diag };
            pp.SetInterpolationDepthLimit(sInterpolationDepth);
            pp.SetIncludeDepthLimit(sIncludeDepth);
            pp.SetRecursionDepthLimit(sRecursionDepth);
            pp.SetLoopDepthLimit(sLoopDepth);
            pp.SetIncludeDirectories(sIncludeDirs);

            if (lex.LexFile(sInputFile, false) == false)
            {
                ReportDiagnostic(diag);
                return 1;
            }
            else if (sLex == true && sOutputPreprocess == false)
            {
                ReportLex(lex);
                return 0;
            }

            if (pp.PreprocessInput(lex.GetTokens()) == false)
            {
                ReportDiagnostic(diag);
                return 1;
            }
            else if (sLex == false && sOutputPreprocess == true)
            {
                ReportPreprocess(pp);
                return 0;
            }

            if (sPreprocessOnly == true)
            {
                return 0;
            }

            if (lex.LexString(pp.GetOutput(), true) == false)
            {
                ReportDiagnostic(diag);
                return 1;
            }
            else if (sLex == true && sOutputPreprocess == true)
            {
                ReportLex(lex);
                return 0;
            }
        }

        Parser parser { diag };
        if (parser.ParseInput(lex.GetTokens()) == false)
        {
            ReportDiagnostic(diag);
            return 1;
        }
        else if (sParseOnly == true)
        {
            ReportParse(parser);
            return 0;
        }

        Codegen codegen { diag };
        codegen.SetIncludeDirectories(sIncludeDirs);
        if (codegen.Run(parser.GetOutput()) == false)
        {
            ReportDiagnostic(diag);
            return 1;
        }

        bool ok = codegen.SaveOutput(sOutputFile);
        ReportDiagnostic(diag);
        return (ok == true) ? 0 : 1;
    }
}

// Main Function ***************************************************************

auto main (int argc, const char** argv) -> int
{
    return G10::ASM::Tool::Run(argc, argv);
}
