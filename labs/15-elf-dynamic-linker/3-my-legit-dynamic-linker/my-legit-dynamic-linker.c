#include "rpi.h"
#include "my-legit-dynamic-linker.h"

// From notmain.c
extern my_elf32 exec_e;
extern my_elf32 libpi_e;

// Resolve all undefined symbols in the .dynsym section of the given ELF32 file at load-time.
// This is needed because some symbols in a dynamic library might exist in our main executable (e.g., `notmain`)
void load_time_resolve(my_elf32 *exec_e, my_elf32 *dyn_e) {
    printk("[MY-DL] Resolving undefined symbols in shared library...\n");

    // Best-effort attempt to resolve undefined symbols in the .dynsym section
    uint32_t n_symbols = dyn_e->e_hash[1]; // this is equivalent to the number of symbols in .dynsym
    for (int i = 0; i < n_symbols; i++) {
        if (dyn_e->e_dynsym[i].st_shndx == SHN_UNDEF && dyn_e->e_dynsym[i].st_name) { // symbol is undefined
            char *symbol_name = dyn_e->e_dynstr + dyn_e->e_dynsym[i].st_name;
            // printk("[MY-DL] Undefined symbol: %s\n", symbol_name);
            // printk("[MY-DL] resolved addr: %x\n", resolve_symbol(&main_elf, symbol_name));
            dyn_e->e_dynsym[i].st_value = resolve_symbol(exec_e, symbol_name);
        }
    }
}

// Called by my_dl_entry_asm
// gotplt: the address of the third entry of .got.plt
// gotplt_entry: the address of the unresolved symbol's .got.plt entry
// Performs dynamic linking and resolves the symbol, saves its address in gotplt_entry
// Returns the address of the resolved symbol
uint32_t dynamic_linker_entry_c(uint32_t *gotplt, uint32_t *gotplt_entry) {

    // Useful for debugging: read what is in stack
    // volatile uint32_t *sp;
    // asm volatile("mov %0, sp" : "=r"(sp));

    // Useful for debugging: print out the values of gotplt and gotplt_entry
    // printk("[MY-DL] gotplt: %x, gotplt_entry: %x\n", gotplt, gotplt_entry);
    // printk("[MY-DL] *gotplt: %x, *gotplt_entry: %x\n", *gotplt, *gotplt_entry);

    // The first entry of .got.plt is the address to .dynamic section
    // elf32_dynamic *e_dynamics = (elf32_dynamic *)(*(gotplt - 2));
    // ...But we can just use our utility struct my_elf32

    // The recipe:
    //   1. Find the unresolved symbol index in `.rel.dyn` section. Hint: since we already know `&GOT[2]` and `&GOT[i+3]`, we can subtract these addresses to find `i`.
    //   2. Retrieve the `.dynsym` index from the `.rel.dyn` entry. You can do this by taking the relocation entry's INFO field and bitwise shifting it to the right by 8 (the first 8 bits hold other data).
    //   3. Retrieve symbol's string representation from the `.dynstr` section.
    //   4. Call `resolve_symbol` function (from part 2) to retrieve the symbol address.
    //   5. Fill in `GOT[i+3]` with symbol address
    //   6. Return the symbol address
    todo("Do dynamic linking");
    // char *symbol_name = ???;
    // printk("[MY-DL] Dynamic linker: Unresolved symbol encountered: <%s>. Dynamic linker invoked.\n", symbol_name);

    // Try to resolve the symbol and fill in the got table entry
    // uint32_t symbol_addr = resolve_symbol(&libpi_e, symbol_name);
    // *gotplt_entry = symbol_addr;
    // printk("[MY-DL] Dynamic linker: Resolved symbol %s to %x\n", symbol_name, symbol_addr);

    // return symbol_addr; // the rest should be handled by asm
    return 0;
}
