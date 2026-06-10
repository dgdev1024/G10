.import counter_loop
.export main

.section "other stuff", data
    bug: .dword 42

.section "main program", code
    main:
        ld l0, $00
        call counter_loop
        stop
