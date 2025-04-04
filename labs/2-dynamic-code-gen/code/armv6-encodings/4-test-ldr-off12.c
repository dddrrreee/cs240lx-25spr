// test for mov_imm8
#include "rpi.h"
#include "asm-helpers.h"
#include "bit-support.h"
#include "pi-random.h"
#include "armv6-encodings.h"

enum { ntrials = 512 };

// flip this if you want debug output.
#if 0
#   define enc_debug(args...) debug(args)
#else
#   define enc_debug(args...) do { } while(0)
#endif

static int ptr_diff(const void *a, const void *b) {
    return (const char *)a - (const char *)b;
}


void notmain(void) {
    output("testing ldr instructions\n");

    // we put a bunch of constants above and below and make
    // sure they can get loaded correctly.
    enum { MAXCONST = 512 };
    uint32_t constants[MAXCONST];
    uint32_t code[12];
    uint32_t constants2[MAXCONST];

#   define nelem(x) (sizeof (x) / sizeof (x[0]))
    for(int i = 0; i < nelem(constants); i++)
        constants2[i] = constants[i] = pi_random();

    uint32_t (*fp)(void) = (void*)code;

    reg_t lr = reg_mk(armv6_lr);
    reg_t r0 = reg_mk(0);
    reg_t pc = reg_mk(armv6_pc);

    // generate a routine to load a constant into 
    // r0 using pc as a base register and return.
    for(unsigned i = 0; i < ntrials; i++) {
        int offset = ptr_diff(&constants[i],&code[0]);

        enc_debug("offset=%d, at that point=%d\n",
            offset, code[offset/4]);
        offset -= 8;    // pc is 8 bytes beyond.
        enc_debug("offset after = %d\n", offset);

        code[0] = armv6_ldr_off12(r0, pc, offset);
        code[1] = armv6_bx(lr);
        prefetch_flush();

        uint32_t expect = constants[i];
        uint32_t got = fp();
        if(got != expect)
            panic("expected %x: got %x\n", expect,got);

        offset = ptr_diff(&constants2[i], &code[0]);
        enc_debug("offset=%d, at that point=%d\n",
            offset, code[offset/4]);
        offset -= 8;    // pc is 8 bytes beyond.
        code[0] = armv6_ldr_off12(r0, pc, offset);
        code[1] = armv6_bx(lr);
        prefetch_flush();

        expect = constants2[i];
        got = fp();
        if(got != expect)
            panic("expected %x: got %x\n", expect,got);
    }
    trace("SUCCESS: trials=%d: ldr_off12 seems to work\n", ntrials);
}
