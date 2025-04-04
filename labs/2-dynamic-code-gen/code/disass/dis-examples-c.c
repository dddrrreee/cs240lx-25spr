// used to hold example C code that you want to see asm for.
#include "rpi.h"

uint32_t mov_example(uint32_t a1, uint32_t a2, uint32_t a3) { return a2+a3; }

void call_hello(void) {
    printk("hello world\n");
}

void notmain(void) {
}
