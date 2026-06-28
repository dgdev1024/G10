
.charmap "&euro;", 0xAC, 0x20

.section "other stuff", data
    bug: .dword 42
    test: .string "&euro;"

.section "main program", code
    main::
        ld l0, $00
        call counter_loop
        stop
