// test for load_imm32: load a 32-bit unsigned constant.
#include "rpi.h"
#include "asm-helpers.h"
#include "bit-support.h"
#include "pi-random.h"
#include "armv6-encodings.h"

enum { ntrials = 4096 };

void notmain(void) {

    uint32_t code[12];
    uint32_t (*fp)(void) = (void*)code;

    reg_t lr = reg_mk(armv6_lr);
    reg_t r0 = reg_mk(0);

    // generate a simple routine to return an 8bit
    // constant.  relies on no icache being enabled.
    for(unsigned i = 0; i < ntrials; i++) {
        uint32_t *cp = code;
        uint32_t imm32 = pi_random();

        cp = armv6_load_imm32(cp, r0, imm32);
        *cp++ = armv6_bx(lr);
        prefetch_flush();

        uint32_t got = fp();
        if(imm32 != got)
            panic("expected=%x, got=%x\n", imm32, got);
    }
    trace("SUCCESS: trials=%d: load_imm32 seems to work\n", ntrials);
}
