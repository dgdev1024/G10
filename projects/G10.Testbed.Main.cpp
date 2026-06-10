/**
 * @file    G10.Testbed.Main.cpp
 * @brief   Contains the primary entry point for the G10 CPU's testbed
 *          emulator application, and related definitions.
 */

// Includes ********************************************************************

#include <print>
#include <G10.Testbed.System.hpp>

// Static Variables ************************************************************

namespace G10::Testbed
{
    static std::string sProgramName {};
    static std::string sROMFile {};
    static std::uint64_t sFrameLimit { 0 };
}

// Static Functions ************************************************************

namespace G10::Testbed
{
    static auto ParseArguments (int argc, const char** argv) -> bool
    {
        sProgramName = argv[0];
        for (int i = 1; i < argc; )
        {
            std::string arg = argv[i++];
            if (arg == "-r" || arg == "--rom")
            {
                if (i < argc)
                {
                    sROMFile = argv[i++];
                }
                else
                {
                    std::println(stderr,
                        "Error: Missing ROM file path after {}", arg);
                    return false;
                }
            }
            else if (arg == "-f" || arg == "--frames")
            {
                if (i < argc)
                {
                    try { sFrameLimit = std::stoull(argv[i++]); }
                    catch (...)
                    {
                        std::println(stderr, "Error: Invalid frame limit after {}", arg);
                        return false;
                    }
                }
                else
                {
                    std::println(stderr, "Error: Missing frame limit after {}", arg);
                    return false;
                }
            }
            else
            {
                std::println(stderr, "Error: Unknown argument {}", arg);
                return false;
            }
        }

        if (sROMFile.empty())
        {
            std::println(stderr, "Error: No ROM file specified");
            return false;
        }

        return true;
    }

    static auto Run (int argc, const char** argv) -> int
    {
        if (ParseArguments(argc, argv) == false)
            { return 1; }

        CPU::Program program;
        auto loadProgram = program.LoadFile(sROMFile);

        if (loadProgram.has_value() == false)
        {
            std::println(stderr, "Error: Failed to load ROM file '{}': '{}'", 
                sROMFile, loadProgram.error());
            return 1;
        }

        System testbed;
        testbed.SetProgram(program);
        std::uint64_t frames = 0;
        while (testbed.GetCPU().IsStopped() == false)
        {
            if (CPU::Executive::StepCore(testbed.GetCPU()) == false)
            {
                std::println(stderr, "Error: Failed to step CPU");
                testbed.Report();
                return 1;
            }

            frames++;
            if (sFrameLimit > 0 && frames >= sFrameLimit)
            {
                break;
            }
        }

        testbed.Report();
        return 0;
    }
}

// Main Function ***************************************************************

auto main (int argc, const char** argv) -> int
{
    return G10::Testbed::Run(argc, argv);
}
