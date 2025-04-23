// a bit more complicated:
//  1. do N random size allocations.
//  2. make sure there aren't errors.
//  3. do N random legal writes
//  4. make sure there aren't errors.
//  5. free everything
//  6. make sure there aren't errors.
//  7. do random illegal writes, record in <count>
//  8. make sure there are <count> errors.

// use after free.
#include "rpi.h"
#include "ckalloc.h"

void notmain(void) {
    printk("test4: use after free\n");

    ck_verbose_set(0);
    enum { N  = 64 };

    //  1. do N random size allocations.
    void **vec = ckalloc(sizeof *vec * N);
    for(int i = 0; i < N; i++)
        vec[i] = ckalloc(N);

    //  2. make sure there aren't errors.
    if(ck_heap_errors())
        panic("invalid error!!\n");
    else
        trace("SUCCESS heap checked out\n");

    //  3. do N random legal writes
    for(int i = 0; i < N; i++) {
        put32(vec[i] + rpi_rand32() % i, i);
        //  4. make sure there aren't errors.
        if(ck_heap_errors())
            panic("invalid error!!\n");
    }

    //  5. free everything
    for(int i = 0; i < N; i++)
        ckfree(vec[i]);

    // heh.  can't do this one b/c will kill
    // the pointers.
    // ckfree(vec);

    //  6. make sure there aren't errors.
    if(ck_heap_errors())
        panic("invalid error!!\n");

    //  7. do random illegal writes, record in <count>
    unsigned exp_errors = 0;
    for(int i = 0; i < N; i++) {
        unsigned rz1_bug =  rpi_rand32() % REDZONE_NBYTES;
        unsigned rz2_bug =  rpi_rand32() % REDZONE_NBYTES;

        if(rpi_rand32() % 2) {
            if(rz1_bug) {
                put8(vec[i] - rz1_bug, 0);
                exp_errors++;
            }
        } else {
            if(rz2_bug) {
                put8(vec[i] + i + rz2_bug, 0);
                exp_errors++;
            }
        }
    }

    //  8. make sure there are <count> errors.
    int n = ck_heap_errors();
    if(n != exp_errors)
        panic("invalid number of errors, expected %d, nave %d\n", n, N*2);

    trace("SUCCESS heap checked out\n");
}
