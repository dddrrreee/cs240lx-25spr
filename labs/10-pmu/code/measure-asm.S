@ use this file to put your assembly code: gives more 
@ precise control.
#include "rpi-asm.h"

#define ALIGN .align 4

ALIGN
.globl nop_1
nop_1:
    nop  @ 1
    nop  @ 2
    nop  @ 3
    nop  @ 4
    nop  @ 5

    nop  @ 1
    nop  @ 2
    nop  @ 3
    nop  @ 4
    nop  @ 5

    bx lr

ALIGN
.globl nop_2
nop_2:
    nop  @ 1
    nop  @ 2
    nop  @ 3
    nop  @ 4
    nop  @ 5

    nop  @ 1
    nop  @ 2
    nop  @ 3
    nop  @ 4
    nop  @ 5

    bx lr
