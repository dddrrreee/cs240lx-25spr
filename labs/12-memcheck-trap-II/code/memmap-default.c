// a very basic virtual memory setup suitable for 
// memory trapping and general memory protection.
//   - identity map the regions defined in <memmap>
// over-simplistic in many ways, but we ship it so can
// get work done.
#include "rpi.h"

// 140e code for doing virtual memory using
// TLB pinning.
#include "pinned-vm.h"
// 140e exception handling support
#include "full-except.h"

// default definitions for how address space
// is laid out.
#include "memmap-default.h"

enum { MAX_PIN = 8 };
static int pin_idx;

//**************************************************************
// very simple minded 1MB segment allocator.
//

// XXX: need to use mbox to check that this is actually avail.
enum { MAX_SECS = 156 };  
static uint32_t sections[MAX_SECS];

// hint for where we left off.
static uint32_t seg_avail;

static uint32_t addr_to_sec(uint32_t sec) {
    return sec >> 20;
}
static int seg_is_legal(uint32_t sec) {
    return sec < MAX_SECS;
}
// is physical address <pa> allocated?
static int seg_is_alloced(uint32_t pa) {
    uint32_t s = addr_to_sec(pa);
    assert(seg_is_legal(s));
    return sections[s];
}

static int seg_alloc(uint32_t pa) {
    if(seg_is_alloced(pa))
        panic("pa=%x is already allocated\n", pa);
    return sections[pa] = 1;
}

// find a free segment, starting from <seg_avail> hint.
// just return it, don't allocate.
uint32_t mb_find_free(void) {
    uint32_t s = addr_to_sec(seg_avail);

    for(; s < MAX_SECS; s++) {
        if(!seg_is_alloced(s)) {
            uint32_t addr = s << 20;
            seg_avail = addr + MB(1);
            return addr;
        }
    }
    panic("no segment avail?\n");
}

// reserve a mb but do not map it.  if <addr>=0,
// reserves the next free mb starting from
// <seg_avail>.
uint32_t mb_reserve(uint32_t addr) {
    assert(addr % MB(1) == 0);

    // find free address if need.
    if(!addr)
        addr = mb_find_free();
    // now allocate it.
    if(seg_is_alloced(addr))
        panic("can't allocated %x\n", addr);

    return addr;
}

// find a free MB section, and allocate it with <dom>
// and permissions <perm>.  returns the address.
uint32_t mb_map(uint32_t addr, uint32_t dom, uint32_t perm) {
    if(pin_idx >= MAX_PIN)
        panic("too many pins!  %d\n", pin_idx);
    // needs to be MB aligned.
    assert(addr % MB(1) == 0);
    // legal dom
    assert(dom<16);
    // mild sanity checking of perms.
    assert(perm == no_user || perm == user_access);

    // reserve MB segment.
    addr = mb_reserve(addr);

    // now pin it.
    pin_t attr = pin_mk_global(dom, perm, MEM_uncached);
    pin_mmu_sec(pin_idx++, addr, addr, attr);

    return addr;
}

//**************************************************************
// setup the simplest possible VM: 
//   identity mapping of only the 1mb sections used by 
//   our basic process (code, data, heap, stack and 
//   exception stack).  
// we pin these entries in the tlb so we don't even 
// need a page table.  
//
// we tag the heap with its own domain id (<dom_trap>), 
// and everything else with a different one <kern_dom>
//
// to keep things simple, we specialize this to what we
// need with our simple memtrap tests.

// need to change this to allow heap to be parameterized.
// (or, probably better: added later).
int vm_map_everything(uint32_t mb) {
    // right now we aren't handling other stuff.
    assert(mb==1);
    kmalloc_init_set_start((void*)SEG_HEAP, MB(mb));

    full_except_install(0);

    // initialize the hardware MMU for pinned vm
    pin_mmu_init(no_trap_access);
    assert(!mmu_is_enabled());

    // compute the different mapping attributes.  
    // we only do simple uncached mappings today
    // (but shouldn't matter).

    // device memory: kernel domain, no user access, 
    // memory is strongly ordered, not shared.
    // we use 16mb section.  
    //
    // need to check: arm1176 states that 16mb sections
    // force dom=0.  this might only be true for page tables.  
    pin_t dev  = pin_16mb(pin_mk_global(dom_kern, no_user, MEM_device));

    // kernel memory: same as device, but is only uncached.  
    // good speedup: change to cache, various write-back.
    pin_t kern = pin_mk_global(dom_kern, no_user, MEM_uncached);

    // heap.  different from kernel memory b/c:
    // 1. needs a different domain so will trap.
    // 2. user_access: if we are going to use single stepping 
    //    the code will run at user level.  (alternatively
    //    we could set <dom_trap> to manager permission)
    pin_t heap = pin_mk_global(dom_trap, user_access, MEM_uncached);

    // now identity map kernel memory.
    unsigned idx = 0;
    pin_mmu_sec(idx++, SEG_CODE, SEG_CODE, kern);

    // we could mess with the alignment to give the
    // heap more memory.
    pin_mmu_sec(idx++, SEG_HEAP,        SEG_HEAP,       heap);
    pin_mmu_sec(idx++, SEG_STACK,       SEG_STACK,      kern);
    pin_mmu_sec(idx++, SEG_INT_STACK,   SEG_INT_STACK,  kern);
    pin_mmu_sec(idx++, SEG_BCM_0,       SEG_BCM_0,      dev);

    // we aren't using user processes or anythings so we
    // just claim ASID=1 as our address space identifier.
    enum { ASID = 1 };
    pin_set_context(ASID);

    // turn the MMU on.
    assert(!mmu_is_enabled());
    mmu_enable();
    assert(mmu_is_enabled());
    // vm is now live!  

    // where we can continue allocating from.
    seg_avail = SEG_HEAP + MB(1);
    pin_idx = idx;

    // return next free segment.  
    return seg_avail;
}
