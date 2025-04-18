#include "rpi.h"
#include "ckalloc.h"

void alloc_one(unsigned nbytes) {
    // allocate 1 block
    char *p = ckalloc(nbytes);

    // should be allocated!
    let h = ck_ptr_is_alloced(p);
    if(!h)
        panic("we just allocated %p\n", p);
    let blk = h->block_id;
    trace("allocated [%p] blockid=%d, is allocated\n", p, blk);

    // check: p is in h
    if(!ck_ptr_in_block(h, p))
        panic("impossible: %p not in its block\n", p);

    ckfree(p);
    if(ck_ptr_is_alloced(p))
        panic("we just allocated %p but is free?\n", p);

    trace("SUCCESS: [%p] blockid=%d, is free!\n", p, blk);
}

void notmain(void) {
    trace("CK test: simple alloc and free\n");

    enum { N = 10 };

    for(int i = 0; i < N; i++) 
        alloc_one(i+1);

    trace("SUCCESS: alloc/free %d blocks\n", N);
}
