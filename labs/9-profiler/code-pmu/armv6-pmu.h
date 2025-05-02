#ifndef __ARMV6_PMU_H__
#define __ARMV6_PMU_H__
#include "bit-support.h"
#include "asm-helpers.h"

// implement these.

// get the type of event0 by reading the type
// field from the PMU control register and 
// returning it.
static inline uint32_t pmu_type0(void) {
    todo("get type of event 0");
}

// set PMU event0 as <type> and enable it.
static inline void pmu_enable0(uint32_t type) {
    todo("enable event 0 for <type>");
    assert(pmu_type0() == type);
}

// get current value for event 0 
static inline uint32_t pmu_event0_get(void) {
    todo("get value for 0");
}

// get the type of event1 by reading the type
// field from the PMU control register and 
// returning it.
static inline uint32_t pmu_type1(void) {
    todo("get type of event 1");
}

// set event1 as <type> and enable it.
static inline void pmu_enable1(uint32_t type) {
    assert((type & 0xff) == type);
    todo("enable event 1 for <type>");
    assert(pmu_type1() == type);
}

static inline uint32_t pmu_event1_get(void) {
    todo("get value for event 1");
}





// wrapper so can pass in the PMU register number.
static inline void pmu_enable(unsigned n, uint32_t type) {
    if(n==0)
        pmu_enable0(type);
    else if(n == 1)
        pmu_enable1(type);
    else
        panic("bad PMU coprocessor number=%d\n", n);
}

// wrapper so can pass in the PMU register number.
static inline uint32_t pmu_event_get(unsigned n) {
    if(n==0)
        return pmu_event0_get();
    else if(n == 1)
        return pmu_event1_get();
    else
        panic("bad PMU coprocessor number=%d\n", n);
}
#endif
