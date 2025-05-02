#include "rpi-asm.h"

@ linker hack so we can force this file in the first portion of 
@ the linked binary.
.section ".text.boot"

@ this is where we start.  if you look in any '.list' file, you 
@ better see code at the first addresses (in our case: starting
@ at 0x8000).
.globl _start
_start:

    @ force the mode to be SUPER.
    mov r0,  #SUPER_MODE
    orr r0,r0,#(1<<7)    @ disable interrupts.
    msr cpsr, r0
    prefetch_flush(r1)

    mov sp, #STACK_ADDR @ initialize stack pointer
    mov fp, #0          @ clear frame pointer reg.  don't think needed.
    @ bl notmain        @ we could jump right to notmain
    bl _cstart          @ call our code to do initialization.
    bl rpi_reboot       @ if they return just reboot.

@ keep put32/get32 here so that they are in the same location in every binary.
@ if you want to override them easily, can re-define the name in rpi-asm.h
@
@ we write them as assembly routines to make absolutely sure the compiler
@ cannot optimize them (reorder, eliminate, cache, etc.)

@
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

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ the routines below should give the same hash for everyone.
@   1. this file is linked first so will be at the same location
@      in every binary.
@   2. everyone's assembler should convert this assembly code to 
@      the same machine code  (not true for C code).
@   3. therefore: the same code will be at the same location for everyone.
@   4. therefore: the single step hashes will be the same:
@           - same instructions;
@           - at same location;
@           - run on same initial state;
@      = same result.
@
@  this determinism let's us cross check our results across
@  everyone(!) and across time(!).

@ first trivial test:
@  - one nop
@  - load integer for EXIT into r0
@  - do system call.
@ do this first to see that the code works.
MK_FN(nop_1)
    nop
    bx lr

MK_FN(nop_10)
    nop @ 1
    nop @ 2
    nop @ 3
    nop @ 4
    nop @ 5
    nop @ 6
    nop @ 7
    nop @ 8
    nop @ 9
    nop @ 10
    bx lr   
    asm_not_reached()

@ this sets each register to its decimal
@ name so can check result.
MK_FN(terminal_reg_test)
    mov r0, #0
    mov r1, #1
    mov r2, #2
    mov r3, #3
    mov r4, #4
    mov r5, #5
    mov r6, #6
    mov r7, #7
    mov r8, #8
    mov r9, #9
    mov r10, #10
    mov r11, #11
    mov r12, #12
    mov r13, #13
    mov r14, #14
    @ bad form: r0 = 0, which = SYS_DIE 
    swi 1
@ swi fault pc should be equal to this location
.globl terminal_reg_test_swi
terminal_reg_test_swi:
    asm_not_reached()
