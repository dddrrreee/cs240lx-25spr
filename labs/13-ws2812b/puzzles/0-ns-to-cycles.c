#include "rpi.h"

void notmain(void) {
    // the <ws2812b.h> header uses this calculation to go from
    //   nanoseconds (1000 * 1000 * 1000 per second) 
    // to
    //   arm cycles (700Mhz: so 700 * 1000 * 1000 per second) 
    // where we did a GCD of the numerator and denominator.
#   define ns_to_cycles(x) (unsigned) ((x * 7UL) / 10UL )


    // we could instead do no gcd: it's harder to read,
    // but what else goes wrong?
#   define ns_to_cycles_raw(x) \
        (700 * 1000 * 1000UL * x) / (10 * 1000 * 1000UL)

    output("with gcd: %d without: %d\n", 
            ns_to_cycles(300), 
            ns_to_cycles_raw(300));
}
