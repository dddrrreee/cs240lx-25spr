// simple use-after free: allocate, free, write one byte. 
// should get an error.
#include "rpi.h"
#include "ckalloc.h"

void notmain(void) {
    printk("test1\n");

    char *p = ckalloc(4);
    trace("alloc returned %u [%p]\n", ck_blk_id(p), p);
    ckfree(p);
    *p = 1;

    if(!ck_heap_errors())
        panic("missed-error!\n");
    else
        trace("SUCCESS found error\n");
}
