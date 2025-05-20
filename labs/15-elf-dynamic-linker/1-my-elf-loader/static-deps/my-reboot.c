/*
    Stuart: Very minimal version of reboot functions without dependencies

    The goal is to have as little as possible, and dynamically link the rest.
*/

#include "rpi.h"

void rpi_reboot(void) {
    delay_ms(10);

    const int PM_RSTC = 0x2010001c;
    const int PM_WDOG = 0x20100024;
    const int PM_PASSWORD = 0x5a000000;
    const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;

    PUT32(PM_WDOG, PM_PASSWORD | 1);
    PUT32(PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
    while(1); 
}

void clean_reboot(void) {
    putk("DONE!!!\n"); // without this unix-side doesn't know when to terminate
    uart_flush_tx();
    delay_ms(10);
    rpi_reboot();
}
