#include "rpi.h"
#include "asm-helpers.h"
#include "bit-support.h"
#include "pi-random.h"
#include "armv6-encodings.h"

enum { ntrials = 10 };

void notmain(void) {
    reg_t lr = reg_mk(armv6_lr);
    uint32_t code[12];
    uint32_t (*fp)(uint32_t) = (void*)code;

    code[0] = armv6_bx(lr);

    // test that things work.
    for(unsigned i = 0; i < ntrials; i++) {
        uint32_t x = pi_random();

        uint32_t got = fp(x);

        if(x != got)
            panic("expected=%x, got=%x\n", x,got);
    }
    trace("SUCCESS: trials=%d: bx(lr) seems to work\n", ntrials);
}
