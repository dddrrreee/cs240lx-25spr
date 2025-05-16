/*
 * simple test to use your buffered neopixel interface to push a cursor around
 * a light array.
 */
#include "rpi.h"
#include "WS2812B.h"
#include "neopixel.h"

// the pin used to control the light strip.
enum { pix_pin = 21 };

// crude routine to write a pixel at a given location.
void place_cursor(neo_t h, int i) {
    neopix_write(h,i-2,0xff,0,0);
    neopix_write(h,i-1,0,0xff,0);
    neopix_write(h,i,0,0,0xff);
    neopix_flush(h);
}

void notmain(void) {
    kmalloc_init(1);
    caches_enable(); 
    gpio_set_output(pix_pin);

    // simple example program of the neopixel.
    unsigned npixels = 30;  // you'll have to figure this out.
    neo_t h = neopix_init(pix_pin, npixels);

    // does increasingly faster loops.  bump loop if want.
    unsigned loop = 1;
    while(loop-- > 0) {
        for(int n = 0; n < 30; n++) {
            for(int j = 0; j < 10; j++) {
                output("loop %d\n", j);
                for(int i = 0; i < npixels; i++) {
                    place_cursor(h,i);
                    delay_ms(10-j);
                }
            }
        }
    }
    output("done!\n");
}
