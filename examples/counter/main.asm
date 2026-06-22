.import counter_loop
.export main

.charmap "Hello", 0x92
.charmap ", ", 0x03
.charmap "World!", 0x17

.section "other stuff", data
    bug: .dword 42
    test: .string "Hello, World!"

.section "main program", code
    main:
        ld l0, $00
        call counter_loop
        stop
