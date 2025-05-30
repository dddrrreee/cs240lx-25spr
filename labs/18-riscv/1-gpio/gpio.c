#include "../lib/rp2350.h"

void gpio_set_function(pin_t pin, io_funcsel_t funcsel)
{
    if (pin > 47 || funcsel >= 0x20) {
        panic("illegal pin: %d or funcsel: %d", pin, funcsel);
    }
    todo("implement me");
}

void gpio_set_output(pin_t pin)
{
    // You need to:
    //  - set the FUNCSEL
    //  - set up the pad configuration
    //  - enable the output in the SIO
    todo("implement me");
}

void gpio_set_on(pin_t pin)
{
    // There are a couple different ways you can do this, but there's a simplest way to do this that uses only one MMIO write
    todo("implement me");
}

void gpio_set_off(pin_t pin)
{
    todo("implement me");
}

void gpio_write(pin_t pin, bool level)
{
    // There's a way to implement this without branching
    todo("implement me");
}

void gpio_toggle(pin_t pin)
{
    // Also a way to implement this with one MMIO store
    todo("implement me");
}
