#ifndef __MEMTRACE_H__
#define __MEMTRACE_H__
#include "rpi.h"
#include "switchto.h"   // needed for <reg_t>

enum { MEMTRACE_OK = 1 };

// fault context: passed to the handler on each memory fault.
// pro:
//    - using a structure lets us add additional information later 
//      without rewriting the checker.
// con:
//    - adds complexity.  not great for lab :/
typedef struct fault_ctx {
    // maybe have separate <pre:regs> and <post:regs>
    regs_t *r;              // registers for fault (16 general + cpsr)
                            // sufficient for <switchto>.
    uint32_t pc;            // same as <pre:r->regs[15]>.  won't 
                            // be the same for <post>

    // the following are the same for pre/post.
    uint32_t addr;          // memory address of fault.
    unsigned nbytes;        // number of bytes of access [note: you'll need
                            // to implement this calculation]
    unsigned load_p:1;      // access = load (=1), or store (=0).
} fault_ctx_t;

static inline fault_ctx_t
fault_ctx_mk(regs_t *r, uint32_t addr, unsigned nbytes, int load_p) {
    return (fault_ctx_t) { 
        .r = r, 
        .pc = r->regs[15],
        .addr = addr, 
        .nbytes = nbytes, 
        .load_p = load_p
    };
}

// client handler type: when a fault happens, gets called with:
//  1. <handle>: client supplied pointer that gets passed to each handler
//     invocation.
//  2. <c>: the fault context.
// probably should return different action codes: right now we always
// return <MEMTRACE_OK> as a placeholder.
typedef int (*memtrace_fn_t)(void *handle, fault_ctx_t *c);

// step 1: initialize the system.  
//  must specify:
//    1. the trapping domain (not hard to change to multiple)
//    2. at least one <pre> or <post> handler to call before / after 
//       any trapping memory instruction.
//  note: 
//    - virtual memory must already be setup.
//    - we don't correctly handle executing code in trapping memory
//      would require some finicky changes.
//    - in a adult version you'd be able to: 
//         1. override different pieces.
//         2. have multiple checkers.
//         3. delete checkers.
//      all good to build for yours!
void memtrace_init(
    void *data,           
    memtrace_fn_t pre, 
    memtrace_fn_t post, 
    unsigned trap_dom);

// step 2: turn trapping off/on.
//   note: should probably make it so you can recursively turn 
//   off.
// trapping on
void memtrace_trap_enable(void);
// trapping off
void memtrace_trap_disable(void);

// debug support: turn yapping off/on.  
//   - should provide some more options.
void memtrace_yap_off(void);
void memtrace_yap_on(void);

#endif
