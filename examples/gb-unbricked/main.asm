;
; @file     main.asm
; @brief    Contains the entry point for the breakout game example.
;

.once
.global main

; Includes *********************************************************************

.include "hardware.inc"
.include "tileset.inc"
.include "tilemap.inc"
.include "objects.inc"
.include "utils.inc"

; Metadata *********************************************************************

.section "Program Metadata", metadata
    .org $0000 .asciz "G10.BOY"
    .org $0008 .dword 0x2000
    .org $0010 .asciz "G10.BOY Breakout"
    
; RAM Variables ****************************************************************

.section "RAM Variables", bss[mWRAM]
    wCurrentKeys:   .byte 1
    wNewKeys:       .byte 1

; Entry Point ******************************************************************

.section "Entry Point", code
clear_wram:
    xor l0, l0
    st [wCurrentKeys], l0
    st [wNewKeys], l0

update_keys:

    ; Poll half of the gamepad.
    ld l0, JOYP_GET_BUTTONS
    call update_keys.one_nibble
    mv l1, l0

    ; Poll the other half.
    ld l0, JOYP_GET_DPAD
    call update_keys.one_nibble
    swap l0
    xor l0, l1
    mv l1, l0

    ; Release the controller.
    ld l0, JOYP_GET_NONE
    stp [rJOYP], l0

    ; Combine with previous `wCurrentKeys` to make `wNewKeys`.
    ld l0, [wCurrentKeys]
    xor l0, l1
    and l0, l1
    st [wNewKeys], l0
    st [wCurrentKeys], l1
    ret

update_keys.one_nibble:
    stp [rJOYP], l0
    ldp l0, [rJOYP]
    or l0, $F0
    ret

main:
    ld l0, 0
    stp [rNR52], l0
    call wait_vblank
    call lcd_off
    memcpy mVRAM + $1000, tiles, tiles_end - tiles
    memcpy mVRAM + $1800, tilemap, tilemap_end - tilemap
    memcpy mVRAM, paddle_left, paddle_left_end - paddle_left
    memcpy mVRAM + $10, paddle_right, paddle_right_end - paddle_right
    call clear_wram
    call clear_oam
    object 0, 16, 128, 0, 0
    object 1, 24, 128, 1, 0
    ld l0, %11100100
    stp [rBGP], l0
    stp [rOBP0], l0
    ld l0, LCDC_ON | LCDC_BG_ON | LCDC_OBJ_ON
    stp [rLCDC], l0
    jmp main_loop

main_loop:
    call wait_out_vblank
    call wait_vblank
    call update_keys

check_left:
    ld l0, [wCurrentKeys]
    and l0, JOYP_PAD_LEFT
    jpb zs, check_right
left:
    ld l0, [mOAM + 1]
    dec l0
    cmp l0, 15
    jpb zs, main_loop
    st [mOAM + 1], l0
    ld l0, [mOAM + 5]
    dec l0
    st [mOAM + 5], l0
    jpb main_loop

check_right:
    ld l0, [wCurrentKeys]
    and l0, JOYP_PAD_RIGHT
    jpb zs, main_loop
right:
    ld l0, [mOAM + 5]
    inc l0
    cmp l0, 105
    jpb zs, main_loop
    st [mOAM + 5], l0
    ld l0, [mOAM + 1]
    inc l0
    st [mOAM + 1], l0
    jpb main_loop
