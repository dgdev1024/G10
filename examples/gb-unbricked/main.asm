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

; Constants ********************************************************************

.const kBrickLeft = $05
.const kBrickRight = $06
.const kBlankTile = $08
.const kDigitOffset = $1A
.const kScoreTens = mVRAM + $1870
.const kScoreOnes = mVRAM + $1871

; Metadata *********************************************************************

.section "Program Metadata", metadata
    .org $0000 .asciz "G10.BOY"
    .org $0008 .dword 0x2000
    .org $0010 .asciz "G10.BOY Breakout"
    
; RAM Variables ****************************************************************

.section "RAM Variables", bss[mWRAM]
    wCurrentKeys:   .byte 1
    wNewKeys:       .byte 1
    wBallVelocityX: .byte 1
    wBallVelocityY: .byte 1
    wScore:         .byte 1

; Entry Point ******************************************************************

.section "Entry Point", code
clear_wram:
    xor l0, l0
    st [wCurrentKeys], l0
    st [wNewKeys], l0
    st [wBallVelocityX], l0
    st [wBallVelocityY], l0
    st [wScore], l0

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

;
; Converts a pixel position into an address in the VRAM's tilemap region.
;
; @param    h1      X Position
; @param    l1      Y Position
; @return   d3      Tile Address
;
get_tile_by_pixel:

    ; Prep `d3`. We'll need this later.
    ld d3, 0

    ; First, we need to divide by 8 to convert a pixel position to a tile position.
    ; After this we want to multiply the Y position by 32.
    ; These operations effectively cancel out so we only need to mask the Y value.

    ld l0, l1               ; Move Y position into byte accumulator.
    and l0, %11111000       ; Mask out the lower 3 bits.
    mv l3, l0               ; Move result into low byte of relative tile address.
    ld l2, 0                ; We need to prepare the high byte of the relative tile address, in `h3`.
    mv h3, l2               ; Do this by setting `l2` to zero, then moving it into `h3`.

    ; Now we have the (position * 8) stored in `w3` (aggregate of `h3` and `l3`).
    ; We need the (position * 32), though.

    mv w0, w3               ; Move the position value in `w3` into the word accumulator.
    add w0, w0              ; Achieve multiplication by adding `w0` to itself twice.
    add w0, w0
    mv w3, w0               ; Move result back into `w3`.

    ; Next, convert the X position into an offset.

    mv l0, h1               ; Move the X position in `l1` into the byte accumulator.
    srl l0                  ; Achieve (position / 8) by shifting `l0` right three places logically.
    srl l0
    srl l0

    ; Add the two offsets together.
    add l0, l3
    mv l3, l0
    mv l2, h3
    adc l0, l2
    sub l0, l3
    mv h3, l0

    ; Add our offset to the tilemap's base address.
    ld d0, mVRAM + $1800
    add d0, d3
    mv d3, d0

    ret

is_wall_tile:
    cmp l0, $00
    ret zs
    cmp l0, $01
    ret zs
    cmp l0, $02
    ret zs
    cmp l0, $04
    ret zs
    cmp l0, $05
    ret zs
    cmp l0, $06
    ret zs
    cmp l0, $07
    ret

check_brick:
    ld l1, kBlankTile
    ld l0, [d3]
    cmp l0, kBrickLeft
    jpb zc, check_brick.right
    sti [d3], l1
    st [d3], l1
    call score_point
check_brick.right:
    cmp l0, kBrickRight
    ret zc
    std [d3], l1
    st [d3], l1
    call score_point
    ret

score_point:
    ld l0, [wScore]
    add l0, 1
    daa
    st [wScore], l0
    call update_score
    ret

update_score:
    ld l0, [wScore]
    and l0, %11110000
    swap l0
    add l0, kDigitOffset
    st [kScoreTens], l0

    ld l0, [wScore]
    and l0, %00001111
    add l0, kDigitOffset
    st [kScoreOnes], l0
    ret

title_screen:
    call wait_vblank
    call lcd_off
    memcpy mVRAM + $1000, title_screen_tiles, title_screen_tiles_end - title_screen_tiles
    memcpy mVRAM + $1800, title_screen_tilemap, title_screen_tilemap_end - title_screen_tilemap
    ld l0, %11100100
    stp [rBGP], l0
    ld l0, LCDC_ON | LCDC_BG_ON
    stp [rLCDC], l0
