#include "rpi.h"
#include "memtrace.h"

#include "watchpoint.h"

#include "mmu.h"
// 140e exception handling support
#include "full-except.h"
// 140e helpers for getting exception reason.
#include "armv6-except.h"
// 140e code for full context switching
// (caller,callee and cpsr).
#include "switchto.h"

#include "sbrk-trap.h"

// 1 = we expect a domain fault.
// 0 = we expect a or a watchpoint fault.
// used to catch some mistakes.
static int expect_domain_fault_p = 1;

// right now we only allow a single checker.  wrap this
// up for multiple checkers.
static memtrace_fn_t pre;
static memtrace_fn_t post;
static void *data;

static int quiet_p = 0;
void memtrace_yap_off(void) { quiet_p = 1; }
void memtrace_yap_on(void)  { quiet_p = 0; }

// pre-computed domain register values.
static uint32_t trap_access = 0;
static uint32_t no_trap_access = 0;

static int trap_is_on_p(void) {
    todo("return 1 if trapping on: use domain_access_ctrl_get");
}
static void trap_on(void) {
    todo("turn trapping on: use domain_access_ctrl_set");
}
static void trap_off(void) {
    todo("turn trapping off: use domain_access_ctrl_set");
}

// turn memtracing on: wrapper with extra error checking.
void memtrace_trap_enable(void) {
    // need at least one handler!
    assert(pre || post);
    // if not true, didn't init
    assert(trap_access && no_trap_access);
    assert(!trap_is_on_p());
    trap_on();
}

// turn memtracing off: wrapper with extra error checking.
void memtrace_trap_disable(void) {
    // if not true, didn't init
    assert(trap_access && no_trap_access);
    assert(trap_is_on_p());
    trap_off();
}

// XXX: a good extension: change this so you look at the
// actual instruction and get the actual bytes.
static inline unsigned inst_nbytes(uint32_t inst) {
    return 4;
}

static void data_fault(regs_t *r) {
    // sanity check that we still at SUPER
    //   - should make it so we can run at user level.
    if(mode_get(r->regs[16]) != SUPER_MODE)
        panic("got a fault not at SUPER level?\n");

    // after a domain fault: call <pre>.  
    // after a watchpoint fault: call <post>.
    todo("handle the fault!");

    // drain printk to avoid the "can tx" race in UART.
    while(!uart_can_put8())
        ;

    switchto(r);
}

// initialize memtrace system.
void memtrace_init(
    void *data_h,
    memtrace_fn_t pre_h,
    memtrace_fn_t post_h,
    unsigned trap_dom) {

    // setting up VM does not belong here, but we do it to keep things
    // simple for today's lab.
    assert(!mmu_is_enabled());
    sbrk_init();
    assert(mmu_is_enabled());

    pre = pre_h;
    post = post_h;
    if(!pre && !post)
        panic("must supply one handler: pre=%x, post=%x\n", pre,post);
    data = data_h;
    assert(trap_dom < 16);

    todo("do any additional setup you need");

    // XXX: what's the right way to handle SS exceptions at the same time?
    full_except_install(0);
    full_except_set_data_abort(data_fault);
}
