/*
    Taken from the thread lab in 140E
*/
#include "rpi.h"
#include "rpi-thread.h"

static unsigned thread_count, thread_sum;

static void inline test_init(void) {
    unsigned oneMB = 1024*1024;
    kmalloc_init_set_start((void*)oneMB, oneMB);
}
 
 // trivial first thread: does not block, explicitly calls exit.
 static void thread_code(void *arg) {
     unsigned *x = arg;
 
     // check tid
     unsigned tid = rpi_cur_thread()->tid;
     trace("in thread tid=%d, with x=%d\n", tid, *x);
     demand(rpi_cur_thread()->tid == *x+1, 
                 "expected %d, have %d\n", tid,*x+1);
 
     // check yield.
     rpi_yield();
     thread_count ++;
     rpi_yield();
     thread_sum += *x;
     rpi_yield();
     // check exit
     rpi_exit(0);
 }
 
 void notmain() {
     test_init();
 
     // change this to increase the number of threads.
     int n = 30;
     trace("about to test summing of n=%d threads\n", n);
     thread_sum = thread_count = 0;
 
     unsigned sum = 0;
     for(int i = 0; i < n; i++)  {
         int *x = kmalloc(sizeof *x);
         sum += *x = i;
         rpi_fork(thread_code, x);
     }
     rpi_thread_start();
 
     // no more threads: check.
     trace("count = %d, sum=%d\n", thread_count, thread_sum);
     assert(thread_count == n);
     assert(thread_sum == sum);
     trace("SUCCESS!\n");
 }
 