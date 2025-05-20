#include "rpi.h"
#include "my-fat32-driver.h"

static my_fat32_t my_fat32;
static int my_fat32_initialized = 0;

void my_fat32_init() {
    if (my_fat32_initialized) {
        return;
    }

    kmalloc_init(FAT32_HEAP_MB);
    pi_sd_init();
  
    // printk("Reading the MBR.\n");
    mbr_t *mbr = mbr_read();
  
    // printk("Loading the first partition.\n");
    mbr_partition_ent_t partition;
    memcpy(&partition, mbr->part_tab1, sizeof(mbr_partition_ent_t));
    assert(mbr_part_is_fat32(partition.part_type));
  
    // printk("Loading the FAT.\n");
    my_fat32.fs = fat32_mk(&partition);
  
    // printk("Loading the root directory.\n");
    my_fat32.root = fat32_get_root(&my_fat32.fs);

    my_fat32_initialized = 1;
}

// This is very unique to our Pi setup. `buffer` points to a physical address.
// Neither the caller or this function allocates anything. It just writes to
// the physical address pointed to by `buffer`, and return the number of bytes written.
int my_fat32_read(char *name, char *buffer) {
    if (!my_fat32_initialized) {
        my_fat32_init();
    }

    pi_file_t *file = fat32_read(&my_fat32.fs, &my_fat32.root, name);

    if (!file) {
        printk("%s not found.\n", name);
        return -1;
    }

    memcpy(buffer, file->data, file->n_data);
    return file->n_data;
}

// Helper function; list the names of files at the root directory
// Useful since the name representation on FAT32 might be different from the one we use
void my_fat32_ls() {
    if (!my_fat32_initialized) {
        my_fat32_init();
    }
    fat32_ls(&my_fat32.fs, &my_fat32.root);
}
