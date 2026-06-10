;
; @file     utils.inc
; @brief    Contains some useful utility macros and subroutines.
;

.once
.include "hardware.inc"

; Macros ***********************************************************************

.macro memcpy dest, src, bytes
    ld d14, @src
    ld d15, @dest
    ld w13, @bytes
    call _memcpy
.endm

.macro object index, x, y, tile, attribs
    ld d1, mOAM + (@index * 4)
    ld l0, @y + 16
    sti [d1], l0
    ld l0, @x + 8
    sti [d1], l0
    ld l0, @tile
    sti [d1], l0
    ld l0, @attribs
    sti [d1], l0
.endm

.macro move_object_x index, mag_x
    .if @mag_x == 0
        .return
    .endif

    ld l0, [mOAM + (@index * 4) + 1]
    .if @mag_x > 1
        add l0, @mag_x
    .elif @mag_x == 1
        inc l0
    .elif @mag_x == -1
        dec l0
    .elif @mag_x < -1
        sub l0, -@mag_x
    .endif
    st [mOAM + (@index * 4) + 1], l0
.endm

; Subroutines ******************************************************************

.section "Utility Subroutines", code

;
; @brief    Waits for the next vertical blanking period.
;
wait_vblank:
    ldp l0, [rLY]
    cmp l0, LY_VBLANK
    jpb cs, wait_vblank
    ret

;
; @brief    Waits for the end of the current vertical blanking period.
;
wait_out_vblank:
    ldp l0, [rLY]
    cmp l0, LY_VBLANK
    jpb cc, wait_out_vblank
    ret

;
; @brief    Turns the PPU's LCD off.
; @warning  Call `wait_vblank` first, before calling this.
;
lcd_off:
    ldp l0, [rLCDC]
    and l0, ~LCDC_ENABLE
    stp [rLCDC], l0
    ret

;
; @brief    Turns the PPU's LCD back on.
;
lcd_on:
    ldp l0, [rLCDC]
    or l0, LCDC_ENABLE
    stp [rLCDC], l0
    ret

;
; @brief    Clears the G10.Boy's object attribute memory (OAM).
;
clear_oam:
    ld l0, 0
    ld l1, OAM_SIZE
    ld d2, mOAM
clear_oam_loop:
    sti [d2], l0
    dec l1
    jpb zc, clear_oam_loop
    ret

;
; @brief    Copy bytes from one area to another.
; @note     Use the `memcpy` macro to call this subroutine.
;
; @param    d14     Source Address
; @param    d15     Destination Address
; @param    w13     Length In Bytes
;
_memcpy:
    mv l0, h13
    or l0, l13
    ret zs
    xor l0, l0
_memcpy_loop:
    ld l0, [d14]
    sti [d15], l0
    inc d14
    dec w13
    mv l0, h13
    or l0, l13
    jpb zc, _memcpy_loop
    ret
