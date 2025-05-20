/*
    Stuart: Very minimal version of cstart without dependencies

    The goal is to have as little as possible, and dynamically link the rest.
*/

#include "rpi.h"

void notmain();
extern uint32_t  __bss_start__[];
extern uint32_t  __bss_end__[];
extern uint32_t  __heap_start__[];

void *program_end(void) { return __heap_start__; }

void _cstart() {

    // Zero out the .bss section
    gcc_mb();
    uint8_t * bss      = (void*)__bss_start__;
    uint8_t * bss_end  = (void*)__bss_end__;
    while( bss < bss_end ) *bss++ = 0;
    gcc_mb();

    // Initialize the UART device
    // TODO: use dynamic linking for this
    uart_init();

    // call user's <notmain> (should add argv)
    notmain(); 

    // if they return and don't exit, just reboot
	clean_reboot();
}
