#ifndef __POINTER_H__
#define __POINTER_H__
// trivial helpers that make code a bit cleaner.

#ifndef RPI_UNIX
// XXX: disk offset should be u64_t
typedef uint32_t off_t;   // offset
#else
#include <stddef.h>
#endif

// b - a;
static inline off_t ptr_diff(const void *b, const void *a) {
    assert(b >= a);
    return (const char *)b - (const char *)a;
}

// a + off
static inline const void *ptr_add(const void *a, unsigned off) {
    return (const char *)a + off;
}
static inline void *ptr_add_mut(void *a, unsigned off) {
    return (char *)a + off;
}

#if 0
#define CLONE(xx) \
    ({ typeof(xx) *tmp = kalloc(sizeof *tmp); *tmp = (xx); tmp; })
#endif


static inline uint32_t min_u32(uint32_t a, uint32_t b) {
    return a < b ? a : b;
}

static inline unsigned roundup(unsigned x, unsigned n) {
    return (x+(n-1)) & (~(n-1));
}
#endif
