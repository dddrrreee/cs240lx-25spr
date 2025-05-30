#include "../lib/rp2350.h"

void notmain()
{

    enum { LED_PIN = 25 };

    resets->reset |= RESET_UART0;
    while (!(resets->reset_done & RESET_UART0))
        ;

    uart_init(uart0, 115200, 150'000'000);

    // First, check that 'uart_putc' works.
    uart_putc(uart0, 'H');
    uart_putc(uart0, 'i');
    uart_putc(uart0, '\n');

    // First, check that `uart_puts` works.
    uart_puts(uart0, "-- TEST PUTS\n");

    // Next, test that `printk` works.
    printk("-- TEST PRINTK: Hello, %s!\n", "world");

    // By running this immediately before rebooting, if there are issues with the `flush`, then
    // we'll likely see corruption of the last few bytes. To make this more obvious, we do our
    // best to fill up the FIFO.
    uart_puts(uart0, "-- TEST FLUSH 0123456789 abcdefghijklmnopqrstuvwxyz\n");
    uart_flush(uart0);

    pico2_reboot_immediate();
}
