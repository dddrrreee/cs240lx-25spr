/*
 * engler: example of how alignment can make a difference in 
 * timing on the r/pi A+.  
 *
 * the underlying issue seems to be that the r/pi *appears* to have 
 * a 32 byte prefetch buffer, even with icache disabled.  
 * 
 * whether 8 4-byte instructions fit entirely within the buffer is entirely
 * determined by alignment --- in this case, if the address of the first 
 * instruction is 32 byte aligned (addr % 32 == 0).
 *
 * if you run 2 instructions that entirely fit in the buffer,
 * this will take less time than if the 2 instructions straddle 
 * the buffer (the address of inst 1 % 32 = 28)
 */
#include "rpi.h"
#include "cycle-count.h"

void check_align(void);

void foo(void) {
    unsigned b,e, overhead;
    // instruction prefetch = 32 bytes
    //
    // this alignment will keep the two cycle_cnt_reads() in 
    // a single 32-byte prefetch buffer fetch.  (note, since
    // it's 2 instructions, any alignment >= 8 will do so.
    asm volatile(".align 5");
    b = cycle_cnt_read();
    e = cycle_cnt_read();
    overhead = e-b;
    output("  aligned overhead:\t%d cycles\n", overhead);

    // this should not fail.
    if(overhead != 8)
        panic("expected 8, have overhead=%d\n", overhead);

    // instruction prefetch seems to be 32 bytes.  so we push 
    // the two cycle count reads into different prefetches
    asm volatile(".align 5");
    asm volatile("nop");    // 1
    asm volatile("nop");    // 2
    asm volatile("nop");    // 3
    asm volatile("nop");    // 4
    asm volatile("nop");    // 5
    asm volatile("nop");    // 6
    asm volatile("nop");    // 7
    b = cycle_cnt_read();   // 8: last 4 bytes of prefetch
    e = cycle_cnt_read();   // 1 in the next inst buffer fetch
    overhead = e-b;

    // oh this is very very weird: bounces around from 36 to 109
    // extension if you can figure it out!
    output("  non-aligned overhead:\t%d cycles\n\n", overhead);

    // we do not expect overhead to be 8 given the alignment
    if(overhead <= 8)
        panic("\tnon-align fail: expected > 8, have overhead of %d\n", overhead);
}

void notmain(void) {
    // caches_enable();
    foo();
    foo();
    foo();
    foo();
    foo();
    foo();
    foo();
    foo();
}
