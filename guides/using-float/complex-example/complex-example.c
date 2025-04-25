// trivial example of using floating point + our simple math
// library.
#include "rpi.h"
#include "rpi-math.h"
#include <complex.h>

void notmain(void) {
    double complex c = CMPLX(2,3);
    printk("hello from pi=(%f,%f) float!!\n", creal(c), cimag(c));
}
