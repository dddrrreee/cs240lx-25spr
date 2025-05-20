#ifndef __MY_FAT32_H__
#define __MY_FAT32_H__

#include "rpi.h"
#include "fat32.h"

typedef struct {
    fat32_fs_t fs;
    pi_dirent_t root;
} my_fat32_t;

void my_fat32_init();

// This is very unique to our Pi setup. `buffer` points to a physical address.
// Neither the caller or this function allocates anything. It just writes to
// the physical address pointed to by `buffer`, and return the number of bytes written.
int my_fat32_read(char *name, char *buffer);

// Helper function; list the names of files at the root directory
// Useful since the name representation on FAT32 might be different from the one we use
void my_fat32_ls();

#endif
