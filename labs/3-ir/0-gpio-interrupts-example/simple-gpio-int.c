// simple example of using rising and falling edge interrupts.
//   - connect GPIO <in_pin> to GPIO <out_pin> using a jumper
//     (i.e., have it "loopback")
//   - if out_pin=0: write a 1 and check we get a rising edge 
//     interrupt.
//   - if out_pin=1: write a 0 and check we get a falling 
//     edge interrupt.
#include "rpi.h"
#include "rpi-interrupts.h"
#include "rpi-inline-asm.h"

// make sure you connect 17 to 19.
enum { in_pin = 17, out_pin = 19 };

// these have to be volatile since the interrupt
// handler modifies them and the non-interrupt
// code reads them.
static volatile unsigned n_interrupt, n_falling, n_rising;
 
// called from <interrupt-asm.S> on interrupt.
void interrupt_vector(unsigned pc) {
    dev_barrier();
    n_interrupt++;
    if(gpio_event_detected(in_pin)) {
        if(gpio_read(in_pin) == 0) {
            n_falling++;
            output("falling edge=%d\n", n_falling);
        } else {
            n_rising++;
            output("rising edge=%d\n", n_rising);
        }
        gpio_event_clear(in_pin);
    }
    dev_barrier();
}

// initialize all the interrupt stuff. 
void notmain() {
    trace("test: rise/fall edge detection.\n");

    // install interrupt vector.
    extern uint32_t interrupt_vec[];
    int_vec_init(interrupt_vec);

    gpio_set_output(out_pin);
    gpio_set_input(in_pin);

    // sanity check that loopback works.
    gpio_write(out_pin, 1);
    if(gpio_read(in_pin) != 1)
        panic("must put a loopback jumper from pin=%d to pin=%d\n", 
            in_pin, out_pin);
    gpio_write(out_pin,0);
    // should never fail since the first test passed.
    assert(gpio_read(in_pin) == 0);

    // set interupts on both rising and falling edge.
    gpio_int_falling_edge(in_pin);
    gpio_int_rising_edge(in_pin);
    // if we don't clear this: events seem sticky so 
    // will trigger when enable general interrupts.
    gpio_event_clear(in_pin);

    output("about to enable interrupts\n");
    cpsr_int_enable();

    // now run with interrupts, checking that we get
    //  - a rising edge interrupt when going from 0->1
    //  - a falling edge interupt when going from 1->0.
    for(int i = 0; i < 10; i++) {
        output("%d: expect a rising edge\n",i);
        gpio_write(out_pin, 1);
        assert(n_rising == i+1);

        output("%d: expect a falling edge\n", i);
        gpio_write(out_pin, 0);
        assert(n_falling == i+1);
    }
    trace("SUCCESS: test passed!\n");
}
