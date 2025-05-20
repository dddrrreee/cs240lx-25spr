/*
    A very simple sanity check on ELF parser and loader.
    This program will load a simple ELF file, and branch to its entry point.

    Works on the standard 140E bootloaders. No need for custom bootloaders.
*/
#include "rpi.h"

#include "my-fat32-driver.h"
#include "my-elf-loader.h"

// Change this to the filename of the statically linked ELF file you want to load
static char *exec_filename = "0-STATIC.ELF";
static char *exec_base = (char *)0x0;  // Must load the ELF file to 0x0, as it is position-dependent

void notmain() {

    // Use this helper function to list the filenames on the SD card
    // Note that FAT32 will create alias dirents for our files (ex. _0-DY~37.ELF).
    // We can ignore them and just use the original file names
    // Remember to keep the filenames in all caps (ex. 1-STATIC.ELF, not 1-static.elf)
    // my_fat32_ls();

    // Extension: all of the below can be done on the pi-side bootloader

    // Load the ELF file from the SD card to memory
    load_elf(exec_filename, exec_base);

    // Now our ELF file is fully loaded in memory, starting at 0x0
    // We can now start parsing it

    // Step 1. Verify the ELF header. We'll do only a few checks as an exercise.
    elf32_header *e_header = (elf32_header *)exec_base;
    verify_elf(e_header);

    // Enough checking for now. Let's move on

    // Step 2. bss section needs to be zero-initialized (per C standards)
    // ** cstart.c no longer does this! This would be a more "legal" way of initializing the bss section **
    // In order to do this, we need to:
    //   - Find out where the section header table starts from the ELF header
    //   - Iterate through the section header table entries until we find the .bss section
    //      - Thankfully, the .bss section is the only section that has sh_type == SHT_NOBITS
    //        for our case. So we can rely on this for now.
    //   - Zero-initialize the .bss section
    // Refer to 1-4, 1-9, 1-10, and 1-13
    bss_zero_init(e_header);

    // Step 3. Jump to the entry point of the ELF file.
    jump_to_elf_entry(e_header);
}
