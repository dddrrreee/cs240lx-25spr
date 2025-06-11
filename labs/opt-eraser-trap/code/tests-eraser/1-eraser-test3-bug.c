// engler: should get an error b/c the new thread has no lock.
#include "eraser.h"

// has the lock/unlock etc implementation.
#include "fake-thread.h"

static int l,l2;

void notmain() {

    eraser_init();
    eraser_set_thread_id(1);
    int *x = kmalloc(4);

    lock(&l);
    put32(x,0x12345678);   // should be fine.

    unlock(&l);

    eraser_set_thread_id(2);

    trace("should be EXCLUSIVE: addr=%p, state=%s\n", x, eraser_state_s(x));
    eraser_expect(x, SH_EXCLUSIVE);

    trace("should have an error because b/c second thread touches w/o a lock\n");
    put32(x,0x12345678);   // bug b/c no lock held.

    trace("SUCCESS: load should pass\n");
}
