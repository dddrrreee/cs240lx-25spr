#include "../lib/rp2350.h"

[[gnu::section(".text.boot_header")]]
static volatile uint32_t BOOT_HEADER[]
    = {
          // block loop goes here
      };

void notmain()
{
    uart_init(uart0, 115200, 150'000'000);
    printk("Booted successfully");

    pico2_reboot();
}
