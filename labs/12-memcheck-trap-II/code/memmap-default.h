#ifndef __MEMMAP_DEFAULT_H__
#define __MEMMAP_DEFAULT_H__
// this file has the default values for how memory is laid out
// in our default <libpi/memmap>: where the code, data, heap, 
// stack and interrupt stack are. 
//
// there's other ways to do this but we try to make it more 
// transparent by making it as primitive as possible.
//
// XXX: probably should add a pmap structure that tracks
// the attributes used for a MB.  this lets us do identity
// mapping easily, and also check for compatibility in terms
// of caching, etc.

// the symbols 
#include "memmap.h"
#include "pinned-vm.h"


// we put all the default address space enums here:
// - kernel domain
// - user domain
// - layout.

#define MB(x) ((x)*1024*1024)

// default domains for kernel and user.  not required.
enum {
    dom_kern = 1, // domain id for kernel
    dom_user = 2,  // domain id for user
    dom_trap = 3,  // domain used for trapping

    // setting for the domain reg: 
    //  - client = checks permissions.
    //  - each domain is 2 bits
    dom_bits = DOM_client << (dom_kern*2) 
             | DOM_client << (dom_user*2),

    // this only has the kernel domain: 
    // this will trap any heap acces.
    trap_access     = dom_bits,
    no_trap_access  = trap_access
                    |  DOM_client << (dom_trap*2)
};

enum { 
    // default no user access permissions
    no_user = perm_rw_priv,
    // default user access permissions
    user_access = perm_rw_user,
};

// the default asid we use: not required.
// recall that ASID = 0 is reserved as 
// a scratch register used when switching 
// address spaces as per the ARM manual.
enum {
    default_ASID = 1
};


// These are the default segments (segment = one MB)
// that need to be mapped for our binaries so far
// this quarter. 
//
// these will hold for all our tests today.
//
// if we just map these segments we will get faults
// for stray pointer read/writes outside of this region.
//
// big limitation: the fact that binaries start at
// 0x8000 means that we can't leave 0 unmapped when
// using 1mb segments.
enum {
    // code starts at 0x8000, so map the first MB
    //
    // if you look in <libpi/memmap> you can see
    // that all the data is there as well, and we have
    // small binaries, so this will cover data as well.
    //
    // NOTE: obviously it would be better to not have 0 (null) 
    // mapped, but our code starts at 0x8000 and we are using
    // 1mb sections (which require 1MB alignment) so we don't
    // have a choice unless we do some modifications.  
    //
    // you can fix this problem as an extension: very useful!
    SEG_CODE = MB(0),

    // as with previous labs, we initialize 
    // our kernel heap to start at the first 
    // MB. it's 1MB, so fits in a segment. 
    SEG_HEAP = MB(1),

    // if you look in <staff-start.S>, our default
    // stack is at STACK_ADDR, so subtract 1MB to get
    // the stack start.
    SEG_STACK = STACK_ADDR - MB(1),

    // the interrupt stack that we've used all class.
    // (e.g., you can see it in the <full-except-asm.S>)
    // subtract 1MB to get its start
    SEG_INT_STACK = INT_STACK_ADDR - MB(1),

    // the base of the BCM device memory (for GPIO
    // UART, etc).  Three contiguous MB cover it.
    SEG_BCM_0 = 0x20000000,
    SEG_BCM_1 = SEG_BCM_0 + MB(1),
    SEG_BCM_2 = SEG_BCM_0 + MB(2),

    // we guarantee this (2MB) is an 
    // unmapped address.  
    //
    // XXX: should pull this from the active pmap.
    SEG_ILLEGAL = MB(2),
};

// default kernel attributes
static inline pin_t dev_attr_default(void) {
    return pin_mk_global(dom_kern, no_user, MEM_device);
}
// default kernel attributes
static inline pin_t kern_attr_default(void) {
    return pin_mk_global(dom_kern, no_user, MEM_uncached);
}

static inline uint32_t trap_dom(void) {
    return dom_trap;
}

// sets up a identity VM mapping of the entire address 
// space based on our default <memmap>.
//
// returns the next free MB available.
//
// note: 
//   - really need to figure out a better approach
//     for mapping the heap.  
//   - more generally: this doesn't really compose well.
//     probably should have a structure with <heap>,
//     <code>, <data>, <stack> <bcm> fields
//     with client provided (attribute, perm, and domains) 
//     for each.  fill the structure with reasonable defaults 
//     and let clients override.
//
//     could also have the base addr and size so that <memmap>
//     can fill in.  better than current approach...
int vm_map_everything(uint32_t heap_mb);

// find a free MB section, allocate it, map it with
//  - domain <dom>
//  - permissions <perm>.  
// returns the address.
uint32_t mb_map(uint32_t addr, uint32_t dom, uint32_t perm);

// reserve a 1MB segment but do not map it.  
// if <addr>=0, reserves the next free MB.
// note: should allow <n_mb> parameter.
uint32_t mb_reserve(uint32_t addr);

// find a free segment starting from the low part of the addr
// space.  note: should provide a <n_mb> param.
uint32_t mb_find_free(void);

#endif
