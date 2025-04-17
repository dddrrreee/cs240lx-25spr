// test that compaction works with doubly-linked list:
//  compact -- should find all bytes
//  compact again [should find 0]
//  check for leaks [should find 0]
#include "rpi.h"
#include "ckalloc.h"

// #define N 10
#define N 1

// allocate a linked list.  do a leak detection.
void *test(void) {
    struct list {
        int x; 
        struct list *next;
        struct list *prev;
    } *h = 0;

    for(int i = 0; i < N; i++) {
        struct list *e = ckalloc(sizeof *h);
        memset(e, 0, sizeof *e);
        e->x = 0;
        if(h) {
            assert(!h->prev);
            h->prev = e;
            e->next = h;
        }
        h = e;
    }
    // compiler optimizes this stuff away.
    // h= 0;
    // even this:
    //  memset(&h, 0, sizeof h);
    // perhaps if we put it in another file?
    trace("have a live pointer <h>: should not leak\n");
    check_no_leak();
    // return to try to get around compiler tricks.
    return h;
}

unsigned bar(void);

void notmain(void) {
    printk("GC test: checking that a doubly-linked list is handled.\n");

    // ck_verbose_set(0);
    // note: if we do a check after this, may miss b/c the value is laying around
    // on the stack or reg
    unsigned x = bar();

    // should detect leaks.
    trace("should leak\n");
    check_should_leak();

    // should compact everything: i don't know if this should be a trace --
    // the issue is that the compiler could leave some random values and it's
    // not under our control. 

    // some kind of corruption: compaction is trashing the output.
    // ck_verbose_set(1);

    trace("compacted = [nbytes=%d]\n", ck_gc());


    output("should find 0 bytes on second gc\n");
    unsigned n = ck_gc();
    if(n)
        panic("a second gc should free 0 bytes: found %d\n", n);

    trace("should find no leak\n");
    check_no_leak();
}

// mild attempt to make sure that the random values from <test> are 
// gone by the time we get back to main.
unsigned bar(void) {
#if 1
    void *p = test();
    put32(&p, 0);
    return 0;
#else
    void *p = ckalloc(4);
    ck_verbose_set(1);
    hdr_t *h = ck_ptr_is_alloced(p);
    if(h)
        output("about to check leak on block: %d, ptr=%x\n", h->block_id, p);

    check_should_leak();
    put32(p,0);
    return 0;

#endif
}
