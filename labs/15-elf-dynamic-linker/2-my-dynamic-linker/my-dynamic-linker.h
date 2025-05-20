#ifndef __MY_DYNAMIC_LINKER_H__
#define __MY_DYNAMIC_LINKER_H__

#include "rpi.h"
#include "my-elf-loader.h"

// Locate essential sections for relocation and dynamic linking.
// Fill in the convenience struct `my_elf32` with the addresses of the sections.
// We can use the .dynamic section to locate .hash, .dynsym, .dynstr, .got.plt, and .rel.dyn sections.
// Refer to 
void get_dynamic_sections(my_elf32 *e);

// Perform load-time relocation of all the symbols in the 
// .got section of this ELF32 file (Read through relocation tables, .rel.plt and .rel.dyn, 
// jump to the symbol table to find the symbol address, and fill in the appropriate entries)
// This is needed because shared libraries are position-independent and thus do not know the
// symbol addresses at runtime.
void load_time_relocation(my_elf32 *e);

// Given a symbol name and an elf file, find the address of the symbol in the
// shared library loaded into memory.
// A super fun extension: use the .hash section to find the symbol in O(1) time!
uint32_t resolve_symbol(my_elf32 *e, char *symbol_name);

#endif
