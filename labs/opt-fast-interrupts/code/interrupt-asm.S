#include "rpi-asm.h"

@ just use the default int stack address.
#define INT_STACK INT_STACK_ADDR

@ <default_vec_ints> jumps to this trampoline when we get 
@ an interrupt.    
@
@ the trampoline sets up the interrupt stack, saves state 
@ and calls the C routine <int_vector>.  when that returns 
@ it restores state and jumps back to interrupted instruction.
@
@ this is the only assembly you have to modify (initially).
interrupt:
        sub   lr, lr, #4               @ correct interrupt pc
        mov sp, #INT_STACK             @ load the stack pointer
        push  {r0-r12,lr}              @ push all regs (trim this to caller)
        mov   r0, lr                   @ pass exception pc as arg0
        bl    int_vector               @ call our interrupt handler.
        pop   {r0-r12,lr}              @ pop all the regs
        movs    pc, lr                 @ resume back at exception location.

.align 5; 
.globl default_vec_ints
default_vec_ints: 
    b reset
    b undef
    b syscall
    b prefetch_abort
    b data_abort
    b reset
    b interrupt

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ these are trampolines for unexpected exceptions: they just
@ die with an error.  you don't have to modify them.

#define MK_TRAMPOLINE(name, offset, fn)         \
    name:                                       \
        sub   lr, lr, # (offset);               \
        mov sp, #INT_STACK;               \
        push  {r0-r12,lr};                \
        mov   r0, lr;                     \
        bl    fn;                         \
        pop   {r0-r12,lr};                \
        movs    pc, lr


@ call the default handlers and blow up.  override if you
@ want to do something fancier.
MK_TRAMPOLINE(reset,            4, reset_vector)
MK_TRAMPOLINE(undef,            4, undefined_instruction_vector)
MK_TRAMPOLINE(prefetch_abort,   4, prefetch_abort_vector)
MK_TRAMPOLINE(data_abort,       8, data_abort_vector)
MK_TRAMPOLINE(syscall,          4, syscall_vector)

