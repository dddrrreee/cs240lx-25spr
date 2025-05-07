// single header file code to setup/use the armv6 performance
// monitor unit (PMU).  note: the r/pi does not propagate
// the PMU interrupt, unfortunately, so there is no way to 
// do a sampling profiler by using it.
//
// come for the PMU, stay for the cpp tricks...
#ifndef __RPI_PMU_H__
#define __RPI_PMU_H__

#include "bit-support.h"
#include "asm-helpers.h"

// use the <cp_asm> macro to define the different 
// get/set PMU instructions.
cp_asm(pmu_cycle, p15, 0, c15, c12, 1)

// you should fill in some additional ones.

// turn PMU on (enables cycle counter)
//
// NOTE: <cstart.c> turns the PMU on by default, 
// so you don't really need to ever call this
// in regular code.
static inline void pmu_on(void) {
    pmu_control_set(1);
}

// turn PMU off.
//
// afaik there is no real reason to do so ---
// we just use this routine to test your 
// <pmu_control_set>
static inline void pmu_off(void) {
    pmu_control_set(0);
}


// p 3-134 arm1176.pdf: configure the PMU control register
static inline void pmu_control_config(uint32_t in) {
    todo("implement");

    pmu_control_set(in);
}

// get the type of event0 by reading the type
// field from the PMU control register and 
// returning it.
static inline uint32_t pmu_type0(void) {
    todo("implement");
}

// set PMU event0 as <type> and enable it.
static inline void pmu_enable0(uint32_t type) {
    todo("implement");
    assert(pmu_type0() == type);
}

// get the type of event1 by reading the type
// field from the PMU control register and 
// returning it.
static inline uint32_t pmu_type1(void) {
    todo("implement");
}

// set event1 as <type> and enable it.
static inline void pmu_enable1(uint32_t type) {
    assert((type & 0xff) == type);
    todo("implement");

    assert(pmu_type1() == type);
}

// wrapper so can pass in the PMU register number.
// as long as <n> is a constant, all these checks
// should be eliminated at compile time.
static inline void pmu_enable(unsigned n, uint32_t type) {
    if(n==0)
        pmu_enable0(type);
    else if(n == 1)
        pmu_enable1(type);
    else
        panic("bad PMU coprocessor number=%d\n", n);
}

// wrapper so can pass in the PMU register number.
// as long as <n> is a constant, all these checks
// should be eliminated at compile time.
static inline uint32_t pmu_event_get(unsigned n) {
    if(n==0)
        return pmu_event0_get();
    else if(n == 1)
        return pmu_event1_get();
    else
        panic("bad PMU coprocessor number=%d\n", n);
}

// measure and print the count of <type0> and <type1>
// events when running the statements <stmts>.
//
// we could make it so it records the current
// and then restores it.  not sure if this 
// makes any sense.
#define pmu_stmt_measure_set(cnt0, cnt1, msg, type0, type1, stmts) do { \
    pmu_ ## type0 ## _on(0);                            \
    pmu_ ## type1 ## _on(1);                            \
    uint32_t ty0 = pmu_ ## type0(0);                    \
    uint32_t ty1 = pmu_ ## type1(1);                    \
    uint32_t cyc = pmu_cycle_get();                     \
                                                        \
    /* partially stop gcc from moving stuff */          \
    gcc_mb();                                           \
    stmts;                                              \
    gcc_mb();                                           \
                                                        \
    uint32_t n_ty0 = pmu_ ## type0(0) - ty0;            \
    uint32_t n_ty1 = pmu_ ## type1(1) - ty1;            \
    uint32_t n_cyc = pmu_cycle_get() -  cyc;            \
    const char *s0 = pmu_ ## type0 ## _str();           \
    const char *s1 = pmu_ ## type1 ## _str();           \
                                                        \
    cnt0 = n_ty0;                                       \
    cnt1 = n_ty1;                                       \
                                                        \
    output("%s:%d: %s:\n\t%d : cycles\n\t%d : %s\n\t%d: %s\n", \
        __FILE__, __LINE__, msg, n_cyc, cnt0,s0, cnt1,s1);              \
} while(0)

#define pmu_stmt_measure(msg, type0, type1, stmts) do {     \
    uint32_t c0 __attribute__((unused));                    \
    uint32_t c1 __attribute__((unused));                    \
    pmu_stmt_measure_set(c0,c1,msg, type0, type1, stmts);   \
} while(0)


// see 3-139 in arm1176.pdf
//
// good example of how you can define a table of attributes 
// and then use macros (passed in as XX) to select different 
// fields and do code generation.
#define PMU_DEFS(XX)                                                    \
    XX(ret_miss,        0x26, "return mispredicted")                    \
    XX(ret_hit,         0x25, "return predicted")                       \
    XX(ret_cnt,         0x24, "return count")                           \
    /* the addresss was pushed on the return stack.  */                 \
    /* there is also a 0x24 where it is popped off the */               \
    /* stack.  idk difference. */                                       \
    XX(call_cnt,        0x23, "call count")                             \
    XX(main_tlb_miss,   0x16, "TLB miss")                               \
    XX(wb_drain, 0x12, "writeback drained (stall?)")                    \
    /* data cache miss, noncacheable, write through. */                 \
    XX(data_access,     0x10, "explicit data access")                   \
    XX(tlb_miss,        0xF,  "main tlb miss")                          \
    /* i think this is all indirect jumps */                            \
    XX(pc_write,        0xD,  "sw changed pc [indir jump?]")            \
    XX(dcache_wb,       0xC,  "dcache write back")                      \
    XX(dcache_miss,     0xB,  "dcache miss")                            \
    XX(dcache_access,   0xA,  "dcache access")                          \
    XX(inst_cnt,        0x7,  "instruction count")                      \
    XX(branch_miss,     0x6,  "branch mispredict")                      \
    XX(branch_cnt,      0x5,  "branch executed")                        \
    XX(dtlb_miss,       0x4,  "data microtlb miss")                     \
    XX(itlb_miss,       0x3,  "instruction microtlb miss")              \
    XX(data_stall,      0x2,  "data dependency stall")                  \
    XX(inst_stall,      0x1,  "instruction stall [tlb/cache miss]")     \
    XX(icache_miss,     0x0,  "icache miss")                            \
    XX(cycle_cnt,       0xff, "cycle count")                            \
    /* is this for gpio etc? */                                         \
    XX(ld_st_unit,      0x11, "load store uniq req q")

// define enums using <PMU_DEFS>.
enum {
#   define PMU_ENUMS(name, val, string) PMU_ ## name = val,
    PMU_DEFS(PMU_ENUMS)
};

// generate the different event wrappers for setting
// reading.
#define PMU_FNS(fn_name,val,string)                         \
    /* enable event <type> */                               \
    static void pmu_ ## fn_name ## _on(unsigned n)          \
        { pmu_enable(n, val); }                             \
    /* read event <type> counter */                         \
    static uint32_t pmu_ ## fn_name(unsigned n)             \
        { return pmu_event_get(n); }                        \
    /* get printable string of event <type>*/               \
    static const char * pmu_ ## fn_name ## _str(void)       \
        { return string; }

PMU_DEFS(PMU_FNS)

#endif
