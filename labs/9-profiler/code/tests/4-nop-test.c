// get rid of some overhead so we see more <uart_can_putc>
// and <GET32> code.
#include "rpi.h"
#include "ss-pixie.h"

void nop_10(void);

void notmain(void) {
    pixie_verbose(0);


    caches_enable();
    // run it once to warm up
    nop_10();

    pixie_start();
        nop_10();
    unsigned n = pixie_stop();
    pixie_dump(1);
}

