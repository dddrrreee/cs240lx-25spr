@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ all the assembly code we need for pixie.
@
@ note: assumes we only switch from SUPER to USER and back
@ (not a big deal to change.)

#include "rpi-constants.h"
#include "rpi-asm.h"

@ switch to unprivileged USER mode from from
@ a privileged mode.
@
@ needed b/c single-stepping only works at user
@ level but we run kernel code a privileged
@ SUPER mode.
@
@ the complication is that USER mode has its
@ own private shadow copies of <lr> and <sp> 
@ registers so we have to migrate them from
@ SUPER to USER using non-shadowed registers. 
MK_FN(pixie_switchto_user_asm)
    @ move current mode's {sp,lr} into 
    @ non-shadowed caller-saved registers
    @ {r1,r2}.  (we use caller so we don't 
    @ have to save them.)
    mov r1, sp
    mov r2, lr

    @ get original cpsr so can return it.
    @ r3 is caller-saved.
    mrs r3, cpsr 

    @ switch to user mode
    cps #USER_MODE
    @ required before mode switch guaranteed
    @ see <rpi-asm.h>
    prefetch_flush(r0)   

    @ NOTE: at this point we will start getting SS exceptions.

    @ move into USER's <sp>, <lr>
    mov sp, r1
    mov lr, r2

    bx lr

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ the next three routines are related.
@
@ single stepping requires changing from privileged mode to 
@ user level.
@ 
@ - to get back to privileged we need to invoke a system
@   call.  we invoke a system call using <syscall_invoke_asm>.
@   this code isn't tricky.
@ 
@ - in the system call we return back to the original 
@   privileged mode by: using <pixie_resume_asm> to load 
@   all registers saved by: <syscall_trampoline>.  this
@   code is somewhat tricky.
@
@   thus, the offsets <syscall_trampoline> saves registers to
@   must be consistent with the offsets <pixie_resume_asm>
@   loads them from.

@ user-level trampoline to call system call.
@ we've written this same code many times in 
@ 140e, nothing special.
MK_FN(pixie_invoke_syscall)
    swi 0
    bx lr

@ system call trampoline.  currently have 
@ a single system call, so specialize stuff
@ for simplicity.
syscall_trampoline:
    @ we push registers r0-r15 in ascending
    @ order on the stack and pass a pointer to
    @ the base of this array to <pixie_syscall>.
    @ this lets the system call switch 
    @ back to privileged modes and resume
    @ easily with trivial indexes.

    @ assumes:
    @   (1) we have a valid system <sp>
    @   (2) that we will explicitly set <sp>
    @       later using <resume>
    
    @ we do this in an incredibly dumb way
    sub sp, sp, #4
    stm sp, {lr}         @ user-level pc (r15), 
                         @ (note: is 4 bytes past swi)
    sub sp, sp, #4
    stm sp, {r14}^       @ user-level lr (r14) 
                         @ note the caret "^"!
    sub sp, sp, #4
    stm sp, {r13}^       @ user-level sp (r13)
                         @ note the caret "^"!

    @ registers r0-r12 are the same at user/priv
    @ (i.e., not shadowed).  <push> will push them 
    @ in descending order on the stack 
    @ (r12 at the highest offset, then r11, ... r0
    @ at the lowest)
    push {r0-r12}
    
    @ put the base of the register list into 
    @ second argument register
    mov  r1, sp

    @ if we were going to allow the system
    @ call to return and jump back: for 
    @ simplicity redundantly push {lr} so 
    @ can pop it off easily.
    @   push {lr}
    @   bl pixie_syscall
    @   push {lr}
    @   movs    pc, lr 

    @ since we never resume, don't need to 
    @ save anything.  we use a bl to catch
    @ if the C code returns.
    bl pixie_syscall
    asm_not_reached();

@ called from <pixie_syscall>: 
@   1. switch back to mode specified in <cpsr>
@   2. load all registers.
@ 
@ note: we could use the 140e approach of using switchto.
@ this is conceptually simpler.  but we expose everything
@ in this lab as a review, and for people that didn't take
@ it.  if you want: you can rewrite to use as an extension.
MK_FN(pixie_switchto_super_asm)
    @ change to super mode
    cps #SUPER_MODE
    prefetch_flush(r1)

    @ load all the registers including
    @ pc (will jump to it)

    ldm  r0, {r0-r15}
    asm_not_reached();


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ hygenic exception handling below.
@
@ we don't expect other exceptions besides prefetch/syscall.
@ so if they occur: just load the stack pointer, the right 
@ exception pc, and call the default panic handler.

@ automatically generate exception trampoline:
@
@  - call it <name>, and keep scope private.
@  - correct the exception pc (held in lr) by <offset>
@  - save more registers than needed 
@  - pass the pc as the first argument to <fn> 
@    and call it.
@
// keep the name of the trampoline private.
#define MK_TRAMPOLINE(name, offset, fn)     \
    name:                                   \
        sub   lr, lr, #(offset);            \
        mov sp, #INT_STACK_ADDR;            \
        push  {r0-r12,lr};                  \
        mov   r0, lr;                       \
        bl    fn;                           \
        pop   {r0-r12,lr};                  \
        movs    pc, lr 

@ call the default handlers and blow up.  override if you
@ want to do something fancier.
MK_TRAMPOLINE(reset,            4, reset_vector)
MK_TRAMPOLINE(undef,            4, undefined_instruction_vector)
MK_TRAMPOLINE(data_abort,       8, data_abort_vector)
MK_TRAMPOLINE(interrupt,        4, int_vector)

@ note: you'll want to customize this trampoline yourself 
@ so you can have clean copies of the PMU counters.
MK_TRAMPOLINE(prefetch_abort,   4, prefetch_abort_vector)

@ we only expect <prefetch_abort> and <syscall> to get invoked.
@ everything else will call a panic handler that dies (see
@ the default handlers in <libpi/staff-src/>

.align 5;   @ we use vector base, so need 32-byte alignment.
.globl pixie_exception_vec
pixie_exception_vec:
    b reset
    b undef
    b syscall_trampoline
    b prefetch_abort
    b data_abort
    b reset
    b interrupt
    asm_not_reached()   @ we don't expect fast interrupt.

    @ the swi instruction is <lr>-4
    @ we don't use it, so ignore.
    @   sub   lr, lr, #4

    @ for simplicity: we usually set the exception
    @ stack pointer.  here (1) we have a valid <sp>
    @ (2) we explicitly set the <sp> when we resume
    @ using <pixie_resume_asm> (below).
