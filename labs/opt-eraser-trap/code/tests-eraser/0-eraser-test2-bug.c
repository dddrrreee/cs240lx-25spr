// note: this is *not* a bug in the fancy eraser.
#include "eraser.h"

// has the lock/unlock etc implementation.
#include "fake-thread.h"

static int l;

void notmain() {
    eraser_init();

    // if you want to see transitions, change to 1:
    //  eraser_verbose_set(1);
    eraser_verbose_set(0);

    eraser_set_thread_id(1);
    int *x = kmalloc(4);

    const char *s = eraser_state_s(x);
    trace("should be virgin: addr=%p, state=%s\n", x, s);
    eraser_expect(x, SH_VIRGIN);

    trace("--------------------------------------------------\n");
    trace("expect a store error at pc=%p, addr=%p\n", put32, x);
    trace("--------------------------------------------------\n");
    put32(x,0x12345678);   // trivial error.

    trace("should be exclusive: addr=%p, state=%s\n", x, eraser_state_s(x));
    eraser_expect(x, SH_EXCLUSIVE);

    // this isn't true.
    panic("should have caught unprotected store error\n");

}
