#ifndef __MY_LEGIT_DYNAMIC_LINKER_H__
#define __MY_LEGIT_DYNAMIC_LINKER_H__

#include "rpi.h"
#include "my-dynamic-linker.h"

// Resolve all undefined symbols in the .dynsym section of the given ELF32 file at load-time.
// This is needed because some symbols in a dynamic library might exist in our main executable (e.g., `notmain`)
void load_time_resolve(my_elf32 *exec_e, my_elf32 *dyn_e);

// - This is the entry point for the dynamic linker
// - The address of this label is stored in the third entry of .got section at load-time
// - During runtime, when a program reaches an unresolved symbol, it will eventually 
//   jump to this address (after going through .plt and .got.plt entries)
// - Registers and stack will hold:
//   - r0~r3: arguments to the unresolved symbol
//   - r12: address to the .got.plt entry that should be filled with 
//          the address of the resolved symbol
//   - lr: holds the address to the third entry of the .got.plt section, which holds 
//         the address of the dynamic linker entry function. 
//   - stack: holds one 4B word that is the return address. Should be stored in lr 
//            before jumping to the resolved symbol
void dynamic_linker_entry_asm(void);

// Called by my_dl_entry_asm
// gotplt: address of the third entry of .got.plt
// gotplt_entry: address of the unresolved symbol's .got.plt entry
// Performs dynamic linking and resolves the symbol, saves its address in gotplt_entry
// Returns the address to the resolved symbol
uint32_t dynamic_linker_entry_c(uint32_t *gotplt, uint32_t *gotplt_entry);

#endif
