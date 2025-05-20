#include "rpi-asm.h"

@ linker hack so we can force this file in the first portion of the linked binary.
.section ".text.boot"

@ main entry point for our programs
.globl _start
_start:
    b skip
.space 0x10000-0x8004,0 @ to not overlap with the programs we load
                        @ Note this is very specific to the test programs being upper bounded in size
skip:
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
