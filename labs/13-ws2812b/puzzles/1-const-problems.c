// this is a weird little puzzle that is worth understanding
#include "rpi.h"
#include "cycle-count.h"

// possible set_on_raw implementation.
static inline void gpio_set_on_raw(unsigned pin) {
    *(volatile uint32_t*)(0x2020001C) = 1 << pin;
}

void notmain(void) {
    uint32_t s,e,tot,tot1,tot2,tot3;
    
    // "it's the icache!  but the icache is off.
    // caches_enable();

    asm volatile(".align 5");
    s = cycle_cnt_read();
        gpio_set_on_raw(21);
    e = cycle_cnt_read();
    tot1 = e - s;

    asm volatile(".align 5");
    s = cycle_cnt_read();
        gpio_set_on_raw(21);
    e = cycle_cnt_read();
    tot2 = e - s;

    asm volatile(".align 5");
    s = cycle_cnt_read();
        gpio_set_on_raw(21);
    e = cycle_cnt_read();
    tot3 = e - s;

    output("run1 = %d, run2 =%d, run3 = %d\n", tot1, tot2, tot3);
}

