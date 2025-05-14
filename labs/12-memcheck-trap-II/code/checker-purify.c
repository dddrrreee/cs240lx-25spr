// engler: cs240lx: a sort-of purify checker: gives an error
// message if a load/store to a heap addres is:
//   - not within a legal block.
//   - to freed memory.
//
// uses the checking allocator (ckalloc).  for the moment
// just reboot on an error.
// 
// limits:
//   - does not check that its within the *correct*
//     legal block (can do this w/ replay).
//   - does not track anything about global or stack memory.
#include "memtrace.h"
#include "ckalloc.h"
#include "purify.h"
#include "sbrk-trap.h"
#include "memmap-default.h"

static int purify_quiet_p = 0;
void purify_yap_off(void) {
    purify_quiet_p = 1;
    memtrace_yap_off();
}
void purify_yap_on(void) {
    purify_quiet_p = 0;
    memtrace_yap_on();
}

// turn off trapping when we allocate.  Q: what if we 
// don't do this?
void *purify_alloc_raw(unsigned n, src_loc_t l) {
    memtrace_trap_disable();

        // if shadow memory: mark [p,p+n) as allocated
        unsigned *p = (ckalloc)(n, l);

    memtrace_trap_enable();
    return p;
}

// turn off trapping when we free.  Q: what if we 
// don't do this?
void purify_free_raw(void *p, src_loc_t l) {
    memtrace_trap_disable();

        // if shadow memory: mark [p,p+n) as free
        (ckfree)(p, l);

    memtrace_trap_enable();
}

static int handler(void *data, fault_ctx_t *f) {
    todo("implement this code!  if error: reboot");
    return MEMTRACE_OK;
}

void purify_init(void) {
    memtrace_init(0, handler, 0, dom_trap);
    memtrace_trap_enable();
    memtrace_yap_off();
}
