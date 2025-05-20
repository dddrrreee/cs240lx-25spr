#include "rpi-asm.h"

@  void dynamic_linker_entry_asm(void)
@  - This is the entry point for the dynamic linker
@  - The address of this label is stored in the third entry of .got section at load-time
@  - During runtime, when a program reaches an unresolved symbol, it will eventually 
@    jump to this address (after going through .plt and .got.plt entries)
@  - Registers and stack will hold:
@    - r0~r3: first 4 arguments to the unresolved symbol
@    - r12: address to the .got.plt entry, which should be filled with 
@           the address of the resolved symbol
@    - lr: holds the address to the third entry of the .got.plt section, which holds 
@          the address of the dynamic linker entry function. 
@    - stack: holds one 4B word that is the return address. Should be stored in lr 
@             before jumping to the resolved symbol. 
@             Also holds additional arguments which should be left as-is
.globl dynamic_linker_entry_asm
dynamic_linker_entry_asm:

    push {r0-r11}    @ save argument registers and callee-saved registers 
                     @ (r12 is not needed, lr is already in the stack)

    mov r0, lr       @ first argument to dynamic_linker_entry_c is the address of the third entry 
                     @ of .got.plt
    mov r1, r12      @ second argument to dynamic_linker_entry_c is the address of the unresolve 
                     @ symbol's .got.plt entry

    bl dynamic_linker_entry_c @ after this function call, the symbol is resolved in .got.plt and its 
                              @ address is also stored in r0.
                              @ We just need to restore the other registers and jump to it

    // Note sure if this is needed. But these are self-modifying code sync recipe from the VM lab
    mcr p15, 0, r12, c7, c10, 4 @ DSB
    mcr p15, 0, r12, c7, c5, 6  @ Invalidate BTB
    mcr p15, 0, r12, c7, c10, 4 @ DSB
    mcr p15, 0, r12, c7, c5, 4  @ Prefetch Flush

    mov r12, r0      @ store the resolved symbol address in r12
    pop {r0-r11}     @ restore saved registers
    pop {lr}         @ restore lr, which is put to stack by .plt stub before this function

    bx r12           @ jump to the resolved symbol
