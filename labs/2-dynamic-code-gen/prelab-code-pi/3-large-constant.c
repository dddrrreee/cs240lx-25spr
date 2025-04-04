#include "rpi.h"

// example of how arm loads large values.
//  - sticks the constant at the end of the code
//  - does an ldr off the pc.
//
// so the C code becomes:
//   e59f0000    ldr r0, [pc] 
//   e12fff1e    bx  lr
//   12345678    .word   0x12345678
uint32_t ret_large(void) {
    return 0x12345678;
}







void notmain(void) {
}
