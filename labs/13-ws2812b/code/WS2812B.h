#ifndef __WS2812B__
#define __WS2812B__
/*
 * engler: possible interface fo ws2812b light strip, cs240lx
 *
 * the low-level "driver" code for the WS2812B addressable LED light strip.  
 *      Datasheet in docs/WS2812B
 *
 * the higher level support stuff is in neopix.c
 * 
 * Key sentence  missing from most documents:
 *	Addressing is indirect: you don’t pass a NeoPixel’s address
 *	via SPI. Instead the order of data in the frame provides the
 *	addressing: the first three color values are taken by the first
 *	NeoPixel in line, the second set of values by the next NeoPixel,
 *	the third by the third and so on.
 *		--- https://developer.electricimp.com/resources/neopixels
 *
 * 	I.e., it's not a FIFO buffer, the first thing you write gets claimed
 *	by the first LED, etc.
 * 
 * to send a:
 *	- 0 = T0H, T0L
 * 	- 1 = T1H, T1L
 * 	- FLUSH = how long to wait with voltage low to flush it out.
 *
 * One way to communicate more information with a device is to use a fancier
 * protocol (e.g., SPI, I2C).  A simpler hack is to use time --- there's a lot 
 * of information you can encode in various numbers of nanoseconds.    
 *
 * The WS2812B uses time as follows:
 *      To send a 1 bit: write a 1 for T1H, then a 0 for T0H.
 *      To send a 0 bit: write a 1 for T1L, then a 0 for T0L.
 *
 * The protocol:
 *   - Each pixel needs 3 bytes of information (G,R,B) [in that order] 
 *     for the color.  
 *   - The first 3 bytes you sent get claimed by the first pixel, 
 *     the second 3 bytes by the second, and so forth.
 *   - You then hold the communication pin low for FLUSH cycles to 
 *     tell the pixel you are done.
 *  
 * The data sheet for WS2812B says the following timings: 
 *      T0H    0: .35us +- .15us
 *      T1H    1: .9us +- .15us
 *      T0L    0: .9us +- .15us
 *      T1L    1: .35us +- .15us
 *      Trst = 80us 
 *
 * The above timings work. However, for long light strings, they can 
 * consume significant time.   Ideally we'd like to push the times as 
 * low as possible so that we can set/unset more pixels per second.   
 * The following *seem* to work on the light strings I have, but could
 * break on others (or could break on mine with the wrong set of pixels).   
 * Use with caution:
 *      T0H = 200ns + eps
 *      T1H = 750ns + eps
 *      T0L = 750ns + eps
 *      T1L = 200ns + eps
 */
#include "cycle-count.h"

// A+ CPU speed: 700MHz (700 million cycles per second)
// Cool use of your mailbox code: Overclock the pi to increase the
// precision of your timings.
#define MHz 700UL

// macro to convert nanoseconds to pi cycles: you need to modify 
// this if you change the pi's clock speed.
//
// Q: what if we don't do GCD and multiply x by 700M and divide by
// 1B (1000M)?
#define ns_to_cycles(x) (unsigned) ((x * 7UL) / 10UL )

// these are the timings recommended by the datasheet in cycles.
//
//
// if you optimize the T1H, T0H, T1L, and T0L timings (make them smaller)
// i would suggest making a seperate copy you can flip between.
// why optimize:
//      the serial, time-based WS2812B protocl has the downside that 
//      everything is (obviously) O(n) based on the number of pixels 
//      --- the longer the delays for each pixel, the slower we can write
//      the entire strip.  general approach: sleazily cut down the 
//      delays to the lowest that seem to consistently work.  for a long
//      light strip this can add up.
enum { 
// #   error "you need to define these enums using the datasheet."

        // to send a 1: set pin high for T1H ns, then low for T0H ns.
        T1H = ns_to_cycles(0),        // Width of a 1 bit in cycles
        T0H = ns_to_cycles(0),        // Width of a 0 bit in cycles
        // to send a 0: set pin high for T1L ns, then low for T0L ns.
        T1L = ns_to_cycles(0),        // Width of a 1 bit in cycles
        T0L = ns_to_cycles(0),        // Width of a 0 bit in cycles

