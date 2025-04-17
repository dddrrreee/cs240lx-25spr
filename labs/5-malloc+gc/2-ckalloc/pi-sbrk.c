#include "kr-malloc.h"

void *sbrk(long increment) {
    static int init_p;

    assert(increment > 0);
    if(!init_p) {
        kmalloc_init_set_start((void*)0x100000, 0x100000);
        init_p = 1;
    }
    return kmalloc(increment);
}
