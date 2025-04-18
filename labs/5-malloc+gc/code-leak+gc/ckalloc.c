// implement a simple ckalloc/free that adds ckhdr_t to the 
// allocation.
#include "rpi.h"
#include "ckalloc.h"
#include "kr-malloc.h"

unsigned ck_verbose_p = 0;

// should hold all allocated blocks
static hdr_t *alloc_list; 

// returns pointer to the first allocated header block.
hdr_t *ck_first_alloc(void) {
    return alloc_list;
}

// return header associated with <ptr> if one exists.
hdr_t *ck_ptr_is_alloced(void *ptr) {
    for(hdr_t *h = ck_first_alloc(); h; h = ck_next_hdr(h))
        if(ck_ptr_in_block(h,ptr)) 
            return h;
    return 0;
}


/***********************************************************************
 * implement the rest
 */

// is <ptr> inside <h>'s data block?
unsigned ck_ptr_in_block(hdr_t *h, void *ptr) {
    if(h->state != ALLOCED) {
        panic("should only have allocated blocks: sdtate=%d\n", h->state);
        return 0;
    }

    // use ck_data_start/_end 
    todo("check that <ptr> is in data for <h>\n");
}


// free a block allocated with <ckalloc>
void (ckfree)(void *addr, src_loc_t l) {
    hdr_t *h = ck_ptr_is_alloced(addr);
    if(!h)
        loc_panic(l, "freeing bogus pointer: %p\n", addr);

    // allocated block starts right after the header.

    void *blk_start = ck_data_start(h);
    if(blk_start != addr)
        loc_panic(l, "not freeing using start pointer: have %p, need %p\n",
            addr, blk_start);

    if(h->state != ALLOCED)
        loc_panic(l, "freeing unallocated memory: state=%d\n", h->state);
    if(ck_verbose_p)
        loc_debug(l, "freeing %p\n", addr);

    assert(ck_ptr_is_alloced(addr));
    h->state = FREED;

    // just remove from the allocated list.
    todo("implement the rest\n");

    kr_free(h);
}


// interpose on kr_malloc allocations and
//  1. allocate enough space for a header and fill it in.
//  2. add the allocated block to  the allocated list.
void *(ckalloc)(uint32_t nbytes, src_loc_t l) {
    static unsigned block_id=1;

    hdr_t *h = kr_malloc(nbytes + sizeof *h);

    memset(h, 0, sizeof *h);
    h->nbytes_alloc = nbytes;
    h->state = ALLOCED;
    h->alloc_loc = l;
    h->block_id = block_id++;

    // set addr;
    void *addr = 0;

    todo("put on allocated list\n");

    assert(ck_ptr_is_alloced(addr));
    if(ck_verbose_p)
        loc_debug(l, "successful alloc of %p\n", addr);
    return addr;
}
