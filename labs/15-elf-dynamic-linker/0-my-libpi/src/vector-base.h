// once this works, move it to: 
//    <libpi/include/vector-base.h>
// and make sure it still works.
#ifndef __VECTOR_BASE_SET_H__
#define __VECTOR_BASE_SET_H__
#include "libc/bit-support.h"
#include "asm-helpers.h"

/*
 * vector base address register:
 *   arm1176.pdf:3-121 --- lets us control where the 
 *   exception jump table is!  makes it easy to switch
 *   tables and also make exceptions faster.
 *
 * defines: 
 *  - vector_base_set  
 *  - vector_base_get
 */

// Macro function definitions (makes two inline functions that involve asm)
// "secure_or_non_secure_vector_base_address" refer to c12 register of CP15 (in 3-121 of arm1176.pdf)
cp_asm_get(secure_or_non_secure_vector_base_address, p15, 0, c12, c0, 0);
cp_asm_set(secure_or_non_secure_vector_base_address, p15, 0, c12, c0, 0);

// return the current value vector base is set to.
static inline void *vector_base_get(void) {
    return (void*)secure_or_non_secure_vector_base_address_get();
}

// check that not null and alignment is good.
static inline int vector_base_chk(void *vector_base) {
    if(!vector_base)
        return 0;
    // todo("check alignment is correct: look at the instruction def!");
    return vector_base && (((int)vector_base & 0b11111) == 0);
}

// set vector base: must not have been set already.
static inline void vector_base_set(void *vec) {
    if(!vector_base_chk(vec))
        panic("illegal vector base %p\n", vec);

    void *v = vector_base_get();
    // if already set to the same vector, just return.
    if(v == vec)
        return;

    if(v) 
        panic("vector base register already set=%p\n", v);

    // todo("set vector base here.");
    secure_or_non_secure_vector_base_address_set((uint32_t)vec);

    // double check that what we set is what we have.
    v = vector_base_get();
    if(v != vec)
        panic("set vector=%p, but have %p\n", vec, v);
}

// set vector base to <vec> and return old value: could have
// been previously set (i.e., non-null).
static inline void *
vector_base_reset(void *vec) {
    void *old_vec = 0;

    if(!vector_base_chk(vec))
        panic("illegal vector base %p\n", vec);

    // todo("get old vector base, set new one\n");
    old_vec = vector_base_get();
    secure_or_non_secure_vector_base_address_set((uint32_t)vec);

    // double check that what we set is what we have.
    assert(vector_base_get() == vec);
    return old_vec;
}
#endif
