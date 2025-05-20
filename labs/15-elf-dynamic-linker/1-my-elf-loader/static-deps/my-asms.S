#include "rpi-asm.h"

@ Stuart: Very minimal version of assembly functions without dependencies
@ The goal is to have as little as possible, and dynamically link the rest.

.globl put32
.globl PUT32
put32:
PUT32:
    str r1,[r0]  
    bx lr      

.globl get32
.globl GET32
get32:
GET32:
    ldr r0,[r0]
    bx lr      

.globl put8
.globl PUT8
put8:
PUT8:
    strb r1,[r0]
    bx lr

.globl BRANCHTO
BRANCHTO:
    bx r0

.globl dev_barrier
dev_barrier:
    mcr p15, 0, r0, c7, c10, 4
    bx lr
