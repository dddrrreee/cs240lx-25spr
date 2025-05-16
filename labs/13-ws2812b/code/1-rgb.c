// simple test to check if you do red, green, blue correctly.
#include "rpi.h"
#include "WS2812B.h"
#include "neopixel.h"

// the pin used to control the light strip.
enum { pix_pin = 21 };

void notmain(void) {
    kmalloc_init(1);
    // if you don't do this, the granularity is too large for the timing
    // loop. 
    caches_enable(); 
    gpio_set_output(pix_pin);

    // how many pixels you want on.
    unsigned n = 16;

    // turn on one pixel to blue.
    // alter the code to make sure you can:
    //  1. write red and gree
    //  2. write different pixels.
    for(unsigned i = 0; i < 2; i++) {
        output("setting red\n");
        for(unsigned j = 0; j < n; j++)
            pix_sendpixel(pix_pin, 0xff,0,0);
        pix_flush(pix_pin);
        delay_ms(1000*5);

        output("setting green\n");
        for(unsigned j = 0; j < n; j++)
            pix_sendpixel(pix_pin, 0, 0xff,0);
        pix_flush(pix_pin);
        delay_ms(1000*5);

        output("setting blue\n");
        for(unsigned j = 0; j < n; j++)
            pix_sendpixel(pix_pin, 0,0, 0xff);
        pix_flush(pix_pin);
        delay_ms(1000*5);
    }
    output("done\n");
}
