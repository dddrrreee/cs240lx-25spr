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


    pmu_stmt_measure(msg, 
            inst_cnt, 
            inst_stall, 
    {
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
    });
}

void notmain(void) {
    output("---------------------------------------------------\n");
    for(int i = 0; i < 3; i++)
        measure_nops("no cache",i);

    caches_enable();
    output("---------------------------------------------------\n");

    for(int i = 0; i < 3; i++)
        measure_nops("cache",i);
}
