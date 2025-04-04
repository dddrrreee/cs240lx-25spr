// do a bunch of timing tests on jit-dot.  make sure
// works before you do this; hard to debug.
#include "jit-dotproduct.h"

static void run_vector_test(
    unsigned ntrials, 
    unsigned n, 
    unsigned percent_0,
    int verbose_p) {

    // we'll have a free in a couple of labs.  
    uint32_t *a = vec_mk(n,0);
    uint32_t *b = vec_mk(n,percent_0);
    if(verbose_p) {
        vec_print("A", a,n);
        vec_print("B", b,n);
    }

    // should do a bunch of tests.
    // quick test.
    uint32_t d0 = vec_dot(a,b,n);

    vec_fn_t dot_fn = jit_dot(b,n);
    uint32_t d1 = dot_fn(a);

    if(d0 != d1)
        panic("static dot=%d, jit dot=%d\n", d0,d1);

    if(verbose_p) 
        output("passed: static dot = jit dot = %d\n", d0);

    output("--------------------------------------------\n");
    output("n=%d, percent zero=%d, ntrials=%d\n",
        n, percent_0, ntrials);

#if 0
    // use PMU to figure stuff out!
    pmu_t p = pmu_mk(PMU_ICACHE_MISS, PMU_BRANCH_MISPREDICT);
    for(int i = 0; i < ntrials; i++) {
        pmu_t p0 = PMU_MEASURE(p, d0 = vec_dot(a,b,n));
        pmu_t p1 = PMU_MEASURE(p, d1 = dot_fn(a));

        pmu_print("static:", p0);
        pmu_print("jit:", p1);
    }
#endif

    output("no cache:\n");
    for(int i = 0; i < ntrials; i++) {
        output("   static:\t%d cycles\n", 
            TIME_CYC(d0 = vec_dot(a,b,n)));
        output("   jit:\t\t%d cycles\n", 
            TIME_CYC(d1 = dot_fn(a)));
        assert(d0==d1);
    }


    caches_enable();

    output("cache enabled:\n");
    for(int i = 0; i < ntrials; i++) {
        output("   static:\t%d cycles\n", 
            TIME_CYC(d0 = vec_dot(a,b,n)));
        output("   jit:\t\t%d cycles\n", 
            TIME_CYC(d1 = dot_fn(a)));
        assert(d0==d1);
    }
}

void notmain(void) {
    jit_init();

    enum { trials = 2 };
    for(unsigned n = 32; n < 1024; n *= 4) {
        for(unsigned percent_0 = 0; percent_0 < 100;  percent_0 += 15) {
            run_vector_test(trials, n, percent_0,0);
            jit_free_all();
        }
    }
}
