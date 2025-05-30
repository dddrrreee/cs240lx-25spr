#include "../lib/rp2350.h"

enum { LED_PIN = 25 };

void notmain()
{
    uart_init(uart0, 115200, 150'000'000);
    printk("Starting LED blinker");

    gpio_set_output(LED_PIN);
    gpio_set_off(LED_PIN);

    for (size_t j = 0; j < 10; ++j) {
        for (size_t i = 0; i < 1'000'000; ++i)
            __asm__ volatile("nop");
        gpio_toggle(LED_PIN);
    }

    pico2_reboot();
}
