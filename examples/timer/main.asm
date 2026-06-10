.global main
.const seconds = 10

.macro put_string
    .byte @1, '\n', 0
.endm

.section "Tick Message", data[$12000]
    tick_message:           put_string "Tick!"

.section "Tick Interrupt", int1
    tick_int:               call print_tick_msg
                            inc l0
                            cmp l0, seconds
                            jpb zs, tick_int_done
                            reti
    tick_int_done:          stop

.section "Print Tick", code[$EADBEEF]
    print_tick_msg:         mv l2, l0
                            ld d1, tick_message
                            jpb print_tick_msg.loop
                            nop
    print_tick_msg.loop:    ld l0, [d1]
                            cmp l0, '\0'
                            jpb zs, print_tick_msg.done
                            stp [0x42], l0
                            inc d1
                            jpb print_tick_msg.loop
    print_tick_msg.done:    mv l0, l2
                            ret

.section "Main Program", code[$A951542]
    main:                   ld l0, (1 << 1)
                            stp [0x00], l0
                            xor l0, l0
                            ei
    main_loop:              jpb main_loop
