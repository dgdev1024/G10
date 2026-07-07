#pragma once

#if !defined(G10_CONFIG_DEBUG)

#include <string>
namespace G10::Boy
{
    constexpr std::string_view kIniData = R"(
        [Window][WindowOverViewport_11111111]
        Pos=0,19
        Size=1280,701
        Collapsed=0

        [Window][Debug##Default]
        Pos=60,60
        Size=400,400
        Collapsed=0

        [Window][Dear ImGui Demo]
        Pos=0,663
        Size=313,380
        Collapsed=0
        DockId=0x00000009,0

        [Window][About Dear ImGui]
        Pos=60,60
        Size=592,217
        Collapsed=0

        [Window][Emulation]
        Pos=341,19
        Size=505,701
        Collapsed=0
        DockId=0x00000002,0

        [Window][Dear ImGui Metrics/Debugger]
        Pos=315,19
        Size=331,1024
        Collapsed=0
        DockId=0x00000001,0

        [Window][Registers]
        Pos=848,19
        Size=432,298
        Collapsed=0
        DockId=0x00000007,0

        [Window][Memory]
        Pos=848,319
        Size=432,401
        Collapsed=0
        DockId=0x00000008,0

        [Window][Tiles]
        Pos=0,19
        Size=339,440
        Collapsed=0
        DockId=0x0000000B,0

        [Window][Networking]
        Pos=0,461
        Size=339,259
        Collapsed=0
        DockId=0x0000000C,0

        [Docking][Data]
        DockSpace         ID=0x08BD597D Window=0x1BBC0F80 Pos=0,19 Size=1280,701 Split=X
        DockNode        ID=0x00000003 Parent=0x08BD597D SizeRef=339,812 Split=Y Selected=0x9CF383C4
            DockNode      ID=0x0000000B Parent=0x00000003 SizeRef=339,510 Selected=0x9CF383C4
            DockNode      ID=0x0000000C Parent=0x00000003 SizeRef=339,300 Selected=0xE5857C2D
        DockNode        ID=0x00000004 Parent=0x08BD597D SizeRef=1099,812 Split=X
            DockNode      ID=0x00000009 Parent=0x00000004 SizeRef=362,812 Selected=0x9CF383C4
            DockNode      ID=0x0000000A Parent=0x00000004 SizeRef=1556,812 Split=X
            DockNode    ID=0x00000005 Parent=0x0000000A SizeRef=665,701 Split=X
                DockNode  ID=0x00000001 Parent=0x00000005 SizeRef=331,701 Selected=0xD9E076F4
                DockNode  ID=0x00000002 Parent=0x00000005 SizeRef=670,701 CentralNode=1 Selected=0xA51CA92B
            DockNode    ID=0x00000006 Parent=0x0000000A SizeRef=432,701 Split=Y Selected=0x387A5E37
                DockNode  ID=0x00000007 Parent=0x00000006 SizeRef=239,298 Selected=0x387A5E37
                DockNode  ID=0x00000008 Parent=0x00000006 SizeRef=239,401 Selected=0x561FC6F4
    )";
}

#endif
