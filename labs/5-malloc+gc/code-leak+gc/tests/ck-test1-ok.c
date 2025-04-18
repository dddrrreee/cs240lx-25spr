#include "rpi.h"
#include "ckalloc.h"

void notmain(void) {
    trace("CK test: simple alloc and free\n");

    // allocate 1 block
    char *p = ckalloc(4);

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
