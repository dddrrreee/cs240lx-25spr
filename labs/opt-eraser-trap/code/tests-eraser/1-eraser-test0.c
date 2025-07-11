// engler: should have no error and be in EXCLUSIVE state
// (b/c no thread accesses)
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

    trace("should *not* have an error below b/c no other thread touches\n");

    trace("should be exclusive: addr=%p, state=%s\n", x, eraser_state_s(x));
    eraser_expect(x, SH_EXCLUSIVE);

    get32(x);    // not an error 
    get32(x);    // not an error 
    get32(x);    // not an error 
    get32(x);    // not an error 
    get32(x);    // not an error 
    get32(x);    // not an error 
    get32(x);    // not an error 
    get32(x);    // not an error 
    get32(x);    // not an error 

    trace("SUCCESS: load passed!\n");
}
