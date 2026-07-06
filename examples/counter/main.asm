
.newcharmap "test"
.charmap "&euro;", 0xAC20

.macro test
    .byte @#
.endm

.section "other stuff", data
    bug: .dword 42
    lbl: .byte "&euro;"

.section "main program", code
    main::
        ld l0, $00
        call counter_loop
        stop
