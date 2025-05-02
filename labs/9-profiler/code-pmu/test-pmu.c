#include "rpi.h"
#include "armv6-pmu.h"

// fill this in.
enum {
    PMU_INST_CNT = 0x7,
};

void notmain(void) {
    pmu_enable0(PMU_INST_CNT);
    assert(pmu_type0() == PMU_INST_CNT);

    uint32_t inst_s = pmu_event0_get();
    uint32_t inst_e = pmu_event0_get();
    output("empty instructions= %d\n", inst_e - inst_s);

    inst_s = pmu_event0_get();
    asm volatile("nop"); // 1
    asm volatile("nop"); // 2
    asm volatile("nop"); // 3
    asm volatile("nop"); // 4
    asm volatile("nop"); // 5

    asm volatile("nop"); // 1
    asm volatile("nop"); // 2
    asm volatile("nop"); // 3
    asm volatile("nop"); // 4
    asm volatile("nop"); // 5

    inst_e = pmu_event0_get();

    // we expect to see 10 instructions.
    output("instructions= %d\n", inst_e - inst_s);
}
