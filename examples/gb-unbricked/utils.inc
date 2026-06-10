;
; @file     utils.inc
; @brief    Contains some useful utility macros and subroutines.
;

.once

; Macros ***********************************************************************

.macro memcpy dest, src, bytes
    ld d14, @src
    ld d15, @dest
    ld w13, @bytes
    call _memcpy
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
