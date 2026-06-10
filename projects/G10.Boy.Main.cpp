/**
 * @file    G10.Boy.Main.cpp
 * @brief   Contains the primary entry point for the G10.Boy emulator
 *          application, and related definitions.
 */

// Includes ********************************************************************

#include <G10.Boy.Application.hpp>

// Main Function ***************************************************************

auto main (int argc, const char** argv) -> int
{
    G10::Boy::Application app { argc, argv };
    return app.Start();
}
