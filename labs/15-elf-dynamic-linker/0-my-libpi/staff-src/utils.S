#include "rpi-asm.h"

@ Stuart: only the util functions from staff-start.S
@         as _start is not needed for libpi.so

@ put32(addr,val).  c-code equivalant:
@   - put32(addr, val) { *(uint32_t*)addr = val; }
@
@ store value <val> (passed in second argument register r1) 
@ into memory address <addr> (passed in first argument 
@ register, r0).
@
.globl put32
.globl PUT32
put32:
PUT32:
    str r1,[r0]     @ store r1 into address held in r0
    bx lr           @ return

@ get32(addr)
@
@ c-code equivalent: 
@   - get32(addr) { return *(uint32_t*)addr; }
@
@ load value of <addr> passed in first argument register (r0).
@
.globl get32
.globl GET32
get32:
GET32:
    ldr r0,[r0]     @ load address held in r0 into r0
    bx lr           @ return


@ jump to the address in r0.  we don't mess w/ link register
@ so called routine will return to our caller.
.globl BRANCHTO
BRANCHTO:
    bx r0
