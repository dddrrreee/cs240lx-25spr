#include "rpi.h"

/*
    Stuart: Very minimal version of timer functions without dependencies

    The goal is to have as little as possible, and dynamically link the rest.
*/

uint32_t timer_get_usec(void) {
    dev_barrier();
    uint32_t u = GET32(0x20003004);
    dev_barrier();
    return u;
}

void delay_us(uint32_t us) {
    uint32_t s = timer_get_usec();
    while(1) {
        uint32_t e = timer_get_usec();
        if ((e - s) >= us)
            return;
    }
}

void delay_ms(uint32_t ms) {
    delay_us(ms * 1000);
}

void delay_sec(uint32_t sec) {
    delay_ms(sec * 1000);
}
