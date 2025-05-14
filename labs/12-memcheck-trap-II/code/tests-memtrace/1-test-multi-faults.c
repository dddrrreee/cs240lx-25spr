// a bit fancier: allocate a chunk of trapping memory, iterate over
// it taking faults, verify that the answer is correct.  takes
// around 30k faults with the current values.  just change <N>
// or <K> to bump this number up!
//
// great extension: speed this up alot!
#include "rpi.h"
#include "memtrace.h"
#include "memmap-default.h"

// need to specify the size: 1,2,4,8,... --- gcc is allowed to merge
// adjacent object writes, but i don't think will ever be able to see.
//
// probably better to pass in the fault.  but then you have to eat the
// load misses.

// 0 = quiet.
enum { quiet_p = 1 };

static int trace_handler(void *data, fault_ctx_t *f) {
    unsigned *n = data;
    *n += 1;

    if(!quiet_p)
        output("\t%d: memtrace: pc=%x, addr=%x, nbytes=%d, load=%d\n",
                    *n, 
                    f->pc,
                    f->addr, 
                    f->nbytes, 
                    f->load_p);

    return MEMTRACE_OK;
}

void notmain(void) {
    caches_enable();
    trace("memcheck tracing\n");

    int n_faults = 0;
    memtrace_init(&n_faults, trace_handler, 0, dom_trap);
    memtrace_yap_off();

    enum { K = 1024 };
    uint32_t *p = kmalloc(sizeof *p * K);
    
    memtrace_trap_enable();

    enum { N = 128/8 };
    trace("about to trace: expect %d faults\n", (N*K)*2+K);

    // inefficient way to increment every location
    // by <N>.
    for(int i = 0; i < N; i++)
        for(int j = 0; j < K; j++)
            p[j] += 1;

    // go and check everything
    for(int j = 0; j < K; j++) {
        if(p[j] != N)
            panic("N=%d, *p[%d]=%d\n", N, j, p[j]);
    }
    trace("SUCCESS: total faults = %d\n", n_faults);
}
