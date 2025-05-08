// a simple example of how to trap and emulate
// memory operations for a given memory regions
// by using ARMv6 domain fault trapping.
//
// we want to do trap-and-emulate so we can make 
// memory tools easily.  these tools require:
//   1. trapping all loads and stores before they 
//      occur.
//   2. inspecting them (eg to check if 
//      the address was illegal).  
//   3. if the access was bad: emit an error message.
//   4. otherwise perform the memory instruction 
//      and continue.
//
// using some clever ARM hardware tricks we can
// do this pretty easily.
// 
// the basic idea: given a memory region M we want
// to trap:
//   1. tag all of M's virtual memory mappings 
//      with its own domain id <d_trap> (ARMv6 has 
//      16 domains).
//   2. when we want to trap, remove permissions
//      for the <d_trap> domain.
//   3. when we want turn off trapping, add 
//      permissions for the <d_trap> domain.
//   4. when we get a fault and want to emulate
//      the memory instruction:
//        A. disable trapping so we can access the 
//           memory.
//        B. perform the memory op
//        C. enable trapping.
//        D. jump back to the next instruction.
//
// the only painful part of this on the arm is
// step 4.B: performing the memory op since
// since ARM has a huge number of memory ops.
// thus, *for this example code* we cheat by 
// forcing the client to access trapping memory 
// with either PUT32 or GET32 because:
//    1. they use ldr (GET32) or str (PUT32)
//    2. so it's easy to emulate them. 
//
// you can see this by looking at any .list file:
//  0000803c <PUT32>:
//      803c:   e5801000    str r1, [r0]
//      8040:   e12fff1e    bx  lr
//
//  00008044 <GET32>:
//      8044:   e5900000    ldr r0, [r0]
//      8048:   e12fff1e    bx  lr
//
// The lab removes this restriction by using 
// single-stepping to make emulating any 
// memory operation easy (by just running it)
#include "rpi.h"

// 140e code for doing virtual memory using
// TLB pinning.
#include "pinned-vm.h"
// 140e exception handling support
#include "full-except.h"
// 140e helpers for getting exception reason.
#include "armv6-except.h"
// 140e code for full context switching
// (caller,callee and cpsr).
#include "switchto.h"

// default definitions for how address space
// is laid out.
#include "memmap-default.h"

enum { 
    // pick some unused domain id.
    kern_dom = 1,
    heap_dom = 2,
    
    // pre-compute the domain register values
    // that we need.
    //
    //  - <DOM_client> = hardware checks the page
    //    permissions.
    //  - each domain is 2 bits so we have to multiply
    //    by 2.

    // this only has the kernel domain: 
    // this will trap any heap acces.
    trap_heap_access = DOM_client << (kern_dom*2),

    no_trap          = trap_heap_access 
                     |  DOM_client << (heap_dom*2)
};

// start trapping heap accesses by switching the
// domain register.
static void trap_on(void) {
    domain_access_ctrl_set(trap_heap_access);

    // sanity check.  remove for speed.
    uint32_t v = domain_access_ctrl_get();
    assert(v = trap_heap_access);
}

// turn heap-trapping off.
static void trap_off(void) {
    domain_access_ctrl_set(no_trap);

    // sanity check.  remove for speed.
    uint32_t v = domain_access_ctrl_get();
    assert(v = no_trap);
}

// we need virtual memory for trapping.  so setup the
// simplest possible VM: 
//   identity mapping of only the 1mb sections used by 
//   our basic process (code, data, heap, stack and 
//   exception stack).  
// we pin these entries in the tlb so we don't even 
// need a page table.  
//
// we tag the heap with its own domain id (<heap_dom>), 
// and everything else with a different one <kern_dom>
//
// to keep things simple, we specialize this to what we
// need with our simple memtrap tests.
//
static int vm_map_everything(void) {
    // initialize the hardware MMU for pinned vm
    pin_mmu_init(no_trap);
    assert(!mmu_is_enabled());


    // compute the different mapping attributes.  
    // we only do simple uncached mappings today
    // (but shouldn't matter).

    // device memory: kernel domain, no user access, 
    // memory is strongly ordered, not shared.
    // we use 16mb section.
    pin_t dev  = pin_16mb(pin_mk_global(kern_dom, no_user, MEM_device));

    // kernel memory: same as device, but is only uncached.  
    pin_t kern = pin_mk_global(kern_dom, no_user, MEM_uncached);

    // heap.  different from kernel memory b/c:
    // 1. needs a different domain so will trap.
    // 2. user_access: since when we add single stepping 
    //    the code will run at user level.  (alternatively
    //    we could set <heap_dom> to manager permission)
    pin_t heap = pin_mk_global(heap_dom, user_access, MEM_uncached);

    // now identity map kernel memory.
    unsigned idx = 0;
    pin_mmu_sec(idx++, SEG_CODE, SEG_CODE, kern);

    // we could mess with the alignment to give the
    // heap more memory.
    pin_mmu_sec(idx++, SEG_HEAP, SEG_HEAP, heap);
    pin_mmu_sec(idx++, SEG_STACK, SEG_STACK, kern);
    pin_mmu_sec(idx++, SEG_INT_STACK, SEG_INT_STACK, kern);
    pin_mmu_sec(idx++, SEG_BCM_0, SEG_BCM_0, dev);

    // we aren't using user processes or anythings so we
    // just claim ASID=1 as our address space identifier.
    enum { ASID = 1 };
    pin_set_context(ASID);

    // turn the MMU on.
    assert(!mmu_is_enabled());
    mmu_enable();
    assert(mmu_is_enabled());
    // vm is now live!

    // return index in case if want to allocate more.
    return idx;
}

