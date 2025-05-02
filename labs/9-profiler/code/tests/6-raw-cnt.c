// get the raw cycle counts for nops.
#include "rpi.h"
#include "cycle-count.h"

void nop_10(void);

__attribute__((noinline))  
void test(const char *msg) {
    output("measurig: <%s>\n", msg);

    uint32_t b,e,overhead;

    // 16-byte prefetch buffer fetch.
    asm volatile(".align 4");
    b = cycle_cnt_read();
    e = cycle_cnt_read();
    output("only measurment overhead = %d\n", e-b);

    asm volatile(".align 4");
    b = cycle_cnt_read();
    asm volatile("nop");    // 4 bytes
    e = cycle_cnt_read();
    output("overhead of one nop = %d\n", e-b);

    asm volatile(".align 4");
    b = cycle_cnt_read();
    asm volatile("nop");    // 4 bytes
    asm volatile("nop");    // 4 bytes
    e = cycle_cnt_read();
    output("overhead of two nops = %d\n", e-b);
}

void notmain(void) {
    test("uncached");
    caches_enable();
    test("cached cold");
    test("cached warmed-up 1");
    test("cached warmed-up 2");
}
