// print out the string in the header.
#include "rpi.h"

void notmain(void) {

    // look in:
    // - <libpi/memmap> for an example of how to define a symbol.
    // - <libpi/include/memmap.h> for an example of how to reference the
    //   symbol from C code.
    todo("add a symbol to the header before `hello` that you can reference and print");

    // set this to the symbol.
    const char *header_string = 0;

    assert(header_string);
    printk("<%s>\n", header_string);
    printk("success!\n");
}
