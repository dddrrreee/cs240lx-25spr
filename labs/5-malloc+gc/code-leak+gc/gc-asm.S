#include "rpi-asm.h"

@ trampoline used to:
@   - save all callee saved registers to stack.
@   - call ck_gc_fn(sp)
@   - return
@
@  note: also have to save/restore lr
MK_FN(ck_gc)
    asm_todo("implement this\n");

@ similar trampoline as above, but will call
@ <ck_find_leaks_fn>.  note: sp is in a different
@ register
MK_FN(ck_find_leaks)
    asm_todo("implement this\n");
