/*
    Test: act blink
    
    The GPIO functions are in libpi.so, so must be dynamically linked
*/

#include "rpi.h"

enum { act_led = 47 };
void act_init(void) { gpio_set_output(act_led); }
void act_on(void) { gpio_write(act_led,0); }
void act_off(void) { gpio_write(act_led,1); }

void notmain(void) {
    act_init();
    for(int i = 0; i < 50; i++) {
        act_on();
        delay_cycles(1000000);
        act_off();
        delay_cycles(1000000);
    }
}
