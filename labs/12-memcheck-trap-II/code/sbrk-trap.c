#include "rpi.h"
#include "sbrk-trap.h"
#include "memmap-default.h"

static void *no_trap_heap;
static uint32_t no_trap_nbytes = 0;

// is addresss <addr> in the heap?  if so can check.
// 
// [currently we only fault on the heap, so is just
// a sanity check.]
int sbrk_in_heap(uint32_t addr) {
    let s = kmalloc_heap_start();
    let e = kmalloc_heap_end();

    void *p = (void*)addr;
    if(p >= s && p < e)
        return 1;
    return 0;
}

// we replicate this from ck-gc.c
void *sbrk(long increment) {
    if(increment > MB(1))
        panic("increment way too big: %d\n", increment);

    // if shadow memory would need to initialize.
    return kmalloc(increment);
}


void sbrk_init(void) {
    assert(!mmu_is_enabled());
    vm_map_everything(1);
    assert(mmu_is_enabled());

    // allocate a non-trapping heap for tool use.
    no_trap_heap = (void*)mb_map(0, dom_kern, no_user);
    no_trap_nbytes = MB(1);
}

static inline unsigned roundup(unsigned x, unsigned n) {
    // power of 2
    assert((n&-n) == n);
    return (x+(n-1)) & (~(n-1));
}



// checkers should call this for their own memory so don't 
// recursively trap.  
void *notrap_alloc(unsigned n) {
    assert(n);
    n = roundup(n,8);
    assert(n%8==0);

    if(n > no_trap_nbytes)
        panic("requesting %d bytes: only have %d\n", n, no_trap_nbytes);

    void *addr = no_trap_heap;
    no_trap_heap += n;
    memset(addr, 0, n);
    return addr;
}

void notrap_free(void *ptr) {
    // do nothing.
}
