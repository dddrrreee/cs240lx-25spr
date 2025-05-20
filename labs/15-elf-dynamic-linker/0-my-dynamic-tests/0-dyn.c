#include "rpi.h"

static int bss_var;

void notmain() {
    printk("BSS var: %d\n", bss_var);
    printk("Hello, world!\n");
    clean_reboot();
}
