#include "rpi.h"
#include "ss-pixie.h"

void nop_10(void);

void notmain(void) {
    pixie_verbose(1);
    pixie_start();
    nop_10();
    unsigned n = pixie_stop();
    trace("done: traced [%d] instructions\n", n);
}
