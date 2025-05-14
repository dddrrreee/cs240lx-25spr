#include "rpi.h"
#include "memtrace.h"
#include "memmap-default.h"

static int quiet_p = 0;
static volatile int n_faults = 0;

static int trace_handler(regs_t *r, uint32_t fault_addr, int load_p) {
    n_faults++;

    if(quiet_p)
        return 1;

#if 0
    output("%d: memtrace: pc=%x, addr=%x, load=%d\n",
                    n_faults, 
                    r->regs[15], 
                    fault_addr, 
                    load_p);
#endif

    return 1;
}

// currently just setup the virtual memory.
void trace_init(void) {
    vm_map_everything(1);
    memtrace_init_default(trace_handler, 0, dom_trap);
}

#if 0
void notmain(void) {
    output("memcheck tracing\n");
    trace_init();

    enum { K = 20 };
    uint32_t *p = kmalloc(sizeof *p * K);
    
    memtrace_yap_off();
    output("about to trace\n");
    memtrace_on();

    output("should fault on %p\n", p);
    enum { N = 1000 };
    for(int i = 0; i < N; i++)
        for(int j = 0; j < K; j++)
            p[j] += 1;

    output("turning trace off\n");
    memtrace_off();
    output("p=%d\n", *p);

    for(int j = 0; j < K; j++) {
        if(p[j] != N)
            panic("N=%d, *p[%d]=%d\n", N, j, p[j]);
    }
}
#else
#if 1
void foo(void) {
    uint64_t x = ~0;
    volatile uint64_t *u = kmalloc(sizeof *u);
    output("about to write to %x\n", u);
    *u = x;
    output("done writing to %x\n", u);
}

void notmain(void) {
    output("memcheck tracing\n");

    trace_init();
    memtrace_on();
    foo();
    memtrace_off();
}
#endif

#if 0
void notmain(void) {
    trace_init();

    uint32_t *ret = kmalloc(sizeof *ret);
    *ret = 0xe12fff1e;
    memtrace_on();
    BRANCHTO((uint32_t)ret);
    panic("back\n");

    // memtrace_on();
    panic("back!\n");
}
#endif

#endif
