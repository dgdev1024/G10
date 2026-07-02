
.newcharmap "test"
.charmap "&euro;", 0xAC, 0x20

.macro test
    .byte @#
.endm

.section "other stuff", data
    bug: .dword 42
    lbl: test "Dennis W. Griffin", 34, 0x42, "&euro;"

.section "main program", code
    main::
        ld l0, $00
        call counter_loop
        stop
