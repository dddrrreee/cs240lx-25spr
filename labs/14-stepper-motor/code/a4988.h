#ifndef __A4988_H__
#define __A4988_H__

// this is a dopey implementation --- you likely want to tweak the
// data structure and code signatures (esp if go with threads or
// interrupts).
enum { forward = 0, backward = 1, off = 2 };

// initial simple-minded per-stepper structure.  you probably want
// to modify this extensively.
//
// potentially extend with: 
//   1. setup and hold time from the datasheet.
//   2. degrees per step.
//   3. change to nanoseconds to can have more precision on 
//      ramp-up and slow-down.
//   4. microsteps.
//   5. possibly add the orientation in degrees to make it easier for
//      clients?
typedef struct {
    unsigned dir_pin;   // pin to set direction.
    unsigned dir_delay; // how many usec to wait after setting dir pin
    unsigned step_pin;  // pin to set steps.
    unsigned step_delay;// how many usec to wait after setting step pin

    unsigned nstep;
} step_t;

static inline step_t 
step_mk(unsigned dir_pin, 
        unsigned dir_delay, 
        unsigned step_pin, 
        unsigned step_delay) {
    assert(dir_pin != step_pin);
    assert(dir_pin && dir_pin < 32);
    assert(step_pin && step_pin < 32);
    assert(step_delay > 0);

    // dir delay should be smaller.
    assert(dir_delay <= step_delay);

    dev_barrier();
    gpio_set_output(dir_pin);
    gpio_set_output(step_pin);
    dev_barrier();

    return (step_t) {
        .step_delay = step_delay,
        .dir_delay = dir_delay,
        .dir_pin = dir_pin,
        .step_pin = step_pin
    };
}

// note: if we're turning the same as the previous direction,
// can reduce delays.
// 
// generally: start with longer delays.
static inline void step(step_t *s, int dir) {
    assert(dir == forward || dir == backward);
    assert(s->step_delay);

    unimplemented();
}

static inline void step_forward(step_t *s) { step(s,forward); }
static inline void step_backward(step_t *s) { step(s,backward); }

// step <n> steps in direction <dir>.  this lets the driver
// optimize ramp up and ramp down speeds.
static inline void step_n(step_t *s, int dir, unsigned n) {
    todo("option: implement so you go fast(er) but don't miss steps.\n");
}

#endif
