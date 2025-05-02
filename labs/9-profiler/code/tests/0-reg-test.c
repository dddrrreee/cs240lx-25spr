// roughly test that each register gets saved.
// when <sys_die_handler> gets called:
//   - all regs should be their decimal value except for 
//     the pc
// see: <staff-start.S:terminal_reg_test>
#include "rpi.h"
#include "ss-pixie.h"

// <staff-start.S>
void terminal_reg_test(void);

// <staff-start.S>: pc value where we do the swi call.
// b/c of how we do start, this value should be the 
// same in every binary.
extern uint32_t terminal_reg_test_swi[];

// called from system call handler when code dies.
void pixie_die_handler(uint32_t regs[16]) {
    trace("die handler\n");
    for(int i = 0; i < 16; i++) {
        trace("regs[%d] = %x\n", i, regs[i]);
    
        // all regs should be their decimal value
        // except for the pc
        uint32_t expected = i;
        if(i == 15)
            expected = (uint32_t)terminal_reg_test_swi;

        if(regs[i] != expected)
            panic("expected = %x, got=%x\n", expected, regs[i]);
    }
    trace("success!\n");
}

void notmain(void) {
    pixie_start();
    terminal_reg_test();
    not_reached();
}
