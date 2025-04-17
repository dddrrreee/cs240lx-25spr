// checking allocator.  used for the gc lab.   main functionality:
// given a pointer <p> to an allocated block <b>, can look up the 
// associated header.
#ifndef __CKALLOC_H__
#define __CKALLOC_H__

// helper code for tracking callsite.
#include "src-loc.h"

// a block can be ALLOCED or FREED
typedef enum {  ALLOCED = 11, FREED } state_t;

// these are used for debugging malloc lab
enum { REDZONE_NBYTES = 128, REDZONE_VAL = 0xfe };

// this header is prepended to each allocated block.
typedef struct ck_hdr {
    // next allocated block or next free block.
    struct ck_hdr *next;

    // how many bytes the user requested to allocate: this can be less
    // than the actual amount we allocatd (can't be less!).
    uint32_t nbytes_alloc;  

    // used for checking: record the state of the block.
    // { ALLOCED, FREED }
    uint32_t state;          

    // unique block id: this lets us compare different runs even if the 
    // allocation location changes.  starts at 0.
    uint32_t block_id;

    // see src-loc.h: holds file, fn, lineno that ckalloc was called
    // at.
    src_loc_t alloc_loc;    

    // used for gc: i didn't cksum these.
    uint32_t refs_start;    // number of pointers to the start of the block.
    uint32_t refs_middle;   // number of pointers to the middle of the block.

    uint16_t mark;          // 0 initialize.
    
    // used in the error checking lab.
    uint8_t rz1[REDZONE_NBYTES];
} hdr_t;

// requires that <p> points to the start of the allocated region.
static inline unsigned ck_blk_id(void *p) {
    hdr_t *h = p;
    return h[-1].block_id;
}

// returns pointer to the first allocated header block.
hdr_t *ck_first_alloc(void);

// returns pointer to the first free'd header block.
hdr_t *ck_first_free(void);

// given header <h>: returns pointer to next hdr
// or 0 if none.
static inline hdr_t *ck_next_hdr(hdr_t *p) {
    return p ? p->next : 0;
}

// given header <h> returns the number of bytes 
// allocated.
static inline unsigned ck_nbytes(hdr_t *h) {
    return h->nbytes_alloc;
}

// given header <h> return a pointer to the start of
// allocated data.
static inline void *ck_data_start(hdr_t *h) {
    return &h[1];
}

// given header <h> return pointer to the end of its
// allocated data.
static inline void *ck_data_end(hdr_t *h) {
    return (char *)ck_data_start(h) + ck_nbytes(h);
}

// pointer to first redzone [debug alloc lab]
static inline uint8_t *ck_get_rz1(hdr_t *h) {
    return h->rz1;
}

// pointer to the second redzon [debug alloc lab]
static inline uint8_t *ck_get_rz2(hdr_t *h) {
    return ck_data_end(h);
}

// is <ptr> in block <h>?
unsigned ck_ptr_in_block(hdr_t *h, void *ptr);

// returns header if pointer <p> on the allocated list?
hdr_t *ck_ptr_is_alloced(void *ptr);

// we do things this way so we can automatically pass in
// the location the routine was called at (using <SRC_LOC_MK()>)
// --- makes error reporting better.
#define ckalloc(_n) (ckalloc)(_n, SRC_LOC_MK())
#define ckfree(_ptr) (ckfree)(_ptr, SRC_LOC_MK())
void *(ckalloc)(uint32_t nbytes, src_loc_t loc);
void (ckfree)(void *addr, src_loc_t loc);

// integrity check the allocated / freed blocks in the heap
//
// returns number of errors in the heap.   stops checking
// if heap is in an unrecoverable state.
//
// TODO (extensions): 
//  - probably should have returned an error log, so that the 
//    caller could report / fix / etc.
//  - give an option to fix errors so that you can keep going.
int ck_heap_errors(void);

// public function: call to flag leaks (part 1 of lab).
//  - <warn_no_start_ref_p> = 1: warn about blocks that only have 
//    internal references.
//
//  - returns number of bytes leaked (where bytes = amount of bytes the
//    user explicitly allocated: does not include redzones, header, etc).
unsigned ck_find_leaks(int warn_no_start_ref_p);

// mark and sweep: works similarly to ck_find_leaks, manually
// frees any unreferenced blocks and resets state to FREED
// 
// Invariant:
//  - it should always be the case that after calling ck_gc(), 
//    immediately calling ck_find_leaks() should return 0 bytes 
//    found.
unsigned ck_gc(void);

// These two routines are just used for test cases.

// Expects no leaks.
void check_no_leak(void);
// Expects leaks.
unsigned check_should_leak(void);

#if 0
// info about the heap useful for checking.
struct heap_info {
    // original start of the heap.
    void *heap_start;
    // end of active heap (the next byte we would allocate)
    void *heap_end;

    // ckfree increments this on each free.
    unsigned nbytes_freed;
    // ckmalloc increments this on each free.
    unsigned nbytes_alloced;
};

struct heap_info heap_info(void);
#endif


// can use this to do debugging that you can turn off an on.
#define ck_debug(args...) \
 do { if(ck_verbose_p) debug("CK_DEBUG:" args); } while(0)

// just emits an error.
#define ck_error(_h, args...) do {      \
        trace("ERROR:");\
        printk(args);                    \
        hdr_print(_h);                  \
} while(0)

// emit error, then panic.
#define ck_panic(_h, args...) do {      \
        trace("ERROR:");\
        printk(args);                    \
        hdr_print(_h);                  \
        panic(args);                    \
} while(0)

// used to print out header values.
static void inline hdr_print(hdr_t *h) {
    trace("\tlogical block id=%u, [addr=%p] nbytes=%d\n", 
            h->block_id, 
            ck_data_start(h),
            h->nbytes_alloc);
    src_loc_t *l = &h->alloc_loc;
    if(l->file)
        trace("\tBlock allocated at: %s:%s:%d\n", l->file, l->func, l->lineno);

#if 0
    // next lab
    l = &h->free_loc;
    if(h->state == FREED && l->file)
        trace("\tBlock freed at: %s:%s:%d\n", l->file, l->func, l->lineno);
#endif
}

// control amount of printing.
extern unsigned ck_verbose_p;
static inline void ck_verbose_set(int v) {
    ck_verbose_p = v;
}

#endif