title_screen.loop:
    call update_keys
    ld l0, [wCurrentKeys]
    and l0, JOYP_PAD_START
    jpb zs, title_screen.loop
    call wait_out_vblank
    ret

main:
    ld l0, 0
    stp [rNR52], l0
    call title_screen
    call wait_vblank
    call lcd_off
    memcpy mVRAM + $1000, game_tiles, game_tiles_end - game_tiles
    memcpy mVRAM + $1800, game_tilemap, game_tilemap_end - game_tilemap
    memcpy mVRAM, paddle_left, paddle_left_end - paddle_left
    memcpy mVRAM + $10, paddle_right, paddle_right_end - paddle_right
    memcpy mVRAM + $20, ball, ball_end - ball
    call clear_wram
    call clear_oam
    object 0, 16, 128, 0, 0
    object 1, 24, 128, 1, 0
    object 2, 16, 100, 2, 0
    ld l0, 1
    st [wBallVelocityX], l0
    ld l0, -1
    st [wBallVelocityY], l0
    ld l0, %11100100
    stp [rBGP], l0
    stp [rOBP0], l0
    ld l0, LCDC_ON | LCDC_BG_ON | LCDC_OBJ_ON
    stp [rLCDC], l0

main_loop:
    call wait_out_vblank
    call wait_vblank
    call update_keys

    ld l0, [wBallVelocityX]
    mv l1, l0
    ld l0, [mOAM + (2 * 4) + 1]
    add l0, l1
    st [mOAM + (2 * 4) + 1], l0

    ld l0, [wBallVelocityY]
    mv l1, l0
    ld l0, [mOAM + (2 * 4)]
    add l0, l1
    st [mOAM + (2 * 4)], l0

bounce_on_top:
    ld l0, [mOAM + (2 * 4)]
    sub l0, 16 + 1
    mv l1, l0
    ld l0, [mOAM + (2 * 4) + 1]
    sub l0, 8
    mv h1, l0
    call get_tile_by_pixel
    ld l0, [d3]
    call is_wall_tile
    jpb zc, bounce_on_right
    call check_brick
    ld l0, 1
    st [wBallVelocityY], l0

bounce_on_right:
    ld l0, [mOAM + (2 * 4)]
    sub l0, 16
    mv l1, l0
    ld l0, [mOAM + (2 * 4) + 1]
    sub l0, 1
    mv h1, l0
    call get_tile_by_pixel
    ld l0, [d3]
    call is_wall_tile
    jpb zc, bounce_on_left
    call check_brick
    ld l0, -1
    st [wBallVelocityX], l0

bounce_on_left:
    ld l0, [mOAM + (2 * 4)]
    sub l0, 16
    mv l1, l0
    ld l0, [mOAM + (2 * 4) + 1]
    sub l0, 8 + 1
    mv h1, l0
    call get_tile_by_pixel
    ld l0, [d3]
    call is_wall_tile
    jpb zc, bounce_on_bottom
    call check_brick
    ld l0, 1
    st [wBallVelocityX], l0

bounce_on_bottom:
    ld l0, [mOAM + (2 * 4)]
    sub l0, 8 - 1
    mv l1, l0
    ld l0, [mOAM + (2 * 4) + 1]
    sub l0, 8
    mv h1, l0
    call get_tile_by_pixel
    ld l0, [d3]
    call is_wall_tile
    jpb zc, bounce_on_paddle
    call check_brick
    ld l0, -1
    st [wBallVelocityY], l0

bounce_on_paddle:
    ld l0, [mOAM]                   ; Paddle Left Side's Y position.
    sub l0, 8
    mv l1, l0
    ld l0, [mOAM + (2 * 4)]         ; Ball's Y position.
    cmp l0, l1
    jpb zc, bounce_done

    ld l0, [mOAM + (2 * 4) + 1]     ; Ball's X position
    mv l1, l0
    ld l0, [mOAM + (0 * 4) + 1]     ; Paddle Left Side's X position
    sub l0, 8
    cmp l0, l1
    jpb cc, bounce_done
    ld l0, [mOAM + (1 * 4) + 1]     ; Paddle Right Side's X position
    sub l0, 8
    cmp l0, l1
    jpb cc, bounce_done
    add l0, 16
    cmp l0, l1
    jpb cs, bounce_done
    ld l0, -1
    st [wBallVelocityY], l0

bounce_done:

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
