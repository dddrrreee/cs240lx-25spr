#include "../lib/rp2350.h"

void notmain()
{
    enum { LED_PIN = 25 };

    gpio_set_output(LED_PIN);

    sio->gpio_out_clr = 1 << LED_PIN;

    // for (size_t j = 0; j < 5; ++j) {
    // for (size_t i = 0; i < 1'000'000; ++i)
    // __asm__ volatile("nop");
    // sio->gpio_out_xor = 1 << LED_PIN;
    // }

    uart_init(uart0, 115200, 150'000'000);

    printk("Hello, %s!\n", "world");

    uart_flush(uart0);

    pico2_reboot();
}
