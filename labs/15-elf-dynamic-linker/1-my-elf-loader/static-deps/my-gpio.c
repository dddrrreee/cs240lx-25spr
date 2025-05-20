/*
    Stuart: Very minimal version of gpio functions without dependencies

    The goal is to have as little as possible, and dynamically link the rest.
*/

#include "rpi.h"

enum
{
    GPIO_BASE = 0x20200000, // also GPFSEL0
    gpio_set0 = (GPIO_BASE + 0x1C),
    gpio_clr0 = (GPIO_BASE + 0x28),
    gpio_lev0 = (GPIO_BASE + 0x34)
};

void gpio_set_function(unsigned pin, gpio_func_t function)
{
    if (pin >= 32 && pin != 47)
        return;
    if (function < 0 || function > 7)
        return;

    volatile unsigned *target_addr = (volatile unsigned *)GPIO_BASE + pin / 10;
    unsigned shift = (pin % 10) * 3;
    unsigned mask = 0b111 << shift;

    put32(target_addr, (get32(target_addr) & ~mask) | (function << shift));
}

void gpio_set_output(unsigned pin)
{
    if (pin >= 32 && pin != 47)
        return;
    gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

void gpio_set_on(unsigned pin)
{
    if (pin >= 32 && pin != 47)
        return;

    if (pin < 32)
        PUT32(gpio_set0, 0x1 << pin);
    else
        PUT32(gpio_set0 + 4, 0x1 << (pin - 32u));
}

void gpio_set_off(unsigned pin)
{
    if (pin >= 32 && pin != 47)
        return;

    if (pin < 32)
        PUT32(gpio_clr0, 0x1 << pin);
    else
        PUT32(gpio_clr0 + 4, 0x1 << (pin - 32u));
}

void gpio_write(unsigned pin, unsigned v)
{
    if (pin >= 32 && pin != 47)
        return;
    if (v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

void gpio_set_input(unsigned pin)
{
    if (pin >= 32 && pin != 47)
        return;
    gpio_set_function(pin, GPIO_FUNC_INPUT);
}

int gpio_read(unsigned pin)
{
    if (pin >= 32)
        return -1;

    // Datasheet pg 96 (set nth bit for pin n)
    int v = (GET32(gpio_lev0) & (0x1 << pin)) >> pin;
    return DEV_VAL32(v);
}
