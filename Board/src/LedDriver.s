    .def LED_Wait0Low, LED_Wait1Low, LED_Wait0High, LED_Wait1High

    .thumb
    .align 2
    .text

; Wait 0.35us (350ns)
LED_Wait0High:
    .asmfunc

    nop
    nop

    bx LR ; return to caller

    .endasmfunc

; Wait 0.9us (900ns)
LED_Wait0Low:
    .asmfunc

    nop
    nop
    nop
    nop
    nop

    bx LR ; return to caller

    .endasmfunc


    
; Wait 0.9us (900ns)
LED_Wait1High:
    .asmfunc

    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop


    bx LR ; return to caller

    .endasmfunc


; Wait 0.35us (350ns)
LED_Wait1Low:
    .asmfunc

    bx LR ; return to caller

    .endasmfunc
