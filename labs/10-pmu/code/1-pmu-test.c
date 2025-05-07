// simple PMU test: measure cycles, instructions and stalls
// using the raw interface.
#include "rpi.h"
#include "rpi-pmu.h"

__attribute__((noinline)) 
void
measure_nops(const char *msg, int n) {
    uint32_t cyc_s, cyc_e;
    uint32_t inst0_s, inst0_e;
    uint32_t stall1_s, stall1_e;

    // enable the two events.
    pmu_enable(0, PMU_inst_cnt);
    pmu_enable(1, PMU_inst_stall);

    cyc_s       = pmu_cycle_get();      // always on cycle counter
    inst0_s     = pmu_event_get(0);     // instruction count
    stall1_s    = pmu_event_get(1);     // stalls

    asm volatile("nop");  // 1
    asm volatile("nop");  // 2
    asm volatile("nop");  // 3
    asm volatile("nop");  // 4
    asm volatile("nop");  // 5

    cyc_e       = pmu_cycle_get();      // always on cycle counter
    inst0_e     = pmu_event_get(0);     // instruction count
    stall1_e    = pmu_event_get(1);     // stalls

    output("%d:%s: total cyc=%d, tot inst=%d, tot stalls=%d\n",
        n,
        msg,
        cyc_e - cyc_s,
        inst0_e - inst0_s,
        stall1_e - stall1_s);
}

void notmain(void) {
    output("---------------------------------------------------\n");
    for(int i = 0; i < 10; i++)
        measure_nops("no cache",i);

    caches_enable();
    output("---------------------------------------------------\n");

    for(int i = 0; i < 5; i++)
        measure_nops("cache",i);
}
