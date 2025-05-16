/*
 * "higher-level" neopixel stuff.
 * our model:
 *  1. you write to an array with as many entries as there are pixels 
 *     as many times as you want.
 *  2. when done with (1) you call <neopix_flush> to externalize the
 *     to pixel array.
 *
 * Note:
 *  - out of bound writes are ignored.
 *  - do not allow interrupts during the flush!
 */
#include "rpi.h"
#include "neopixel.h"
#include "WS2812B.h"

// from datasheet: r,g,b are 8-bit each (each pixel is 24 bit color)
struct neo_pixel {
    uint8_t r,g,b;
};

// you can add other information to this if you want: this gives us a way to 
// control multiple light arrays concurrently, and to use different brands.
struct neo_handle {
    uint8_t pin;      // output pin
    uint32_t npixel;  // number of pixesl.

    // struct hack
    struct neo_pixel pixels[];
};

neo_t neopix_init(uint8_t pin, unsigned npixel) {
    neo_t h;
    unsigned nbytes = sizeof *h + sizeof h->pixels[0] * npixel;
    h = (void*)kmalloc(nbytes);
    memset(h, 0, nbytes);

    h->npixel = npixel;
    h->pin = pin;
    gpio_set_output(pin);
    return h;
}

// write the pixel out with [r,g,b] using <WS2812b.h>
void neopix_sendpixel(neo_t h, uint8_t r, uint8_t g, uint8_t b) {
    todo("transmit [r,g,b]");
}

// do the work:
//  1. write all <npixel> pixels out using <neopix_sendpixel>
//  2. then flush.  
//  3. memset the array to 0 after.
void neopix_flush(neo_t h) { 
    todo("treset");
}

// set pixel <pos> in <h> to {r,g,b}
void neopix_write(neo_t h, uint32_t pos, uint8_t r, uint8_t g, uint8_t b) {
    // silently clip
    if(pos >= h->npixel)
        return;
    todo("append [r,g,b] to h->pixels");
}

// we give this. 
void neopix_fast_clear(neo_t h, unsigned n) {
    for(int i = 0; i < n; i++)
        neopix_sendpixel(h,0,0,0);
    neopix_flush(h);
}

// we give this. 
void neopix_clear(neo_t h) {
    neopix_fast_clear(h, h->npixel);
}
