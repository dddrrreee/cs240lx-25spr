/*************************************************************************
 * engler, cs240lx: Purify/Boehm style leak checker/gc starter code.
 *
 * We've made a bunch of simplifications.  Given lab constraints:
 * people talk about trading space for time or vs. but the real trick 
 * is how to trade space and time for less IQ needed to see something 
 * is correct. (I haven't really done a good job of that here, but
 * it's a goal.)
 * 
 * XXX: note: I don't think the gcc_mb stuff is needed?  Remove and see if get
 * errors.
 * 
 * need to wrap up in regions: the gc should be given a list of places to scan,
 * and there should be one place that knows all the places we use.
 * i think just set this up in cstart?    it has to know where stuff is so it
 * can init.  you can then add a region later.
 */
#include "rpi.h"
#include "rpi-constants.h"
#include "ckalloc.h"
#include "kr-malloc.h"
#include "libc/helper-macros.h"

// implement these five routines below.

// these four are at the end of this file.
static void mark(const char *where, uint32_t *p, uint32_t *e);
static void mark_all(void);
static unsigned sweep_leak(int warn_no_start_ref_p);
static unsigned sweep_free(void);

// this routine is in <gc-asm.S>
//
// write the assembly to dump all registers.
// need this to be in a seperate assembly file since gcc 
// seems to be too smart for its own good.
void dump_regs(uint32_t v[16]);

/**********************************************************************
 * starter code: don't have to modify other than for extensions.
 */

// currently assume allocated memory is contiguous.  otherwise
// we need a list of allocated regions.
static void * heap_start;
static void * heap_end;

void *sbrk(long increment) {
    static int init_p;

    assert(increment > 0);
    if(init_p) 
        panic("not handling\n");
    else {
        unsigned onemb = 0x100000;
        heap_start = (void*)onemb;
        heap_end = (char*)heap_start + onemb;
        kmalloc_init_set_start((void*)onemb, onemb);
        init_p = 1;
    }
    return kmalloc(increment);
}

// quick check that the pointer is between the start of
// the heap and the last allocated heap pointer.  saves us 
// walk through all heap blocks.
//
// MAYBE: warn if the pointer is within number of bytes of
// start or end to detect simple overruns.
static int in_heap(void *p) {
    // should be the last allocated byte(?)
    if(p < heap_start || p >= heap_end)
        return 0;
    // output("ptr %p is in heap!\n", p);
    return 1;
}

// given potential address <addr>, returns:
//  - 0 if <addr> does not correspond to an address range of 
//    any allocated block.
//  - the associated header otherwise (even if freed: caller should
//    check and decide what to do in that case).
//
// XXX: you'd want to abstract this some so that you can use it with
// other allocators.  our leak/gc isn't really allocator specific.
static hdr_t *is_ptr(uint32_t addr) {
    void *p = (void*)addr;
    
    if(!in_heap(p))
        return 0;
    return ck_ptr_is_alloced(p);
}

// return number of bytes allocated?  freed?  leaked?
// how do we check people?
unsigned ck_find_leaks(int warn_no_start_ref_p) {
    mark_all();
    return sweep_leak(warn_no_start_ref_p);
}

// used for tests.  just keep it here.
void check_no_leak(void) {
    // when in the original tests, it seemed gcc was messing 
    // around with these checks since it didn't see that 
    // the pointer could escape.
    gcc_mb();
    if(ck_find_leaks(1))
        panic("GC: should have no leaks!\n");
    else
        trace("GC: SUCCESS: no leaks!\n");
    gcc_mb();
}

// used for tests.  just keep it here.
unsigned check_should_leak(void) {
    // when in the original tests, it seemed gcc was messing 
    // around with these checks since it didn't see that 
    // the pointer could escape.
    gcc_mb();
    unsigned nleaks = ck_find_leaks(1);
    if(!nleaks)
        panic("GC: should have leaks!\n");
    else
        trace("GC: SUCCESS: found %d leaks!\n", nleaks);
    gcc_mb();
    return nleaks;
}

/***********************************************************************
 * implement the routines below.
 */


// mark phase:
//  - iterate over the words in the range [p,e], marking any block 
//    potentially referenced.
//  - if we mark a block for the first time, recurse over its memory
//    as well.
//
// EXTENSION: if we have lots of words, could be faster with shadow memory 
// or a lookup table.  however, given our small sizes, this stupid 
// search may well be faster :)
//
// If you switch: measure speedup!
//
static void mark(const char *where, uint32_t *p, uint32_t *e) {
    assert(p<e);
    assert(aligned(p,4));
    assert(aligned(e,4));

    todo("implement the rest\n");
}

// do a sweep, warning about any leaks.
static unsigned sweep_leak(int warn_no_start_ref_p) {
	unsigned nblocks = 0, errors = 0, maybe_errors=0;
	output("---------------------------------------------------------\n");
	output("checking for leaks:\n");

    // sweep through all the allocated blocks.  
    //  1. if there are no pointers to a block at all: give an error.
    //  2. if there are only refs to the middle and <warn_no_start_ref_p>
    //     is true, give a maybe leak.
    for(hdr_t *h = ck_first_alloc(); h; h = ck_next_hdr(h), nblocks++)
        todo("implement the rest\n");

	trace("\tGC:Checked %d blocks.\n", nblocks);
	if(!errors && !maybe_errors)
		trace("\t\tGC:SUCCESS: No leaks found!\n");
	else
		trace("\t\tGC:ERRORS: %d errors, %d maybe_errors\n", 
						errors, maybe_errors);
	output("----------------------------------------------------------\n");
	return errors + maybe_errors;
}

// a very slow leak checker.
static void mark_all(void) {

    // slow: should not need this: remove after your code
    // works.
    for(hdr_t *h = ck_first_alloc(); h; h = ck_next_hdr(h)) {
        h->mark = h->refs_start = h->refs_middle = 0;
    }
	// pointers can be on the stack, in registers, or in the heap itself.

    // get all the registers.
    uint32_t regs[16];
    dump_regs(regs);
    todo("kill caller-saved registers so we don't falsely suppress errors.\n");

    mark("regs", regs, &regs[14]);

    // get the start of the stack (see libpi/staff-start.S)
    // and sweep up from the current stack pointer (from <regs>)
    uint32_t *stack_top = (void*)STACK_ADDR;
    todo("get sp and mark stack\n");


    // sweep zero-initialized data <bss> and non-zero 
    // initialized <data>
    // these symbols are defined in our memmap
	extern uint32_t __bss_start__, __bss_end__;
	mark("bss", &__bss_start__, &__bss_end__);

	extern uint32_t __data_start__, __data_end__;
	mark("data segment", &__data_start__, &__data_end__);
}


// similar to sweep_leak: go through and <ckfree> any ALLOCED
// block that has no references all all (nothing to start, 
// nothing to middle).
static unsigned sweep_free(void) {
	unsigned nblocks = 0, nfreed=0, nbytes_freed = 0;
	output("---------------------------------------------------------\n");
	output("compacting:\n");

    todo("sweep through allocated list: free any block that has no pointers\n");

	trace("\tGC:Checked %d blocks, freed %d, %d bytes\n", nblocks, nfreed, nbytes_freed);

    return nbytes_freed;
}

unsigned ck_gc(void) {
    mark_all();
    unsigned nbytes = sweep_free();

    // perhaps coalesce these and give back to heap.  will have to modify last.

    return nbytes;
}
