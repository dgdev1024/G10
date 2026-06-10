.once

; Copy bytes from one area to another.
; @param    d14     Source
; @param    d15     Destination
; @param    w13     Length
memcpy:
    ld l0, [d14]
    sti [d15], l0
    inc d14
    dec w13
    mv l0, h13
    or l0, l13
    jpb zc, memcpy
    ret
    