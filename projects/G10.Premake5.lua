
-- `G10.CPU` - G10 Virtual CPU Library ------------------------------------------

project "G10.CPU"
    is_library(true)
    location "../build"
    targetdir "../build/bin/%{cfg.system}-%{cfg.buildcfg}"
    objdir "../build/obj/%{cfg.system}-%{cfg.buildcfg}/%{prj.name}"
    includedirs { "." }
    files {
        "./G10.Common.hpp",
        "./G10.CPU.**.hpp",
        "./G10.CPU.**.cpp"
    }

-- `G10.ASM` - G10 CPU Assembler & Linker Library ------------------------------

project "G10.ASM"
    is_library(true)
    location "../build"
    targetdir "../build/bin/%{cfg.system}-%{cfg.buildcfg}"
    objdir "../build/obj/%{cfg.system}-%{cfg.buildcfg}/%{prj.name}"
    includedirs { "." }
    files {
        "./G10.Common.hpp",
        "./G10.CPU.**.hpp",
        "./G10.ASM.**.hpp",
        "./G10.ASM.**.cpp"
    }
    links { "G10.CPU" }

-- `G10.ASMTool` - G10 CPU Assembler & Linker Tool -----------------------------

project "G10.ASMTool"
    is_library(false)
    location "../build"
    targetname "g10-asm"
    targetdir "../build/bin/%{cfg.system}-%{cfg.buildcfg}"
    objdir "../build/obj/%{cfg.system}-%{cfg.buildcfg}/%{prj.name}"
    includedirs { "." }
    files {
        "./G10.Common.hpp",
        "./G10.CPU.**.hpp",
        "./G10.ASM.**.hpp",
        "./G10.ASMTool.**.cpp",
        "./G10.ASMTool.**.cpp"
    }
    links { "G10.ASM", "G10.CPU" }
    
-- `G10.Testbed` - G10 CPU Testbed Emulator Application ------------------------

project "G10.Testbed"
    is_library(false)
    location "../build"
    targetname "g10-testbed"
    targetdir "../build/bin/%{cfg.system}-%{cfg.buildcfg}"
    objdir "../build/obj/%{cfg.system}-%{cfg.buildcfg}/%{prj.name}"
    includedirs { "." }
    files {
        "./G10.Common.hpp",
        "./G10.CPU.**.hpp",
        "./G10.Testbed.**.hpp",
        "./G10.Testbed.**.cpp"
    }
    links { "G10.CPU" }

-- `G10.GB` - G10-Based Game Boy Implementation --------------------------------

project "G10.GB"
    is_library(true)
    location "../build"
    targetdir "../build/bin/%{cfg.system}-%{cfg.buildcfg}"
    objdir "../build/obj/%{cfg.system}-%{cfg.buildcfg}/%{prj.name}"
    includedirs { "." }
    files {
        "./G10.Common.hpp",
        "./G10.CPU.**.hpp",
        "./G10.GB.**.hpp",
        "./G10.GB.**.cpp"
    }
    links { "G10.CPU" }

-- `G10.Boy` - G10-Based Game Boy Emulator Application -------------------------

project "G10.Boy"
    is_library(false)
    location "../build"
    targetname "g10-boy"
    targetdir "../build/bin/%{cfg.system}-%{cfg.buildcfg}"
    objdir "../build/obj/%{cfg.system}-%{cfg.buildcfg}/%{prj.name}"
    includedirs { ".", "../vendor/imgui", "../vendor/pfd" }
    files {
        "./G10.Common.hpp",
        "./G10.CPU.**.hpp",
        "./G10.GB.**.hpp",
        "./G10.Boy.**.hpp",
        "./G10.Boy.**.cpp",
        "../vendor/imgui/*.h",
        "../vendor/imgui/*.cpp",
        "../vendor/pfd/pfd.hpp",
    }
    links { "G10.CPU", "G10.GB", "SDL3" }
