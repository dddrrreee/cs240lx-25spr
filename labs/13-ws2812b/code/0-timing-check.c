// engler: timing checks for the ws2812b code.  requires that you implement:
//       - delay_ncycles
//       - t1h, t1l, t0h, t0l 
//
// what to do:
//    work through each of the parts (1-6) to make sure your code 
//    meets timing requirements.
//
//    extension credit if you figure out better ways to do stuff!
// 
//  The WS2812b requires nanosecond accurate writes; it's easy to mess
//  these up with random changes.  The code below attempts to performance
//  check that the code workes within its budget.
//
// Some problems:
//  1. obviously monitoring changes timing, so we never really know
//      *exactly* how long something takes.    
// 
//   2. while we detect if the routines the code calls has different
//      timings, these calls are to *copies* of the code (b/c of inlining).
//      the neopixel code calls its own copy of the routine (again:
//      inlining).  as a result, our "checks" can pass routines that will
//      fail when used by neopixel.  e.g., b/c their alignment differs,
//      changinging timing.
//
// partial solution to the above: you can use a second pi and hard poll
// on the output pin recording when/how long the pin was hi/low.   this 
// primitive digital analyzer is actually easy to build, and useful 
// for other things.   we have a lab on this from previous 240lx/140es
// that beats a $400 salae. it's a fun puzzle.
#include "rpi.h"
#include "WS2812B.h"
#include "neopixel.h"
#include "cycle-count.h"

// the pin used to control the light strip.
enum { pix_pin = 21 };

// the alignment we need for today.
#define align()  asm volatile (".align 4")

//********************************************************************
// part 1. even doing nothing has overhead (of course).  
// make sure the overhead of doing an empty, aligned timing
// equals <overhead> (8 cycles)

// measured overhead of using cycle cnt read to measure nothing
enum { overhead = 8 };

void measure_overhead(void) {
    align();
	unsigned s = cycle_cnt_read();
	unsigned e = cycle_cnt_read();
    unsigned tot = e - s;
    if(tot != overhead)
        panic("ERROR: expected %d cycles overhead: have %d\n", 
            overhead, tot);
    output("\tSUCCESS: empty timing has expected overhead = %d\n", tot);
}

static void part1(void) {
    output("------------------------------------------------------------\n");
    output("part 1: measure cost of raw measurement: should be 8 cycles\n");
    measure_overhead();
    measure_overhead();   // double check that it doesn't change
    measure_overhead();   // double check that it doesn't change
}

//********************************************************************
// part 2: check that your delay gives pretty close timings 
// for the values we need (T1H, T1L, T0H, T0L).

// example of how to check timings.  we won't 
// get exact matching, so set a lower bound and
// upper bound and give an error if its outside
// that range.
void check_delay(unsigned delay) {
    output("  checking delay_ncycles(%d)\n", delay);

    align();
	uint32_t s = cycle_cnt_read();
        delay_ncycles(s, delay);
	uint32_t e = cycle_cnt_read();

    uint32_t tot    = e - s;
    uint32_t lb     = delay;
    if(tot < lb)
        panic("TOO FAST: delay_ncycles(%d) took %d cyc\n", delay, tot);

    // if caching off, this gets harder.
    uint32_t slop   = 16;
    uint32_t ub     = lb + slop + overhead;
    if(tot > ub)
        panic("TOO SLOW: delay_ncycles(%d) took %d cyc, expected at most %d\n",
            delay, tot, ub);

    output("\tSUCCESS: delay_ncycles(%d) took %d cycle, between [lb=%d, ub=%d]\n", 
            delay, tot, lb,ub);
}

static void part2(void) {
    output("------------------------------------------------------------\n");
    output("part 2: measuring accuray of different <delay> invocations\n");
    check_delay(1000);
    check_delay(T1H);
    check_delay(T1L);
    check_delay(T0H);
    check_delay(T0L);
}

//**************************************************************
// part 3. check that our raw timing for gpio operations is
// as expected.

void part3(void) {
    output("------------------------------------------------------------\n");
    output("part 3: simple timing of raw gpio op\n");

    align();
	uint32_t s = cycle_cnt_read();
	    gpio_set_off_raw(21);
	uint32_t e = cycle_cnt_read();

    uint32_t tot = e - s;
    // this is a very very slow time.
    uint32_t exp = 100;
    if(tot > exp)
        panic("TOO SLOW: max cycles = %d, took %d cycles\n", exp,tot);
    output("\t<gpio_set_off_raw> = %d cycles\n", tot);
}