// simple data_fault handler to illustrate
// how to handle trapping memory operations.
//
// since ARM has a huge number of memory ops,
// we cheat by forcing the client to access
// trapping memory with either PUT32 or GET32
// because:
//    1. they use ldr (GET32) or str (PUT32)
//    2. so it's easy to emulate them. 
// NOTE: the lab removes this restriction
// by using single-stepping.
//
// emulation:
//  1. checks that the data abort fault is
//     caused by a domain permission error.
//  2. turns trapping off.
//  3. checks that we got called from
//     GET32/PUT32 and emulates them.
//  4. turns trapping back on
//  5. jumps back to the next instruction
//     after the faulting instruction
//     (should be a bx lr).
static void data_fault(regs_t *r) {
    // b4-43 [140e pinned mem]
    uint32_t reason     = data_abort_reason();
    // b4-44 [140e pinned mem]
    uint32_t fault_addr = data_abort_addr();

    // b4-20 has the different reasons.
    if(reason != DOMAIN_SECTION_FAULT)
        panic("was not a debug fault: %b\n", reason);

    // Q: if we don't turn the trapping off?
    trap_off();

    // emulate the instruction.
    //
    // we expect exactly two legal fault 
    // locations: PUT32 and GET32.  Both have
    // trivial mem operations that we emulate.
    //  - PUT32 = str r1, [r0]
    //  - GET32 = ldr r0, [r0]
    uint32_t pc = r->regs[15];
    if(pc == (uint32_t)PUT32) {
        // <data_fault_from_ld> should be false
        // since PUT32 does a store.
        assert(!data_fault_from_ld());
        // emulate: str r1, [r0]
        *(uint32_t *)r->regs[0] = r->regs[1];
    } else if(pc == (uint32_t)GET32) {
        // <data_fault_from_ld> should be true
        // since GET32 does a load.
        assert(data_fault_from_ld());
        // emulate: ldr r0, [r0]
        r->regs[0] = *(uint32_t*)r->regs[0];
    } else
        panic("unexpected fault pc=%x\n", pc);
    
    // skip the faulting memory instruction by
    // adding 4 bytes (the size of the instruction)
    // to the pc.
    r->regs[15] += 4;

    // make sure that the instruction we are jumping
    // back to is a "bx lr" instruction
    // see: the .list for GET32 and PUT32
    pc = r->regs[15];
    uint32_t inst = *(uint32_t *)pc;
    if(inst != 0xe12fff1e)
        panic("%x: not a bx lr!  inst=%x\n", pc, inst);


    // turn trapping back on
    trap_on();

    // jump back 
    switchto(r);
}

// we don't expect prefetch faults for this code.
static void prefetch_fault(regs_t *r) {
    panic("we got a prefetch abort fault at pc=%x\n", r->regs[15]);
}

void notmain(void) { 
    // our kmalloc standard init.
    kmalloc_init_set_start((void*)SEG_HEAP, MB(1));

    // setup the full fault handlers [140e] that take in
    // the full register structure --- all 16 general
    // registers and the cpsr  --- that were live at the 
    // fault.
    full_except_install(0);
    full_except_set_data_abort(data_fault);
    full_except_set_prefetch(prefetch_fault);

    // map everything: when this returns vm is on!
    int idx = vm_map_everything();
    assert(mmu_is_enabled());

    // get the current domain.
    let x = domain_access_ctrl_get();
    output("%d total mappings, domain = %b\n", idx, x);

    // make sure trapping is off while we mess with the
    // heap.
    trap_off();

    uint32_t *v = kmalloc(sizeof *v);

    // turn trapping back on.  NOTE: for this
    // simplistic test when trapping is on
    // we can *only* read/write to heap memory 
    // using GET32/PUT32 b/c of how we wrote
    // the data abort handler.
    trap_on();

    // do <N> trials where we read/write <v>
    // using GET32/PUT32 and validate the result.
    enum { N = 10 };
    for(int i = 0; i < N; i++) {
        output("about to do a PUT32!\n");

        // write a 32 bit value so we can
        // make sure no byte got messed up or
        // ignored.
        uint32_t expect = 0xfaf0faf0+i;
        put32(v, expect);
        output("about to do a GET!\n");
        uint32_t got = get32(v);

        // check that what we read equals what we
        // wrote.
        if(expect != got)
            panic("failed: got=%x, expect=%x\n", got, expect);
        else
            output("%d: success: got=%x, expect=%x\n", 
                                        i,got, expect);
    }
    output("SUCCESS!  passsed %d trials\n", N);
}
