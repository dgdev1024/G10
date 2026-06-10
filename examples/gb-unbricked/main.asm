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
.include "utils.inc"

; Metadata *********************************************************************

.section "Program Metadata", metadata
    .org $0000 .asciz "G10.BOY"
    .org $0008 .dword 0x2000
    .org $0010 .asciz "G10.BOY Breakout"

; Entry Point ******************************************************************

.section "Entry Point", code
main:
    ld l0, 0
    stp [rNR52], l0
    call wait_vblank
    call lcd_off
    memcpy mVRAM + $1000, tiles, tiles_end - tiles
    memcpy mVRAM + $1800, tilemap, tilemap_end - tilemap
    ld l0, %11100100
    stp [rBGP], l0
    ld l0, LCDC_ON | LCDC_BG_ON
    stp [rLCDC], l0

done:
    jpb done
