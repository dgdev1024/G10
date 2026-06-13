.global main
.include "hardware.inc"
.include "tile-data.inc"

.section "G10.BOY Metadata", metadata
    .org $0000 .asciz "G10.BOY"
    .org $0008 .dword 0x2000
    .org $0010 .asciz "Hello, World!"

.section "Standard Library", code
    .include "functions.inc"

.section "Realtime Interrupt", int6
    realtime_interrupt:
        stp [rRTCL], l0
        reti

.section "Main", code

    wait_vblank:
        ldp l0, [rLY]
        cmp l0, 144
        jpb cs, wait_vblank
        ret

    main:
        ; Shut down audio circutry.
        ld l0, 0
        stp [rNR52], l0

        ; Turn LCD off at next vblank.
        call wait_vblank
        xor l0, l0
        stp [rLCDC], l0

        ; Copy Tile Data
        ld d14, tiles
        ld d15, mVRAM + $1000
        ld w13, tiles_end - tiles
        call memcpy

        ; Copy Tile Map
        ld d14, tilemap
        ld d15, mVRAM + $1800
        ld w13, tilemap_end - tilemap
        call memcpy

        ; Initialize real-time clock interrupt.
        ld l0, INT_REALTIME
        stp [rIE0], l0
        ei

        ; Initialize display registers, then turn LCD back on.
        ld l0, 0b11100100
        stp [rBGP], l0

        ld l0, 0b10000001
        stp [rLCDC], l0

    main_done:
        jpb main_done
        