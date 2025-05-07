// a very incomplete set of tests that use the arm PMU counters
#include "rpi.h"
#include "pinned-vm.h"
#include "memmap-default.h"
// in libpi/include
#include "../code/rpi-pmu.h"
#include "cache-support.h"
#include "full-except.h"

// helper to do a raw dache invalidation: if there are dirty
// blocks they will be lost (you can see b/c subsequent reads
// will return old copies)
static inline void dcache_inv(void) {
    uint32_t r=0;
    asm volatile ("mcr p15, 0, %0, c7, c6, 0" :: "r"(r));
    prefetch_flush();
}

static inline void inv_tlb(void) {
    uint32_t x = 0;
    asm volatile ("mcr p15, 0, %0, c8, c7, 0" :: "r"(x));
}

// free address, 16mb aligned.
enum { cache_addr = MB(16) };

// do simple vm identity mapping.  return one single
// entry that is cached.  easy enough to change: this
// is just example.
static void *map_everything(void) {

    //****************************************************
    // 1. map kernel memory 

    // device memory: kernel domain, no user access, 
    // memory is strongly ordered, not shared.
    // we use 16mb section.
    pin_t dev  = pin_16mb(pin_mk_global(dom_kern, no_user, MEM_device));

    // kernel memory: same, but is only uncached.  
    pin_t kern = pin_mk_global(dom_kern, no_user, MEM_uncached);

    pin_mmu_init(~0);
    assert(!mmu_is_enabled());

    unsigned idx = 0;
    pin_mmu_sec(idx++, SEG_CODE, SEG_CODE, kern);
    pin_mmu_sec(idx++, SEG_HEAP, SEG_HEAP, kern);
    pin_mmu_sec(idx++, SEG_STACK, SEG_STACK, kern);
    pin_mmu_sec(idx++, SEG_INT_STACK, SEG_INT_STACK, kern);
    pin_mmu_sec(idx++, SEG_BCM_0, SEG_BCM_0, dev);

    //****************************************************
    // 2. create a single user entry:


    // for today: user entry attributes are:
    //  - non-global
    //  - user dom
    //  - user r/w permissions. 
    // for this test:
    //  - also uncached (like kernel)
    //  - ASID = 1
    enum { ASID = 1 };

    // map a single cached page with writeback allocation.
    // you can change this if you want different caching.
    // <mem-attr.h> has other examples and page numbers.
    enum { wb_alloc     =  TEX_C_B(    0b001,  1, 1) };
    pin_t attr = pin_mk_global(dom_kern, perm_rw_priv, wb_alloc);
    pin_mmu_sec(idx++, cache_addr, cache_addr, attr);
    assert(idx<=8);

    pin_set_context(ASID);
    assert(!mmu_is_enabled());

    return (void*)cache_addr;
}

void notmain(void) { 
    // our standard init.
    kmalloc_init_set_start((void*)SEG_HEAP, MB(1));
    full_except_install(0);

    // map everything, get a pointer to cached.
    void *mem = map_everything();

    // turn the MMU on.
    assert(!mmu_is_enabled());
    mmu_enable();
    assert(mmu_is_enabled());
    // we now have VM!

    volatile uint32_t *ptr = mem;

    // turn all caches on.
    caches_all_on();
    assert(caches_all_on_p());

    uint32_t miss,access, dtlb_miss;

    /***********************************************************
     *  check that we get a dcache miss and a micro dtlb miss.
     */
    pmu_stmt_measure_set(miss, dtlb_miss,
        "verifying we get a data cache miss on first ref",
        dcache_miss, dtlb_miss,
        {
            *ptr;
        });
    
    if(miss != 1)
        panic("expected 1 dcache miss: have %d\n", miss);
    if(dtlb_miss != 1)
        panic("expected 1 dtlb miss: have %d\n", dtlb_miss);
    output("SUCCESS: dcache miss=%d, dtlb miss=%d\n", miss, dtlb_miss);

    /***********************************************************
     *  now that we accessed <ptr> should have zero dtlb and dcache miss.
     */
    pmu_stmt_measure_set(miss, dtlb_miss,
        "making sure no data cache miss",
        dcache_miss, dtlb_miss,
        {
            *ptr;
            *ptr;
            *ptr;
            *ptr;
            *ptr;
        });
    
    if(miss != 0)
        panic("expected 1 miss: have %d\n", miss);
    if(dtlb_miss)
        panic("expected 0 dtlb miss: have %d\n", dtlb_miss);
    output("SUCCESS: miss=%d, access=%d\n", miss, dtlb_miss);


    /*************************************************************
     * invalidate the tlb make make sure that we get misses again.
     */

    inv_tlb();
    pmu_stmt_measure_set(miss, dtlb_miss,
        "should dtlb miss after inv tlb",
        tlb_miss, dtlb_miss,
        {
            *ptr;
        });
    
    if(dtlb_miss != 1)
        panic("expected a dtlb miss: have %d\n", dtlb_miss);
    if(miss != 0)
        panic("main tlb missed? cnt=%d\n", miss);
    output("success!  tlb miss = %d, dtlb miss = %d\n", miss, dtlb_miss);
}
