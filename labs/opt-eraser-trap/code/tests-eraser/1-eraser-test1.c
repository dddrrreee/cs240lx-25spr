// engler: should have no error and be in SHARED state.
#include "eraser.h"

// has the lock/unlock etc implementation.
#include "fake-thread.h"

static int l;

void notmain() {

    eraser_init();
    eraser_set_thread_id(1);
    int *x = kmalloc(4);

    lock(&l);
    put32(x,0x12345678);   // should be fine.

    unlock(&l);

    trace("should *not* have an error because no other thread touches\n");

    trace("should be exclusive: addr=%p, state=%s\n", x, eraser_state_s(x));
    eraser_expect(x, SH_EXCLUSIVE);

    get32(x);    // not an error 
    get32(x);    // not an error 

    // second thread access
    eraser_set_thread_id(2);
    eraser_expect(x, SH_EXCLUSIVE);
    get32(x);    // not an error 

    trace("should be SHARED: addr=%p, state=%s\n", x, eraser_state_s(x));
    eraser_expect(x, SH_SHARED);

    get32(x);    // not an error 
    get32(x);    // not an error 

    // should still be in shared b/c no writes.
    eraser_expect(x, SH_SHARED);

    // 3rd thread access
    eraser_set_thread_id(3);
    get32(x);    // not an error 
    get32(x);    // not an error 
    get32(x);    // not an error 
    // should still be in shared b/c no writes.
    eraser_expect(x, SH_SHARED);

    // back to original 1st thread 
    eraser_set_thread_id(1);
    get32(x);    // not an error 
    // should still be in shared b/c no writes.
    eraser_expect(x, SH_SHARED);


    trace("SUCCESS: load should pass\n");
}
