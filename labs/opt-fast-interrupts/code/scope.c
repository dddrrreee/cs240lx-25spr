/*
 * simple interrupt-based logic analyzer starter code.  you should
 * be able to make this both much faster and much more accurate.
 */
#include "rpi.h"
#include "timer-interrupt.h"
#include "cycle-count.h"
#include "vector-base.h"

// can change these pins to whatever you want.  note that
// some pins make values that are faster to load on the 
// armv6!
enum { out_pin = 26, in_pin = 27 };

static volatile unsigned n_rising_edge, n_falling_edge;
static volatile unsigned n_interrupt;

// trivial array of the pin changes we've seen.
#define N 128
static volatile struct event {
    unsigned v;         // value that pin transitioned from.
    unsigned ncycles;   // the raw cycle cnt we read on interrupt.
} events[N];
static volatile unsigned nevents;

// dumb interrupt handler that records the cycle
// that <in_pin> goes high or low.
void int_vector(uint32_t pc) {
    unsigned s = cycle_cnt_read();

    dev_barrier();

    if(!gpio_event_detected(in_pin)) 
        panic("unexpected interrupt\n");
    else {
        n_interrupt++;

        assert(nevents < N);
        let e = &events[nevents++];
        e->ncycles = s;

        if(gpio_read(in_pin) == 0) {
            n_falling_edge++;
            e->v = 1;
        } else {
            n_rising_edge++;
            e->v = 0;
        }
        gpio_event_clear(in_pin);
    }

    dev_barrier();
}

void test_cost(unsigned pin) { 
    uint32_t s, e;
    volatile struct event *p;

    enum { ntrials = 20 };
    assert(ntrials < N);

    output("about to do %d tests\n", ntrials);
    for(int i = 1; i < ntrials; i++) {
        s = cycle_cnt_read();
        gpio_write(pin,1);
        while(nevents < i)
            ;
        e = cycle_cnt_read();

        p = &events[nevents-1];
        output("int cost = %d [%d from gpio write until cycle_cnt_read()]\n", 
                e - s, p->ncycles - s);
        i++;

        s = cycle_cnt_read();
        gpio_write(pin,0);
        while(nevents < i)
            ;
        e = cycle_cnt_read();
        p = &events[nevents-1];
        output("int cost = %d [%d from gpio write until cycle_cnt_read()]\n", 
                e - s, p->ncycles - s);
    }
}

void notmain() {
    //*****************************************************
    // 1. setup pins and check that loopback works.
    gpio_set_output(out_pin);
    gpio_set_input(in_pin);

    // make sure that loopback is setup.
    gpio_write(out_pin, 1);
    if(gpio_read(in_pin) != 1)
        panic("connect jumper from pin %d to pin %d\n", 
                                    in_pin, out_pin);
    gpio_write(out_pin, 0);
    if(gpio_read(in_pin) != 0)
        panic("connect jumper from pin %d to pin %d\n", 
                                    in_pin, out_pin);

    //*****************************************************
    // 2. setup interrupts
    extern uint32_t default_vec_ints[];

    // setup interrupts.  you've seen this code
    // before.
    dev_barrier();
    PUT32(IRQ_Disable_1, 0xffffffff);
    PUT32(IRQ_Disable_2, 0xffffffff);
    dev_barrier();
    vector_base_set(default_vec_ints);

    // setup interrupts on both rising and falling edges.
    gpio_int_rising_edge(in_pin);
    gpio_int_falling_edge(in_pin);

    // clear so that we don't get a delayed interrupt.
    gpio_event_clear(in_pin);

    // now we are live!
    enable_interrupts();

    //*****************************************************
    // 3. run the test.

    // leave this off initially so its easier to see the effect
    // of speed improvements.
    // caches_enable();

    test_cost(out_pin);
    output("Done!\n");
    return;
}
