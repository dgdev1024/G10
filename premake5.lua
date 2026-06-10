
-- Options ---------------------------------------------------------------------

newoption {
    trigger     = "build-static",
    description = "Build static libraries instead of shared/dynamic libraries"
}

-- Helper Functions ------------------------------------------------------------

function is_library (yes)
    if (yes == true) then
        if (_OPTIONS["build-static"] ~= nil) then
            defines { "G10_BUILD_STATIC" }
            kind "StaticLib"
        else
            defines { "G10_BUILD_SHARED" }
            kind "SharedLib"
        end
        pic "On"
    else
        kind "ConsoleApp"
        defines { "G10_BUILD_APP" }
    end
end

-- Workspace -------------------------------------------------------------------

workspace "G10"
    language "C++"
    cppdialect "C++23"
    configurations {
        "debug",
        "debug-slow",
        "release",
    }
    filter { "configurations:debug" }
        defines { "G10_CONFIG_DEBUG", "DEBUG" }
        symbols "On"
        optimize "Debug"
    filter { "configurations:debug-slow" }
        defines { "G10_CONFIG_DEBUG", "G10_CONFIG_DEBUG_SLOW", "DEBUG" }
        symbols "On"
        optimize "Off"
    filter { "configurations:release" }
        defines { "G10_CONFIG_RELEASE", "NDEBUG" }
        optimize "Full"
    filter { "system:windows" }
        defines { "G10_SYSTEM_WINDOWS" }
    filter { "system:linux" }
        defines { "G10_SYSTEM_LINUX" }
    filter { "system:macos" }
        defines { "G10_SYSTEM_MACOS" }
    filter {}

-- Includes --------------------------------------------------------------------

include "projects/G10.Premake5.lua"
