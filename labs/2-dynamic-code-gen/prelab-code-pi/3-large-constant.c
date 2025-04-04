#include "rpi.h"

// example of how arm loads large values.
uint32_t ret_large(void) {
    return 0x12345678;
}

void notmain(void) {
}
