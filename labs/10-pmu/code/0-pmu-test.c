// simple PMU test: check that cycle counter works.
#include "rpi.h"
#include "rpi-pmu.h"

__attribute__((noinline)) 
void
measure_nops(const char *msg, int n) {
    uint32_t cyc_s, cyc_e;

    // Oh: interesting.  if you uncomment:
    //  - Q: "why do the timings slow down?"
    //  - Q: "how would you fix?"
    // asm volatile(".align 4");
    cyc_s       = pmu_cycle_get();      // always on cycle counter

    asm volatile("nop");  // 1
    asm volatile("nop");  // 2
    asm volatile("nop");  // 3
    asm volatile("nop");  // 4
    asm volatile("nop");  // 5

    asm volatile("nop");  // 1
    asm volatile("nop");  // 2
    asm volatile("nop");  // 3
    asm volatile("nop");  // 4
    asm volatile("nop");  // 5

    asm volatile("nop");  // 1
    asm volatile("nop");  // 2
    asm volatile("nop");  // 3
    asm volatile("nop");  // 4
    asm volatile("nop");  // 5

    cyc_e       = pmu_cycle_get();      // always on cycle counter

    output("%d:%s: total cyc=%d\n",
        n,
        msg,
        cyc_e-cyc_s);
}

void notmain(void) {
    // hack to check that PMU enable/disable working.
    pmu_off();
    uint32_t s = pmu_cycle_get();
    for(int i = 0; i < 100; i++) {
        uint32_t e = pmu_cycle_get();
        if(s != e)
            output("cycle counter changed when disabled\n");
    }

    // now it should work.
    pmu_on();

    output("-------------no caching---------------------------------\n");
    for(int i = 0; i < 10; i++)
        measure_nops("no cache",i);

    // enable cache.
    caches_enable();

    output("------------icache bt on-------------------------------\n");
    for(int i = 0; i < 5; i++)
        measure_nops("cache",i);
}
