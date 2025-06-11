// engler: should have an error b/c wrong lock.
#include "eraser.h"

// has the lock/unlock etc implementation.
#include "fake-thread.h"

static int l1,l2;

void notmain() {

    eraser_init();
    eraser_set_thread_id(1);
    int *x = kmalloc(4);

    lock(&l1);
    put32(x,0x12345678);   // should be fine.
    unlock(&l1);

    trace("should be EXCLUSIVE: addr=%p, state=%s\n", x, eraser_state_s(x));
    eraser_expect(x, SH_EXCLUSIVE);

    eraser_set_thread_id(2);

    // goes to shared mod, but has a consistent lock so is ok.
    trace("should not get an error b/c consistent lock\n");
    lock(&l1);
    put32(x,0x12345678);   // should be fine.
    unlock(&l1);
    eraser_expect(x, SH_SHARED_MOD);

    trace("should get an error b/c we use a different lock\n");
    lock(&l2);
        put32(x,0x12345678);   // bug
    unlock(&l2);

    not_reached();
}
