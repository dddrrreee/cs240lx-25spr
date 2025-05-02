// measure the raw overhead of calls.
#include "rpi.h"
#include "ss-pixie.h"

void call_10(void);

void notmain(void) {
    unsigned n;

    pixie_verbose(0);

    output("uncached 10 calls\n");
    pixie_start();
        call_10();
    n = pixie_stop();
    output("%d instructions\n", n);
    pixie_dump(1);
    pixie_reset();  // reset that clears the histogram

    output("cached 10 calls\n");
    caches_enable();
    call_10(); // run it once to warm up

    pixie_start();
        call_10();
    n = pixie_stop();
    output("%d instructions\n", n);
    pixie_dump(1);
}
