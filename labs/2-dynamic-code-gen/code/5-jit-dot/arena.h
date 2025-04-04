#ifndef __ARENA_H__
#define __ARENA_H__

#include "rpi.h"
#include "pointer.h"
// #include "kr-malloc.h"

typedef struct arena {
    unsigned total_nbytes;
    void *base;
    void *end;

    void *cur;
} arena_t;

static inline arena_t *
arena_mk(unsigned total_nbytes) {
    assert(total_nbytes > 1024);
    assert(total_nbytes <= 64 * 1024 * 1024);
    assert(total_nbytes % 8 == 0);

    arena_t *a = kmalloc(sizeof *a);
    assert(a);

    a->total_nbytes = total_nbytes;
    a->cur = a->base = kmalloc(total_nbytes);
    assert(a->base);
    a->end = (char*)a->base+total_nbytes;

    return a;
}

static inline void *
arena_alloc_notzero(arena_t *a, unsigned n) {
    assert(a);
    // prevent int overflow.
    assert(n < a->total_nbytes);

    n = roundup(n,8);
    assert(n%8==0);

    void *ptr = a->cur;
    void *new_cur = (char*)ptr + n;
    if(new_cur >= a->end)
        panic("out of space: can't allocated %d bytes, have %d left, out of %d!\n", 
            n, ptr_diff(a->end, a->cur), a->total_nbytes);
    
    a->cur = new_cur;
    return ptr;
}

static inline void *
arena_alloc(arena_t *a, unsigned n) {
    void *ptr = arena_alloc_notzero(a,n);
    memset(ptr, 0, n);
    return ptr;
}

static inline void 
arena_reset(arena_t *a) {
    a->cur = a->base;
}


#if 0
static inline void 
arena_free_all(arena_t *a) {
    // doesn't work.
    assert(a);
    memset(a->base, 0xfa, a->total_nbytes);
    kr_free(a->base);
    
    memset(a, 0xfa, sizeof *a);
    kr_free(a);
}
#endif
#endif
