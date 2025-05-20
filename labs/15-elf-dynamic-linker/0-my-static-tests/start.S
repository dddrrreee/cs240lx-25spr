#include "rpi-asm.h"

@ linker hack so we can force this file in the first portion of the linked binary.
.section ".text.boot"

@ main entry point for our programs
.globl _start
_start:
    mov r0,  #SUPER_MODE  @ force the mode to be SUPER.
    orr r0,r0,#(1<<7)     @ disable interrupts.
    msr cpsr, r0
    prefetch_flush(r1)

    mov sp, #STACK_ADDR   @ initialize stack pointer
    mov fp, #0            @ clear frame pointer reg.  don't think needed.
    bl _cstart            @ call our code to do initialization.
    bl rpi_reboot         @ if they return just reboot.
