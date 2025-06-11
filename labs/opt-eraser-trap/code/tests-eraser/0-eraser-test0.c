// engler: doesn't do anything interesting: just checks translation 
// and mmu on/off as expected.
#include "eraser.h"

#include "mmu.h"

int x = 0x12345678;

void notmain(void) {
    trace("check that we are running at in superuser mode\n");

    assert(!mmu_is_enabled());
    eraser_init();
    eraser_verbose_set(1);

    assert(mmu_is_enabled());
    assert(mode_eq(SUPER_MODE));
    //make sure translation works.
    assert(x == 0x12345678);

    // int *y = eraser_alloc(4);
    int *y = kmalloc(4);
    *y = 1;
    assert(*y == 1);

    memtrace_trap_disable();
    // still on, just don't trap.  [not that intuitive]
    assert(mmu_is_enabled());

    trace("trivial asserts should pass\n");
}
