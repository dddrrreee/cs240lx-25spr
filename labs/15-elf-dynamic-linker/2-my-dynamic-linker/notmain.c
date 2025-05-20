#include "rpi.h"
#include "my-dynamic-linker.h"

static char *libpi_filename = "LIBPI.SO";
static char *libpi_base = (char *)0x10000000; // 0x1000'0000 (256MB)

void notmain() {

    // Extension: all of the below can be done on the pi-side bootloader

    // Load libpi.so into memory (use the code from part 1)
    load_elf(libpi_filename, libpi_base);

    // Now libpi.so is fully loaded in memory, starting at 0x0
    // We can now start parsing it

    // Verify the ELF header (use the code from part 1)
    elf32_header *e_header = (elf32_header *)libpi_base;
    verify_elf(e_header);

    // Zero-initialize the .bss section (use the code from part 1)
    bss_zero_init(e_header);

    // Step 1. Locate all the essential sections related to dynamic linking
    my_elf32 e;
    e.e_header = e_header;
    e.e_sheaders = (elf32_sheader *)(libpi_base + e_header->e_shoff);
    e.e_pheaders = (elf32_pheader *)(libpi_base + e_header->e_phoff);
    get_dynamic_sections(&e);

    // Step 2. (already done for you) Perform load-time relocation of all the symbols in the 
    // .got section of this ELF32 file (Read through relocation tables, .rel.plt and .rel.dyn, 
    // jump to the symbol table to find the symbol address, and fill in the appropriate entries)
    // This is needed because libpi.so is a position-independent code and thus does not know the
    // symbol addresses during runtime.
    load_time_relocation(&e);

    // Step 3. Find the address of the symbol through the .dynsym section.
    // A super fun extension: use the .hash section to find the symbol in O(1) time!
    char *symbol_to_find = "printk";
    uint32_t symbol_addr = resolve_symbol(&e, symbol_to_find);

    // Step 4. Jump to the symbol!
    // Cast symbol_addr to function pointer of type int (*)(const char *, ...),
    // which is same as that of printk.
    // Or you can just use inline assembly: prepare inputs and jump to the symbol address directly
    int (*resolved_func)(const char *, ...) = (int (*)(const char *, ...))symbol_addr;
    resolved_func("Hello from the resolved symbol!!!\n");
    printk("Should reach here as well!\n");
}
