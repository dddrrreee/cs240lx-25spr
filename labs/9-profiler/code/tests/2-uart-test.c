// get rid of some overhead so we see more <uart_can_putc>
// and <GET32> code.
#include "rpi.h"
#include "ss-pixie.h"

void notmain(void) {
    caches_enable();

    pixie_verbose(0);
    pixie_start();
    for(int i = 0; i < 10; i++)  {
        uart_put8('h');
        uart_put8('e');
        uart_put8('l');
        uart_put8('l');
        uart_put8('o');
        uart_put8('\n');
    }
    unsigned n = pixie_stop();

    output("done: %d instructions!\n", n);

    // this should dump out the counts.
    pixie_dump(10);
}
