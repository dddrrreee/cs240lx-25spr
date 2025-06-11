#ifndef __ERASER_H__
#define __ERASER_H__

#include "src-loc.h"

void eraser_init(void);

// just for the debug macro.
#include "memtrace.h"


// eraser states.
enum {
    SH_INVALID     = 1 << 0,
    SH_VIRGIN     = 1 << 1,      // writeable, allocated
    SH_FREED       = 1 << 2,
    SH_SHARED = 1 << 3,
    SH_EXCLUSIVE   = 1 << 4,
    SH_SHARED_MOD      = 1 << 5,
};


static inline const char *
eraser_state_str(int state) {
    switch(state) {
    case SH_INVALID:    return "<INVALID>";
    case SH_VIRGIN:     return "<VIRGIN>";
    case SH_FREED:      return "<FREED>";
    case SH_SHARED:     return "<SHARED>";
    case SH_EXCLUSIVE:  return "<EXCLUSIVE>";
    case SH_SHARED_MOD: return "<SHARED-MOD>";
    default: panic("invalid state %d\n", state);
    }
}

extern int eraser_v_p;
static inline void eraser_verbose_set(int v_p) {
    eraser_v_p = v_p;
    if(v_p)
        memtrace_yap_on();
    else
        memtrace_yap_off();
}

// different levels of eraser.
enum { ERASER_TRIVIAL = 1,
    ERASER_SHARED_EX = 2,
    ERASER_SHARED = 3,
    ERASER_HIGHEST = ERASER_SHARED
};

// Tell eraser that [addr, addr+nbytes) is allocated (mark as 
// Virgin).
void eraser_mark_alloc(void *addr, unsigned nbytes);

// Tell eraser that [addr, addr+nbytes) is free (stop tracking).
// We don't try to catch use-after-free errors.
void eraser_mark_free(void *addr, unsigned nbytes);

// mark bytes [l, l+nbytes) as holding a lock.
void eraser_mark_lock(void *l, unsigned nbytes);

// indicate that we acquired/released lock <l>
void eraser_lock(void *l);
void eraser_unlock(void *l);

// indicate that we are running thread <tid>
void eraser_set_thread_id(int tid);

/*****************************************************************
 * helpful debug routines for test cases.
 */

// get state associated with <addr>
int eraser_state(void *addr);
static inline const char *eraser_state_s(void *addr) {
    return eraser_state_str(eraser_state(addr));
}

// return 1 if <addr.state> = <state>
static inline int eraser_match(void *addr, int state) {
    return state == eraser_state(addr);
}

#include "src-loc.h"

static inline void 
eraser_expect_fn(src_loc_t loc, void *addr, int state) {
    if(eraser_match(addr, state))
        return;

    // didn't match, get the states and emit error.
    const char *exp = eraser_state_str(state);
    const char *got = eraser_state_str(eraser_state(addr));

    output("%s:%s:%d:ERROR: expected state %s, have %s\n",
        loc.file, loc.func, loc.lineno, 
        exp, got);
    clean_reboot();
}

// hack to print the file and line.
#define eraser_expect(addr, state) \
    eraser_expect_fn(SRC_LOC_MK(), addr, state)

#endif
