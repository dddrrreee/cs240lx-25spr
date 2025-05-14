// demonstrate trivial memory tracing using the raw interface.
// 1. allocate memory.
// 2. do repeated load-stores to it.
// 3. print result.
#include "rpi.h"
#include "memtrace.h"
#include "memmap-default.h"

// need to specify the size: 1,2,4,8,... --- gcc is allowed to merge
// adjacent object writes, but i don't think will ever be able to see.
//
// probably better to pass in the fault.  but then you have to eat the
// load misses.

static int trace_handler(void *data, fault_ctx_t *f) {
    unsigned *n = data;
    *n += 1;

    output("\t%d: memtrace: pc=%x, addr=%x, nbytes=%d, load=%d\n",
                    *n, 
                    f->pc,
                    f->addr, 
                    f->nbytes, 
                    f->load_p);

    return MEMTRACE_OK;
}

void foo(void) {
    uint32_t x = 0x01010101;

    // note: the call to <kmalloc> will generate a fault since it 
    // zero initializes the allocated memory.
    //
    // we use volatile so we can predict what the loads will be.
    volatile uint32_t *u = kmalloc(sizeof *u);

    // this address will be the same for all programs since we start
    // the heap at the same place.
    trace("about to write to %x\n", u);
    *u += x;  // 1
    *u += x;  // 2
    *u += x;  // 3
    *u += x;  // 4
    // we do this so the fault doesn't break up the printk :)
    x = *u;
    trace("done writing to %x: value=%x\n", u, x);
}

void notmain(void) {
    unsigned n_faults = 0;

    memtrace_init(&n_faults, trace_handler, 0, dom_trap);
    memtrace_yap_off();

    trace("about to turn on tracing\n");
    memtrace_trap_enable();
        foo();
    memtrace_trap_disable();
    trace("trace faults = %d\n", n_faults);
    assert(n_faults == 10);
}
