;
; @file     subroutines.asm
; @brief    Contains the subroutines for our program.
;

; Includes *********************************************************************

.include "hardware.inc"

; Subroutine Code **************************************************************

.section "Subroutines", code

    WaitForVerticalBlank::
        ldp l0, [rLY]
        cmp l0, LY_VBLANK
        jpb cs, WaitForVerticalBlank
        ret

    CopyBytes::                          ; `D13`: destination; `D14`: source; `W15`: count
        mv l0, h15
        or l0, l15
        ret zs
        xor l0, l0
    .loop:
        ld l0, [d14]
        sti [d13], l0
        inc d14
        dec w15
        mv l0, h15
        or l0, l15
        jpb zc, .loop
        ret

    SetBytes::                           ; `D13`: target; `L14`: value; `W15`: count
        mv l0, h15
        or l0, l15
        ret zs
    .loop:
        sti [d13], l14
        dec w15
        mv l0, h15
        or l0, l15
        jpb zc, .loop
        ret

    SerialHandler::
        push d0
        ldp l0, [rSB]
        st [wRecvByte], l0
        ld l0, [wRole]
        and l0, l0
        jpb zc, .host

        ; If client, then set the next SendByte to the byte we just received.
        ld l0, [wRecvByte]
        st [wSendByte], l0
        stp [rSB], l0

        ; Re-arm external clock.
        ld l0, $80
        stp [rSC], l0
        inc d11
        jpb .done

    .host:
        ; Check if we need to pulse the clock a second time
        ld l0, [wHostPhase]
        and l0, l0
        jpb zs, .done  ; If 0, we just received the reply. We are done!

        ; We are in Phase 1. The byte currently in wRecvByte is junk.
        ; We need to clock the connection again to fetch the Client's actual reply.
        
        ; Reset phase to 0 to prevent an infinite interrupt loop
        xor l0, l0
        st [wHostPhase], l0

        ; Reload our SendByte into SB. 
        ; (Sending the same data twice prevents the Client from receiving a 
        ; weird dummy byte like $FF and updating its UI incorrectly).
        ld l0, [wSendByte]
        stp [rSB], l0

        ; Re-arm the internal clock to start Transfer 2 (The Fetch)
        ld l0, $81
        stp [rSC], l0

    .done:
        pop d0
        reti

    ReadJoypad::
        ld l0, JOYP_GET_BUTTONS         ; Get Action Buttons
        stp [rJOYP], l0
        ldp l0, [rJOYP]
        not l0
        and l0, $0F
        mv l1, l0

        ld l0, JOYP_GET_DPAD            ; Get D-Pad
        stp [rJOYP], l0
        ldp l0, [rJOYP]
        not l0
        and l0, $0F
        swap l0
        or l0, l1

        mv l1, l0
        ld l0, [wJoypadState]
        not l0
        and l0, l1
        st [wJoypadPressed], l0
        mv l0, l1
        st [wJoypadState], l0

        ld l0, JOYP_GET_NONE
        stp [rJOYP], l0
        ret

    UpdateUI::
        call WaitForVerticalBlank

        ; Draw Role
        ld l0, [wRole]
        and l0, l0
        jpb zs, .drawClient
        ld l0, $11
        jpb .drawRole

    .drawClient:
        ld l0, $12

    .drawRole:
        st [mVRAM + $1842], l0

        ; Draw Sent Byte
        ld l0, [wSendByte]
        call ByteToHex
        st [mVRAM + $1882], l0
        mv l0, l2
        st [mVRAM + $1883], l0

        ; Draw Recv Byte
        ld l0, [wRecvByte]
        call ByteToHex
        st [mVRAM + $18C2], l0
        mv l0, l2
        st [mVRAM + $18C3], l0

        ret

    ByteToHex::
        mv l1, l0
        swap l0
        and l0, $0F
        mv l3, l0
        mv l0, l1
        and l0, $0F
        mv l2, l0
        mv l0, l3
        ret
