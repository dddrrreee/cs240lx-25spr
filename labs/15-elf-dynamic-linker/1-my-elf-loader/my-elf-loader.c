#include "rpi.h"

#include "my-fat32-driver.h"
#include "my-elf-loader.h"

// Load the ELF file from the SD card to memory, starting at address `base`
void load_elf(char *filename, char *base) {
    int size = my_fat32_read(filename, base);
    if (size < 0)
        panic("[MY-ELF] Couldn't read ELF file from the FAT32 filesystem\n");
    else
        printk("[MY-ELF] ELF file loaded into memory (%x - %x)\n", base, base + size);
}

// Verify the ELF header. We'll do only a few checks as an exercise.
// Refer to 1-3 of ELF.pdf for the ELF header format
void verify_elf(elf32_header *e_header) {
    // 1. Verify the ELF file magic number, which is 0x7f, 'E', 'L', 'F', in that order
    // Refer to 1-3 and 1-5.
    todo("Verify the ELF file magic number");
    // if (???)
    //     panic("[MY-ELF] Not an ELF file!\n");
    // else
    //     printk("[MY-ELF] ELF file magic number verified\n");

    // 2. Verify that the ELF file is either executable or shared object
    // Refer to 1-3.
    todo("Verify that the ELF file is either executable or shared object");
    // if (???)
    //     panic("[MY-ELF] Not an executable or shared object ELF file!\n");
    // else
    //     printk("[MY-ELF] ELF file type verified\n");

    // 3. Verify that the ELF file is for 32-bit architecture
    // Refer to 1-5 and 1-6.
    todo("Verify that the ELF file is for 32-bit architecture");
    // if (???)
    //     panic("[MY-ELF] Not a 32-bit ELF file!\n");
    // else
    //     printk("[MY-ELF] ELF file architecture verified\n");
}

// Zero-initialize the .bss section
// ** cstart.c no longer does this! This would be a more "legal" way of initializing the bss section **
// In order to do this, we need to:
//   - Find out where the section header table starts from the ELF header
//   - Iterate through the section header table entries until we find the .bss section
//      - Thankfully, the .bss section is the only section that has sh_type == SHT_NOBITS
//        for our case. So we can rely on this for now.
//   - Zero-initialize the .bss section
// Refer to 1-4, 1-9, 1-10, and 1-13
void bss_zero_init(elf32_header *e_header) {
    todo("Zero-initialize the .bss section");
    // elf32_sheader *e_sheaders = ???
    // for (int i = 0; i < ???; i++) {
    //     // .bss section found
    //     if (e_sheaders[i].sh_type == SHT_NOBITS) {
    //         char *bss_start = ???
    //         char *bss_end = ???
    //         memset(bss_start, 0, bss_end - bss_start);
    //         printk("[MY-ELF] BSS section zero-initialized (%x - %x)\n", bss_start, bss_end);
    //         return;
    //     }
    // }
}

// Jump to the ELF file's entry point.
void jump_to_elf_entry(elf32_header *e_header) {
    // Find the entry point of the ELF file.
    // The entry point is the address of the first instruction to execute
    // Refer to 1-4
    todo("Find the entry point of the ELF file");
    // uint32_t entry_point = ???;
    // printk("[MY-ELF] Entry point: %x\n", entry_point);

    // Branch to the entry point. Woohoo!
    todo("Branch to the entry point");
    // printk("[MY-ELF] Branching to the entry point\n");
    // ???
    // panic("[MY-ELF] Shouldn't reach here!\n");
}
