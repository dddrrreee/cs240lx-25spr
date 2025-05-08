// a simple single step tracing example that rewrites the 
// lab 9 single-step example code to:
//  1. use the 140e <switchto>
//  2. use the 140e full register exception code.
// since we will use these for memory tracing.
#include "rpi.h"
#include "cpsr-util.h"

// 140e breakpoint support.
#include "breakpoint.h"
// 140e exception handling support
#include "full-except.h"
// 140e helpers for getting exception reason.
#include "armv6-except.h"
// 140e code for full context switching
// (caller,callee and cpsr).
#include "switchto.h"

// routines in single-step-start.S
void single_step_exit(void);
void terminal_reg_test(void);
void nop_10(void);

// registers used to resume when done single stepping.
static regs_t start_regs;
// used to compute which registers an instruction changed.
static regs_t last_regs;

// counter of the number of instructions.
static volatile unsigned n_inst = 0;

// used to turn off output.
static int quiet_p = 0;

// called on each single-step exception. 
static void single_step_handler(regs_t *r) {
    // make sure it was a breakpoint fault.
    if(!brkpt_fault_p())
        panic("have a non-breakpoint fault\n");

    // get the pc and cpsr from the saved regs.
    uint32_t pc = r->regs[15];
    uint32_t cpsr = r->regs[16];

    n_inst++;

    if(!quiet_p) {
        output("%d: single-step fault pc=%x, cpsr=%x:\n", n_inst, pc, cpsr);
        if(n_inst > 1) {
            output("\tchanged regs={ ");
            // compute which registers changed by comparing them
            // to the previous ones.
            for(int i = 0; i < 15; i++) {
                if(r->regs[i] != last_regs.regs[i])
                    output("r%d = %x ", i, r->regs[i]);
            }
            output("}\n");
        }
        last_regs = *r;
    }

    // set a mismatch on the fault pc so that we can run it.
    brkpt_mismatch_set(pc);

    // if you don't print: you don't have to do this.
    // if you do print, there is a race condition if we 
    // are stepping through the UART code --- note: we could
    // just check the pc and the address range of
    // uart.o
    while(!uart_can_put8())
        ;

    // resume.
    switchto(r);
}

// very limited system calls.  we only need these
// b/c SS has to run at USER level so can't switch
// back.
static int single_step_syscall(regs_t *r) {
    // check the saved spsr and make sure is USER level.
    if(mode_get(spsr_get()) != USER_MODE)
        panic("not at USER level?\n");

    // check that we always get called with <sysno>=0
    uint32_t sysno = r->regs[0];
    if(sysno != 0)
        panic("weird sysno=%d, expected 0\n", sysno);

    output("done running ss mode: switching back!\n");
    switchto(&start_regs);
}


// initializes the full register set so it can be
// run on its own.
static regs_t 
thread_mk(void *fn, uint32_t arg, uint32_t nbytes) {
    // compute USER cpsr using current cpsr.
    uint32_t cpsr = cpsr_inherit(USER_MODE, cpsr_get());

    // stack grows down, so stack pointer = 
    //  stack_base + nbytes.
    void *stack = 0;
    if(nbytes)
        stack = kmalloc_aligned(nbytes, 8);
    uint32_t sp = (uint32_t)stack + nbytes;

    // initialize the registers
    //  see <switchto.h>
    return (regs_t) {
        .regs[REGS_PC] = (uint32_t)fn,
        // the first argument to fn
        .regs[REGS_R0] = arg,

        // stack pointer register
        .regs[REGS_SP] = sp,
        // the cpsr to use
        .regs[REGS_CPSR] = cpsr,

        // where to jump to if the code returns.
        // see <ss-start.S>
        .regs[REGS_LR] = (uint32_t)single_step_exit,
    };
}

// we print using put8 so there aren't as many
// traced instructions.
static void hello_world(void) {
    uart_put8('h');
    uart_put8('e');
    uart_put8('l');
    uart_put8('l');
    uart_put8('o');
    uart_put8('\n');
}

// start single step mismatching.
//  1. sets up exception vectors (idempotent).
//  2. enables single stepping (idempotent)
//  3. switches to USER mode.
//
// NOTE:
//   if you want to ignore instructions until 
//   return back outside of <pixie_start>
//   1. get the return address before switching
//      to USER mode
//         ignore_until_pc = __builtin_return_address(0);
//   2. in the single-step handler: ignore all PC values 
//      until <pc> equals <ignore_until_pc>
void notmain(void) {

    // install is idempotent if already there.
    full_except_install(0);
    // for breakpoint handling (like lab 10)
    full_except_set_prefetch(single_step_handler);
    // for system calls (like many labs)
    full_except_set_syscall(single_step_syscall);

    kmalloc_init(1);

    // enable mismatching.  note: we are currently
    // at privileged mode so won't start til we 
    // switch to to user mode.
    //
    // this routine is from 140e.  will: 
    //  1. enable the debug co-processor cp14 
    //  2. set a mismatch on address <0>.  once we are 
    //    at user-level, this will cause any pc != 0 (all 
    //    of them) to immediately mismatch
    brkpt_mismatch_start();

    // switch from SUPER to USER mode: not just a matter of
    // switching the cpsr since the sp,lr are different.

    // 1. sanity check that we are at SUPER_MODE
    if(mode_get(cpsr_get()) != SUPER_MODE)
        panic("not at SUPER level?\n");

    // 2. make a single user level thread to call 
    // <ss-start.S:terminal_reg_test>
    regs_t r = thread_mk(terminal_reg_test, 1, 0);

    // 3. cswitch to it: will immediately immediately 
    // start mismatching when the mode switch occurs.
    debug("about to switch to <terminal-reg_test>\n");
    switchto_cswitch(&start_regs, &r);
    output("DONE!\n");

    // 4. run simple nop_10: 
    quiet_p = 1;
    debug("about to switch to <nop_10>\n");
    n_inst = 0;
    r = thread_mk(nop_10, 0, 0);
    switchto_cswitch(&start_regs, &r);

    // 5. run simple hello world
    debug("about to switch to run a hello world\n");
    n_inst = 0;
    quiet_p = 1;
    r = thread_mk((void*)hello_world, 0, 1024*8);
    switchto_cswitch(&start_regs, &r);

    output("SUCCESS!\n");
}
