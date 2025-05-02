// this is roughly the gprof example from lab 4
#include "rpi.h"
#include "ss-pixie.h"

void notmain(void) {
    // caches_enable();     // Q: what happens if you enable cache?

    pixie_verbose(0);
    pixie_start();
    for(int i = 0; i < 10; i++) 
        output("%d: hello world\n", i);
    unsigned n = pixie_stop();

    output("done: %d instructions!\n", n);

    // this should dump out the counts.
    pixie_dump(10);
}