//******************************************************
// part 4. wrap up all the boilerplate in a macro to make the other
// checks not take so much code.  use it to compare the raw version 
// to libpi version.
void part4(void) { 

    output("------------------------------------------------------------\n");
    output("part 4: wrapping up the measurements and re-doing gpio_set_off_raw\n");

#   define STRING_MK(s) #s

    // measure <fn> and print result.  subtracts overhead.
#   define MEASURE_FN(fn) ({                            \
        align();                                   \
        gcc_mb();\
	    uint32_t s = cycle_cnt_read();                  \
            fn;                                         \
	    uint32_t e = cycle_cnt_read();                  \
        gcc_mb();\
	    uint32_t tot = e - s;                           \
                                                        \
        /* correction = subtract measurement overhead. */\
        output("\t<%s> took %d cycles [corrected=%d]\n",    \
            STRING_MK(fn), tot, tot-overhead);                         \
        tot-overhead;                                   \
    })


    // XXXX OOOOh.  the first is a cache miss for the address.
    // then it reuses it.

    output("  About to compare <gpio_set_off_raw> to <gpio_set_off>\n");
    uint32_t tot  = MEASURE_FN(gpio_set_off(21));
    tot += MEASURE_FN(gpio_set_off(21));
    tot += MEASURE_FN(gpio_set_off(21));

    uint32_t tot_raw  = MEASURE_FN(gpio_set_off_raw(21));
    tot_raw += MEASURE_FN(gpio_set_off_raw(21));
    tot_raw += MEASURE_FN(gpio_set_off_raw(21));


    // NOTE: this is the *corrected* overhead.
    output("  raw total = %d vs %d (diff=%d cycles)\n\n", tot_raw, tot, tot-tot_raw);
}


//****************************************************************
// Part 5.  compare <gpio_set_on_raw> to libpi's <gpio_set_on>

void part5(void) { 
    output("------------------------------------------------------------\n");
    output("part 5: measuring gpio_set_on_raw\n");

    // we remeasure <gpio_set_on_raw> to make sure it's the same.
    output("About to compare <gpio_set_on_raw> to <gpio_set_on>\n");

    uint32_t tot  = MEASURE_FN(gpio_set_on(21));
    tot += MEASURE_FN(gpio_set_on(21));
    tot += MEASURE_FN(gpio_set_on(21));

    uint32_t tot_raw  = MEASURE_FN(gpio_set_on_raw(21));
    tot_raw += MEASURE_FN(gpio_set_on_raw(21));
    tot_raw += MEASURE_FN(gpio_set_on_raw(21));

    // NOTE: this is the *corrected* overhead.
    output("raw total = %d vs %d (diff=%d cycles)\n\n", 
                            tot_raw, tot, tot-tot_raw);
}


//************************************************************
// final part: do the hard timings of the foundational
// routines.

void part6(void) { 
    output("------------------------------------------------------------\n");
    output("part 6: do timing checks.  this is the main goal.\n");
    
    // if you don't care about <lb_bound> just pass in 0.
#   define CHECK_TIMING(fn, lb_bound, ub_bound) do {                       \
        uint32_t tot = MEASURE_FN(fn);                                     \
                                                                           \
        const char *fn_str = STRING_MK(fn);                                \
        /* check that <fn> wasn't faster than <lb_bound> */                \
        if(tot < lb_bound)                                                 \
            panic("TOO FAST: <%s> took %d cyc, lower-bound=%d\n",          \
                fn_str, tot, lb_bound);                                    \
                                                                           \
        /* check that <fn> wasn't slower than <ub_bound> */                \
        if(tot > ub_bound)                                                 \
            panic("TOO SLOW: <%s> took %d cyc, expected at most %d\n",     \
                fn_str, tot, ub_bound);                                    \
                                                                           \
        /* check that <fn> wasn't slower than <ub_bound> */                \
        output("\tSUCCESS: <%s> took %d cycle, b/n [lb=%d,ub=%d]\n",              \
                fn_str, tot, lb_bound,ub_bound);    \
    } while(0)

    // tolerance: 8 cycles
    unsigned tol = 8;

    output("  goal is %d cycles\n", T1H);
    CHECK_TIMING(t1h(21), T1H-tol, T1H+tol);
    CHECK_TIMING(t1h(21), T1H-tol, T1H+tol);
    CHECK_TIMING(t1h(21), T1H-tol, T1H+tol);

    output("  goal is %d cycles\n", T0H);
    CHECK_TIMING(t0h(21), T0H-tol, T0H+tol);
    CHECK_TIMING(t0h(21), T0H-tol, T0H+tol);
    CHECK_TIMING(t0h(21), T0H-tol, T0H+tol);

    output("  goal is %d cycles\n", T1L);
    CHECK_TIMING(t1l(21), T1L-tol, T1L+tol);
    CHECK_TIMING(t1l(21), T1L-tol, T1L+tol);
    CHECK_TIMING(t1l(21), T1L-tol, T1L+tol);
    CHECK_TIMING(t1l(21), T1L-tol, T1L+tol);

    output("  goal is %d cycles\n", T0L);
    CHECK_TIMING(t0l(21), T0L-tol, T0L+tol);
    CHECK_TIMING(t0l(21), T0L-tol, T0L+tol);
    CHECK_TIMING(t0l(21), T0L-tol, T0L+tol);

}


void notmain(void) {
    // enable icache and branch prediction.
    caches_enable();

    // not needed since we don't use the result.
    gpio_set_output(pix_pin);

    // we outline these calls to make it easy to turn off/on"
    // knock them off one at a time.
    part1();
    part2();
    part3();
    part4();
    part5();
    part6();

    trace("SUCCESS!\n");
}

