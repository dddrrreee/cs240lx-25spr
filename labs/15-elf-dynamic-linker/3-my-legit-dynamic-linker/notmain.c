#include "rpi.h"
#include "my-legit-dynamic-linker.h"

// Change this to the filename of the dynamically linked ELF file you want to load
static char *exec_filename = "0-DYN.ELF";
static char *exec_base = (char *)0x0;
my_elf32 exec_e;

static char *libpi_filename = "LIBPI.SO";
static char *libpi_base = (char *)0x10000000; // 0x1000'0000 (256MB)
my_elf32 libpi_e;

void notmain() {

    // Extension: all of the below can be done on the pi-side bootloader

    // Load main executable and libpi.so into memory (part 1)
    load_elf(exec_filename, exec_base);
    load_elf(libpi_filename, libpi_base);

    // Verify the ELF headers (part 1)
    elf32_header *exec_e_header = (elf32_header *)exec_base;
    elf32_header *libpi_e_header = (elf32_header *)libpi_base;
    verify_elf(exec_e_header);
    verify_elf(libpi_e_header);

    // Zero-initialize the .bss sections (part 1)
    bss_zero_init(exec_e_header);
    bss_zero_init(libpi_e_header);

    // Locate dynamic sections (part 2)
    exec_e.e_header = exec_e_header;
    exec_e.e_sheaders = (elf32_sheader *)(exec_base + exec_e_header->e_shoff);
    exec_e.e_pheaders = (elf32_pheader *)(exec_base + exec_e_header->e_phoff);
    libpi_e.e_header = libpi_e_header;
    libpi_e.e_sheaders = (elf32_sheader *)(libpi_base + libpi_e_header->e_shoff);
    libpi_e.e_pheaders = (elf32_pheader *)(libpi_base + libpi_e_header->e_phoff);
    get_dynamic_sections(&exec_e);
    get_dynamic_sections(&libpi_e);

    // (Already done for you) Resolve all undefined symbols in the .dynsym section of the 
    // libpi.so. This is needed because some symbols in libpi.so exist in our main executable (e.g., `notmain`),
    // so it must be resolved at load-time.
    load_time_resolve(&exec_e, &libpi_e);

    // Perform load-time relocation for position-independent code (part 2)
    load_time_relocation(&libpi_e);

    // The only step needed for true runtime dynamic linking: fill in .got.plt[2] with the 
    //  address of the dynamic linker entry function.
    // This is the address that the program will jump to when it reaches an unresolved symbol.
    exec_e.e_pltgot[2] = (uint32_t)dynamic_linker_entry_asm;

    // Jump to the entry point of the ELF file. The dynamic linking will automatically happen!
    jump_to_elf_entry(exec_e_header);
}
