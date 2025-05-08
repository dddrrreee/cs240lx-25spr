// a simple single step tracing example that rewrites the 
// lab 9 single-step example code to:
//  1. use the 140e <switchto>
//  2. use the 140e full register exception code.
// since we will use these for memory tracing.
#include "rpi.h"
#include "cpsr-util.h"

// 140e watchpoint support.
#include "watchpoint.h"
// 140e exception handling support
#include "full-except.h"

// 140e code for full context switching
// (caller,callee and cpsr).
#include "switchto.h"

// used to turn off output (1=no output).
static volatile int n_inst = 0;

static uint32_t expected_fault_addr = 0;
static uint32_t expected_fault_pc = 0;

// called on each single-step exception. 
static void watchpt_handler(regs_t *r) {

    // make sure it was a breakpoint fault.
    if(!watchpt_fault_p())
        panic("have a non-watchpt fault?\n");

    uint32_t pc = r->regs[15];
    uint32_t r0 = r->regs[0];
    uint32_t r1 = r->regs[1];

    // is different from <pc>
    uint32_t fault_pc   = watchpt_fault_pc();
    uint32_t fault_addr = watchpt_fault_addr();

    if(fault_addr != expected_fault_addr)
        panic("expected a fault on %x, have %x\n", 
            expected_fault_addr, 
            fault_addr);

    if(fault_pc != expected_fault_pc)
        panic("expected a fault at pc=%x, have pc=%x\n", 
            expected_fault_pc,
            fault_pc);

    // turn off so can deref
    watchpt_off(fault_addr);

    n_inst++;
    output("%d watchpoint fault:\n", n_inst);
    output("    fault_addr=%x\n", fault_addr);
    output("    fault_pc=%x\n", fault_pc);
    output("    resume=%x\n", pc);
    if(watchpt_load_fault_p()) {
        output("    load: GET32(%x) = %x, r0=%x\n", 
                    fault_addr,
                    GET32(fault_addr), 
                    r0);
    } else {
        output("    store GET32(%x) = %x  r0=%x, r1=%x\n", 
                    fault_addr,
                    GET32(fault_addr), 
                    r0, r1);
    }

    // drain printk if neeed.
    while(!uart_can_put8())
        ;

    // resume by loading all registers.
    switchto(r);
}

void notmain(void) {
    // install is idempotent if already there.
    full_except_install(0);
    // for breakpoint handling (like lab 10)
    full_except_set_data_abort(watchpt_handler);

    for(int i = 0; i < 10; i++) {
        output("--------------------- iter=%d ----------------\n", i);
        uint32_t x = 0;
        uint32_t val = 0x11223344+i;


        output("going to watch addr=%x\n", &x);
        watchpt_on_ptr(&x);

        // we expect a fault on <&x> at pc=<put32>
        expected_fault_addr = (uint32_t)&x;
        expected_fault_pc = (uint32_t)put32;
        put32(&x, val);

        output("after put32: x= %x\n", get32(&x));
        assert(get32(&x) == val);
 
        uint32_t y = x;
        watchpt_on_ptr(&y);

        expected_fault_addr = (uint32_t)&y;
        expected_fault_pc = (uint32_t)get32;

        // we expect a fault on <&y> at pc=<get32>
        uint32_t v = get32(&y);
        output("GET32(%x)=%x\n", &y, v);
        assert(v == val);

        output("iter=%d passed!\n", i);
    }

    output("SUCCESS: all iterations passed!\n");
}
