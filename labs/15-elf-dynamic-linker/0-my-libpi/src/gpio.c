/*
 * Implement the following routines to set GPIO pins to input or
 * output, and to read (input) and write (output) them.
 *  - DO NOT USE loads and stores directly: only use GET32 and
 *    PUT32 to read and write memory.
 *  - DO USE the minimum number of such calls.
 * (Both of these matter for the next lab.)
 *
 * See rpi.h in this directory for the definitions.
 */
#include "rpi.h"

// see broadcomm documents for magic addresses.
//
// if you pass addresses as:
//  - pointers use put32/get32.
//  - integers: use PUT32/GET32.
//  semantics are the same.
enum
{
    GPIO_BASE = 0x20200000, // also GPFSEL0
    gpio_set0 = (GPIO_BASE + 0x1C),
    gpio_clr0 = (GPIO_BASE + 0x28),
    gpio_lev0 = (GPIO_BASE + 0x34)
};

// set GPIO function for <pin> (input, output, alt...).  settings for other
// pins should be unchanged.
void gpio_set_function(unsigned pin, gpio_func_t function)
{
    if (pin >= 32 && pin != 47)
        return;
    if (function < 0 || function > 7)
        return;

    // Datasheet pg91~93
    volatile unsigned *target_addr = (volatile unsigned *)GPIO_BASE + pin / 10;
    unsigned shift = (pin % 10) * 3;
    unsigned mask = 0b111 << shift;

    // Read, modify, and write back
    put32(target_addr, (get32(target_addr) & ~mask) | (function << shift));
}

// set <pin> to be an output pin.
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use array calculations!
void gpio_set_output(unsigned pin)
{
    if (pin >= 32 && pin != 47)
        return;
    gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin)
{
    if (pin >= 32 && pin != 47)
        return;

    // Datasheet pg 95 (set nth bit for pin n)
    if (pin < 32)
        PUT32(gpio_set0, 0x1 << pin);
    else
        PUT32(gpio_set0 + 4, 0x1 << (pin - 32u));
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin)
{
    // this check is redundant, but it will help when
    // it is no longer redundant in the future
    if (pin >= 32 && pin != 47)
        return;

    // Datasheet pg 95 (set nth bit for pin n)
    if (pin < 32)
        PUT32(gpio_clr0, 0x1 << pin);
    else
        PUT32(gpio_clr0 + 4, 0x1 << (pin - 32u));
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v)
{
    if (pin >= 32 && pin != 47)
        return;
    if (v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

// set <pin> to input.
void gpio_set_input(unsigned pin)
{
    if (pin >= 32 && pin != 47)
        return;
    gpio_set_function(pin, GPIO_FUNC_INPUT);
}

// return the value of <pin>
int gpio_read(unsigned pin)
{
    if (pin >= 32)
        return -1;

    // Datasheet pg 96 (set nth bit for pin n)
    int v = (GET32(gpio_lev0) & (0x1 << pin)) >> pin;
    return DEV_VAL32(v);
}
