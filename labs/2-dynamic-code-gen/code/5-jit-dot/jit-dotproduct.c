// generate a custom dot-product routine.
#include "jit-dotproduct.h"

#include "armv6-encodings.h"
#include "arena.h"

arena_t *jit_arena;


void jit_free_all(void) {
    void flush_caches(void);

    arena_reset(jit_arena);
    flush_caches();
}
void jit_init(void) {
    kmalloc_init(4);
    jit_arena = arena_mk(1024*1024);
}


// given in a vector <b> and generate a dot-product routine specialized
// to <b>'s -zero values: 
//    - hardcode's <b>'s values in the instruction stream;
//    - do not generate a multiply if the value is zero. 
//
// additional possible: replace multiply with shifts and adds.
//
// this is a simple example of "partial evaluation"
vec_fn_t jit_dot(uint32_t *b, unsigned n) {
    
    // gross: we don't know a-priori how much code we
    // need. max would be about 6 instructions * n
    unsigned n_inst = 10*n;
    uint32_t *code = calloc(n_inst, 4), 
            *cp = code, 
            *end = code+n_inst;

    // return address
    reg_t lr = reg_mk(armv6_lr);

    // the other vector <a> to use for the dot-product
    // is passed in as the first argument.
    reg_t a = reg_mk(0);

    // temporary registers; we use caller-saved.
    reg_t a_i = reg_mk(1);
    reg_t b_i = reg_mk(2);
    reg_t sum = reg_mk(12);

    // we can eliminate this if we special case the first load.
    cp = armv6_load_imm32(cp, sum, 0);

    // iterate over each non-zero and generate a multiply 
    // accumlate into sum.
    for(int i = 0; i < n; i++) {
        assert((cp + 10) < end);

        // skip zeros.
        if(b[i] == 0)
            continue;

        /* 
            1. set  a_i = a[i].  (this happens once)
            2. set b_i = b[i] (this happens each time the routine
               is called)
            3. use the multiply accumulate instruction.
         */
        todo("implement this code\n");
    }

    // can get rid of this by changing the last instruction.
    *cp++ = armv6_mov(reg_mk(0), sum);
    *cp++ = armv6_bx(lr);
    assert(cp<end);

    return (void*)code;
}

// look at the machine code to see if there are any additional
// tricks.
uint32_t vec_dot(uint32_t *a, uint32_t *b, int n) {
    uint32_t sum, k;
    for (sum = k = 0; k < n; k++)
        sum += a[k]*b[k];
    return sum;
}

