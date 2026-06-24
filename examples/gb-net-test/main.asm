;
; @file     .asm
; @brief    Contains the main entry point of our program.
;

; Includes *********************************************************************

.include "hardware.inc"
.include "assets.inc"

; Interrupt Vectors ************************************************************

.section "Exception Handler", int0
    reti

.section "Vertical Blank Handler", int1
    reti

.section "Serial Handler", int4
    inc d10
    jmp SerialHandler

; Program Metadata *************************************************************

.section "Program Metadata", metadata
    .org $0000          .asciz "G10.BOY"
    .org $0008          .dword 0x2000
    .org $0010          .asciz "G10.BOY Serial Test"

; RAM Variables ****************************************************************

.section "RAM Variables", bss[mWRAM]
    wRole::             .byte 1
    wSendByte::         .byte 1
    wRecvByte::         .byte 1
    wJoypadState::      .byte 1
    wJoypadPressed::    .byte 1
    wHostPhase::        .byte 1

; Main Program *****************************************************************

.section "Main Program", code
    Main::

        ; Audio Off
        ld l0, 0
        stp [rNR52], l0
        
        ; LCD Off
        call WaitForVerticalBlank
        ld l0, 0
        stp [rLCDC], l0

        ; Copy Font to VRAM
        ld d13, mVRAM
        ld d14, FontData
        ld w15, FontDataEnd - FontData
        call CopyBytes

        ; Clear BG Map to blank tile (`$10`)
        ld d13, mVRAM + $1800
        ld l14, $10
        ld w15, 1024
        call SetBytes

        ; Initialize RAM variables
        xor l0, l0
        st [wRole], l0
        st [wSendByte], l0
        st [wRecvByte], l0
        st [wJoypadState], l0
        st [wJoypadPressed], l0
        st [wHostPhase], l0

        ; Prepare Palette
        ld l0, %10011011
        stp [rBGP], l0

        ; LCD On
        ld l0, %10010001
        stp [rLCDC], l0

        ; Prepare Interrupts
        ld l0, %00010011
        stp [rIE0], l0
        ei

    .loop:
        call ReadJoypad
        ld l0, [wJoypadPressed]
        bit 2, l0
        jpb zs, .checkA
        ld l0, [wRole]
        xor l0, 1
        st [wRole], l0

    .checkA:
        ld l0, [wRole]
        and l0, l0
        jpb zs, .clientLogic

    .hostLogic:
        ; Check 'A' button (Bit 0) to initiate transfer
        ld l0, [wJoypadPressed]
        bit 0, l0
        jpb zs, .endLogic
        
        ; Master: Increment byte, load SB, set Internal Clock ($81)
        ld l0, [wSendByte]
        inc l0
        st [wSendByte], l0
        stp [rSB], l0
        ld l0, 1
        st [wHostPhase], l0
        ld l0, $81
        stp [rSC], l0
        jpb .endLogic    

    .clientLogic:
        ldp l0, [rSC]
        bit 7, l0
        jpb zc, .endLogic
        ld l0, [wSendByte]
        stp [rSB], l0
        ld l0, $80
        stp [rSC], l0

    .endLogic:
        call UpdateUI
        halt
        nop
        jpb .loop