        // to make the LED switch to the new values, old the pin low for FLUSH ns
        FLUSH = ns_to_cycles(50 *1000)    // how long to hold low to flush
};

/****************************************************************************
 * Support code.  We put this first so it can be inlined.
 *
 * You should modify these.
 */

// defines the pi-specific code to read its cycle-counter.
#include "cycle-count.h"

// make sure we don't call the non-inlined gpio implementations.
#define gpio_set_on "error"
#define gpio_set_off "error"
#define gpio_write "error"


// duplicate gpio_set_on/gpio_set_off so we can inline to 
// reduce overhead. they have to run in < the delay we are 
// shooting for.
static inline void gpio_set_on_raw(unsigned pin) {
    todo("write this code");
}
static inline void gpio_set_off_raw(unsigned pin) {
    todo("write this code");
}

// use cycle_cnt_read() to delay <n_cyc> cycles measured from <start_cyc>
static inline void delay_ncycles(unsigned start_cyc, unsigned n_cyc) {
    todo("write this code");
}

/****************************************************************************
 * The entire WS2812B protocol: you will write this code.
 *
 * We inline everything to (hopefully) minimize issues meeting our 
 * time budgets. Note: Make sure to put function definitions before uses.
 *
 * You can replace these prototypes with your own functions if you want.  
 * These are just some mild suggestions.
 */

// write 1 for <ncycles>: since reading the cycle counter itself takes cycles
// you may need to add a constant to correct for this.
static inline void write_1(unsigned pin, unsigned ncycles) {
    todo("use gpio_set_on_raw to write 1 for <ncycles>");
}

// write 0 for <ncycles>: since reading the cycle counter takes cycles you
// may need to add a constant to correct for it.
static inline void write_0(unsigned pin, unsigned ncycles) {
    todo("use gpio_set_off_raw to write 0 for <ncycles>");
}

// implement T1H from the datasheet 
//   (H=high, so call write_1 with the right delay)
static inline void t1h(unsigned pin) {
    unimplemented();
}

// implement T0H from the datasheet 
//   (H=high, so call write_1 with the right delay)
static inline void t0h(unsigned pin) {
    unimplemented();
}

// implement T1L from the datasheet. 
//   (L=low, so call write_0 with the right delay)
static inline void t1l(unsigned pin) {
    unimplemented();
}

// implement T0L from the datasheed.
//   (L=low, so call write_0 with the right delay)
static inline void t0l(unsigned pin) {
    unimplemented();
}

// implement RESET from the datasheet.
static inline void treset(unsigned pin) {
    unimplemented();
}

/***********************************************************************************
 * public functions: these are called by <neopixel.c>.  we do this 
 * slight abstraction since there are other types of light strips out
 * there that can all be described with the same basic interface.
 */

// flush out the pixels.
static inline void pix_flush(unsigned pin) { 
    treset(pin); 
}

// transmit a {0,1} bit to the ws2812b
static inline void pix_sendbit(unsigned pin, uint8_t b) {
    unimplemented();
}

// use pix_sendbit to send byte <b>
//
// XXX: it might seem sensible to unroll this code.  however with all 
// the inlining it becomes huge: unclear if better.  if you decide to 
// inline it, make sure you run tests before and after.  
static void pix_sendbyte(unsigned pin, uint8_t b) {
    unimplemented();
}

// use pix_sendbyte to send bytes [<r> red, <g> green, <b> blue] out 
// on pin <pin>.
//
// NOTE: b/c pix_sendbyte is not inlined, there could be non-trivial
// delay between the different calls (return will restore regs etc, call will
// push them).  when you optimize it's possible you need to trim the delays
// you use. 
static inline void pix_sendpixel(unsigned pin, uint8_t r, uint8_t g, uint8_t b) {
    unimplemented();
}

// make sure we allow these routines in client code!
#undef gpio_set_on
#undef gpio_set_off
#undef gpio_write

#endif
