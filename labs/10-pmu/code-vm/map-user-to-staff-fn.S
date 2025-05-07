#include "rpi-asm.h"

MK_FN(pin_set_context)      b staff_pin_set_context
MK_FN(mmu_is_enabled)       b staff_mmu_is_enabled
MK_FN(mmu_enable)           b staff_mmu_enable
MK_FN(mmu_disable)          b staff_mmu_disable
MK_FN(mmu_sync_pte_mods)    b staff_mmu_sync_pte_mods
MK_FN(pin_mmu_init)         b staff_pin_mmu_init
MK_FN(pin_mmu_sec)          b staff_pin_mmu_sec
MK_FN(pin_exists)           b staff_pin_exists
MK_FN(cp15_ctrl_reg1_rd)    b staff_cp15_ctrl_reg1_rd 
