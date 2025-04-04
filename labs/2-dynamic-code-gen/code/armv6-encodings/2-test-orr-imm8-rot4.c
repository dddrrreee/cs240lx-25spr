// test for orr_imm8_rot4
#include "rpi.h"
#include "asm-helpers.h"
#include "bit-support.h"
#include "pi-random.h"
#include "armv6-encodings.h"

enum { ntrials = 1024 };

void notmain(void) {

    uint32_t code[12];
    uint32_t (*fp)(void) = (void*)code;

    reg_t lr = reg_mk(armv6_lr);
    reg_t r0 = reg_mk(0);

    output("about to run mov rd, imm8, rot4 tests\n");

    // exhaustively try all immediates and rotations.
    for(unsigned i = 0; i < 255; i++) {
        // rotation to the right.
        for(unsigned rot = 0; rot < 32; rot += 8) {
            code[0] = armv6_mov_imm8(r0, 0);
            // code[idx++] = armv6_orr_imm8_rot4(r0, r0,i, rot);
            code[1] = armv6_orr_imm8_rot4(r0, r0, i,rot);
            code[2] = armv6_bx(lr);
            prefetch_flush();

            uint32_t exp = i;
            // should just build a rotate?
            if(rot)
                exp = i << (32-rot);

            uint32_t got = fp();
            if(got != exp)
                panic("expected %d (%d<<%d): got %d\n", exp,i,rot,got);
        }
    }
    trace("SUCCESS: trials=%d: orr_imm8_rot4 seems to work\n", 255*4);
}
