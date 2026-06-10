.include "constants.inc"
.import counter
.export counter_loop

.section "print msg", rodata
    msg: .asciz "Hello, World!"

.section "print logic", code
    print_msg:
        mv l2, l0
        ld d1, msg

    print_msg_loop:
        ld l0, [d1]
        cmp l0, '\0'
        jpb zs, print_msg_done
        stp [0x42], l0
        inc d1
        jpb print_msg_loop

    print_msg_done:
        ld l0, '\n'
        stp [0x42], l0
        mv l0, l2
        ret

.section "loop logic", code
    counter_loop:
        ld l0, [counter]
        inc l0
        st [counter], l0
        cmp l0, print
        call zs, print_msg
        cmp l0, max
        jpb zc, counter_loop
        ret
        