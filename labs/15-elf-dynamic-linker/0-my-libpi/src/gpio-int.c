// engler, cs140 put your gpio-int implementations in here.
#include "rpi.h"

// in libpi/include: has useful enums.
#include "rpi-interrupts.h"

/*
 * enum for GPIO interrupt events
 * BCM2835-ARM-Peripheral document page 90
 */
enum
{
    GPIO_EVENT_DETECT_STATUS = 0x20200040, // 2 continuous registers
    GPIO_RISING_EDGE_DETECT_ENABLE = 0x2020004C, // 2 continuous registers
    GPIO_FALLING_EDGE_DETECT_ENABLE = 0x20200058 // 2 continuous registers
};

// BCM page 113 tells us gpio_int[0] is #49 (0-indexed)
static const int GPIO_INT0_TABLE_INDEX = 49;

// returns 1 if there is currently a GPIO_INT0 interrupt, 
// 0 otherwise.
//
// note: we can only get interrupts for <GPIO_INT0> since the
// (the other pins are inaccessible for external devices).
int gpio_has_interrupt(void) {
    // BCM page 115 tells us that GPU pending 2 reg gives source 63:32
    dev_barrier();
    int result = (GET32(IRQ_pending_2) >> (GPIO_INT0_TABLE_INDEX - 32)) & 0b1;
    dev_barrier();
    // DEV_VAL32(result);
    return result;
}

// p97 set to detect rising edge (0->1) on <pin>.
// as the broadcom doc states, it  detects by sampling based on the clock.
// it looks for "011" (low, hi, hi) to suppress noise.  i.e., its triggered only
// *after* a 1 reading has been sampled twice, so there will be delay.
// if you want lower latency, you should us async rising edge (p99)
void gpio_int_rising_edge(unsigned pin) {
    if(pin>=32)
        return;
    dev_barrier();
    // Must read-modify-write, setting 0 will disable
    // todo("implement: detect rising edge\n");
    PUT32(GPIO_RISING_EDGE_DETECT_ENABLE, GET32(GPIO_RISING_EDGE_DETECT_ENABLE) | (0b1 << pin));
    dev_barrier();

    // page 117 of BCM. No need to read-modify-write
    PUT32(IRQ_Enable_2, 0b1 << (GPIO_INT0_TABLE_INDEX - 32));
    dev_barrier();
}

// p98: detect falling edge (1->0).  sampled using the system clock.  
// similarly to rising edge detection, it suppresses noise by looking for
// "100" --- i.e., is triggered after two readings of "0" and so the 
// interrupt is delayed two clock cycles.   if you want  lower latency,
// you should use async falling edge. (p99)
void gpio_int_falling_edge(unsigned pin) {
    if(pin>=32)
        return;
    // Must read-modify-write, setting 0 will disable
    // todo("implement: detect falling edge\n");
    dev_barrier();
    PUT32(GPIO_FALLING_EDGE_DETECT_ENABLE, GET32(GPIO_FALLING_EDGE_DETECT_ENABLE) | (0b1 << pin));
    dev_barrier();

    // page 117 of BCM. No need to read-modify-write
    PUT32(IRQ_Enable_2, 0b1 << (GPIO_INT0_TABLE_INDEX - 32));
    dev_barrier();
}

// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an interrupt.
// if you configure multiple events to lead to interrupts, you will have to 
// read the pin to determine which caused it.
int gpio_event_detected(unsigned pin) {
    if(pin>=32)
        return 0;
    // todo("implement: is an event detected?\n");
    dev_barrier();
    int result = (GET32(GPIO_EVENT_DETECT_STATUS) >> pin) & 0b1;
    dev_barrier();
    // DEV_VAL32(result);
    return result;
}

// p96: have to write a 1 to the pin to clear the event.
void gpio_event_clear(unsigned pin) {
    if(pin>=32)
        return;
    // todo("implement: clear event on <pin>\n");
    // Read-modify-write, just in case (doc doesn't say it)
    dev_barrier();
    PUT32(GPIO_EVENT_DETECT_STATUS, 0b1 << pin);
    dev_barrier();
}
