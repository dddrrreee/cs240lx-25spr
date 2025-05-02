#include "rpi.h"
#include "ss-pixie.h"
#include "breakpoint.h"

// return the current stack pointer.
static inline void *armv6_get_sp(void) {
    void *sp;
    asm volatile("mov %0, sp" : "=r"(sp));
    return sp;
}

__attribute__((noinline))  
void test(void) {
    void nop_10(void);

    trace("ss pixie start\n");

    // should emit traces for each pc
    pixie_start();
    nop_10();
    pixie_stop();

    trace("ss pixie stop\n");
}

void notmain(void) {
    pixie_verbose(0);

    for(int i = 0; i < 10; i++) {
        // trivial test that the stack pointer gets set
        // back after disabling pixie tracing
        let sp1 = armv6_get_sp();
        gcc_mb();  // stop gcc code movement

        // run the test: disabled inlining, so 
        // when returns the stack pointer should be
        // the same.
        test();
    
        gcc_mb();  // stop gcc code movement
        let sp2 = armv6_get_sp();

        output("sp1=%x, sp2=%x\n", sp1,sp2);
        assert(sp1==sp2);
        trace("SUCCESS: %d!\n",i);
    }

}

