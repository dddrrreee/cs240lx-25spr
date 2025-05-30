#include "support.h"

size_t strlen(const char* p)
{
    size_t ret;
    for (ret = 0; *p; p++, ret++)
        ;
    return ret;
}

void* memset(void* p, int v, size_t s)
{
    char* p2 = p;
    for (; s; s--) {
        *p2++ = v;
    }
    return p;
}

[[gnu::naked, gnu::section(".text.boot")]]
void _start()
{
    __asm__(
        // --- Load global pointer
        ".option push\n"
        ".option norelax\n"
        "la gp, __global_pointer$\n"
        ".option pop\n"

        // --- Disable interrupts
        "csrw mie, zero\n"
        "csrci mstatus, 8\n"

        // --- Zero BSS
        "la t0, __bss_start__\n"
        "la t1, __bss_end__\n"
        "1:\n"
        "bgeu t0, t1, 2f\n"
        "sw zero, 0(t0)\n"
        "addi t0, t0, 4\n"
        "j 1b\n"
        "2:\n"

        // --- Initialize stack and frame pointer
        "la sp, __stack_top__\n"
        "andi sp, sp, -16\n"
        "add fp, sp, zero\n"

        // --- Jump to notmain
        "j notmain\n");
}
