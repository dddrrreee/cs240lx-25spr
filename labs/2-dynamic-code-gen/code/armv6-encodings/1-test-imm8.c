// test for mov_imm8
#include "rpi.h"
#include "asm-helpers.h"
#include "bit-support.h"
#include "pi-random.h"
#include "armv6-encodings.h"

enum { ntrials = 10 };

void notmain(void) {

    uint32_t code[12];
    uint32_t (*fp)(void) = (void*)code;

    reg_t lr = reg_mk(armv6_lr);
    reg_t r0 = reg_mk(0);

    // generate a simple routine to return an 8bit
    // constant.  relies on no icache being enabled.
    for(unsigned i = 0; i < ntrials; i++) {
        uint8_t imm8 = pi_random();

        code[0] = armv6_mov_imm8(r0, imm8);
        code[1] = armv6_bx(lr);
        prefetch_flush();

        uint32_t got = fp();
        if(imm8 != got)
            panic("expected=%x, got=%x\n", imm8, got);
    }
    trace("SUCCESS: trials=%d: mov_imm8 seems to work\n", ntrials);
}
