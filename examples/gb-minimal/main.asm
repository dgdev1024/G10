.export main

.const rNR10 = $30
.const rNR11 = $31
.const rNR12 = $32
.const rNR13 = $33
.const rNR14 = $34

.section "G10.BOY Metadata", metadata
    .org $0000 .asciz "G10.BOY"
    .org $0008 .dword 0x2000
    .org $0010 .asciz "G10.BOY Minimal Program"
    
.section "Sound Data", data
    SoundHigh:  .byte $80, $E2, $50, $87

.section "Main", code
    main:       ld d1, SoundHigh
                ld h0, 0
                stp [rNR10], h0
                ldi h0, [d1]
                stp [rNR11], h0
                ldi h0, [d1]
                stp [rNR12], h0
                ldi h0, [d1]
                stp [rNR13], h0
                ldi h0, [d1]
                stp [rNR14], h0

    .done:      jpb .done
