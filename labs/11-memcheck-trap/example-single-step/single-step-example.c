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
// used to make it clear what is going on when single
// stepping.
void single_step_exit(void);
void terminal_reg_test(void);
void nop_10(void);

// registers used to resume when done single stepping.
static regs_t start_regs;

// counter of the number of instructions.
static volatile unsigned n_inst = 0;

// used to turn off output (1=no output).
static int quiet_p = 0;

// see the comment in <single_step_handler>: this code
// should never run: the handler checks if the faulting 
// PC equals <single_step_done> and, if so, stops single
// step execution by switching back to the original 
// privleged code.
void single_step_done(void) {
    panic("this code should not run!\n");
}

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
        // used to compute which registers an instruction changed.
        static regs_t last_regs;

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
        // NOTE: effectively initializes <last_regs> when
        // <n_inst>=1.
        last_regs = *r;
    }

    // a different way to stop single step.
    //
    // recall from lab 9 that single-stepped code cannot
    // turn off single-stepping since it's running unprivileged
    // at USER level.
    //
    // in lab 9 we used a system call to enter privileged
    // mode and turn it off.    here we don't do that.
    // instead, to illustrate other approaches, we simply 
    // check the <pc> value and if its a special routine 
    // (<single_step_done>) stop single step execution by
    // switching back to the original privileged code,
    // saved in <start_regs>.
    //
    // a third way --- mildly reminiscent of how trap-and-emulate
    // virtual machine monitors work --- would be to:
    //   1. have the / unprivileged code simply attempt to execute 
    //      the privileged single-step disable instruction. 
    //   2. this will result in an illegal instruction fault.  
    //   3. in the illegal instruction handler, validate that 
    //      this is what they were doing (decode the instruction
    //      at the faulting PC) and  then switch back to 
    //      <start_regs>.
    if(pc == (uint32_t)single_step_done) {
        output("handler: done running ss mode: switching back!\n");
        switchto(&start_regs);
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

    // resume by loading all registers.
    switchto(r);
}

// we *could* use lab 9's approach to disable single
// stepping by using system calls. instead we 
// use a special routine (see above).  
//
// with that said, we put this lab 9 syscall code here 
// so you can see the alternative approach. you can 
// remove the <panic> and try using system calls 
// as a way to get more understanding.
//
// very limited system calls.  we only need these
// b/c SS has to run at USER level so can't switch
// back.
static int single_step_syscall(regs_t *r) {
    panic("currently should not happen\n");
    
    // check the saved spsr and make sure is USER level.
    if(mode_get(spsr_get()) != USER_MODE)
        panic("not at USER level?\n");

    // check that we always get called with <sysno>=0
    uint32_t sysno = r->regs[0];
    if(sysno != 0)
        panic("weird sysno=%d, expected 0\n", sysno);

    debug("done running ss mode: switching back!\n");
    switchto(&start_regs);
}

// check if they did an illegal instruction.
static void single_step_undef(regs_t *r) {
    // hard-code the exact instruction that disables
    // single step: you can use your derive code to 
    // get all permutations! (r0, r1, ...)
    //
    // 90b0:   ee003eb0    mcr 14, 0, r3, cr0, cr0, {5}
    uint32_t pc = r->regs[15];
    uint32_t inst = GET32(pc);

    // can we catch cps?
    output("pc=%x: got an illegal instruction %x\n", pc, inst);

    if(inst != 0xee003eb0)
        panic("%x: unexpected illegal instruction!\n");

    output("illegal: done running ss mode, switching back!\n");
    switchto(&start_regs);
}

// initializes the full register set so it can be
// run on its own.
static regs_t 
thread_mk(void *fn, uint32_t arg, uint32_t stack_p) {
    // compute USER cpsr using current cpsr.
    uint32_t cpsr = cpsr_inherit(USER_MODE, cpsr_get());

    // statically allocate stack.
    static  __attribute__ ((aligned(8))) 
        uint64_t stack[1024*4];
    uint32_t sp = 0;
    // stack grows down, so stack pointer = 
    //  stack_base + nbytes.
    if(stack_p)
        sp = (uint32_t)stack + sizeof stack;

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
        // see <single-step-start.S>.  note: if you
        // want to change how to stop single step
        // execution (syscall, special pc, illegal
        // instructions) you where the code jumps.
        .regs[REGS_LR] = (uint32_t)single_step_done,
    };
}

// wrapper for running <fn(arg)> at user level in single
// step mode.
static void 
ss_run_fn(const char *msg,   
    void (*fn)(void), 
    uint32_t arg, 
    unsigned nbytes) 
{
    output("--------------------------------------------------\n");
    output("about to switch to <%s>\n", msg);

    n_inst = 0;

    // make a single user level thread to call <fn(arg)>
    regs_t r = thread_mk(fn, arg, nbytes);

    // cswitch to <r>, saving current state in <start_regs>
    // for later resumption.
    // 
    // will immediately immediately start mismatching when the 
    // mode switch to USER occurs.
    switchto_cswitch(&start_regs, &r);

    // back!
    //
    // sanity check that we are back at SUPER
    if(mode_get(cpsr_get()) != SUPER_MODE)
        panic("not at SUPER level?\n");
    output("done!\n");
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

static void hello_illegal(void) {
    hello_world();

    // turn off single step by using illegal
    // instruction.
    // 
    // NOTE: would be better to trap in the context
    // switch but unfortunately the <msr> instruction
    // that sets privilege mode does not trap!
    brkpt_mismatch_stop();
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
    // sanity check that we are at SUPER_MODE
    if(mode_get(cpsr_get()) != SUPER_MODE)
        panic("not at SUPER level?\n");

    // install is idempotent if already there.
    full_except_install(0);
    // for breakpoint handling (like lab 10)
    full_except_set_prefetch(single_step_handler);
    // for system calls (like many labs).  note:
    // current code doesn't use this.  we leave it
    // in case you want to try other way.
    full_except_set_syscall(single_step_syscall);

    // set undefined instruction handler in case
    // you want to try that way of handling fault.
    full_except_set_undef(single_step_undef);

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

    /**************************************************************
     * now that everything is setup: do multiple tests to illustrate 
     * (1) that SS is working and (2) different ways to turn it off.
     *
     * NOTE: would be a bit cleaner to have different test cases, 
     * but putting all the code here means you don't have to check 
     * different files (much).
     */

    // two simple stackless assembly routines (see 
    // <single-step-start.S>
    ss_run_fn("terminal_reg_test", terminal_reg_test, 1, 0);
    ss_run_fn("nop_10", nop_10, 0, 0);

    // stop printing b/c going to do a lot more instructions.
    quiet_p = 1;

    // more advanced: hello world 
    ss_run_fn("hello_world", hello_world, 0, 1);

    // same, but turn things off using an illegal instrution.
    ss_run_fn("hello_illegal", hello_illegal, 0, 1);

    output("SUCCESS!\n");
}
