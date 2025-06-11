// engler: do one safe store and one unsafe load.
#include "eraser.h"

// has the lock/unlock etc implementation.
#include "fake-thread.h"

static int l;

void notmain() {
    eraser_init();
    // if you want to see transitions, change to 1:
    eraser_verbose_set(0);

    eraser_set_thread_id(1);

    lock(&l);
    int *x = kmalloc(4);
    put32(x,0x12345678);   // should be fine.
    unlock(&l);

    trace("--------------------------------------------------\n");
    trace("expect a load error at pc=%p, addr=%p\n", get32, x);
    trace("--------------------------------------------------\n");
    trace("this store should be an error\n");
    put32(x,0x12345678);   

    panic("should have caught unprotected access error\n");

}
