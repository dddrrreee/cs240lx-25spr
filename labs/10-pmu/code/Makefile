# very simple tests. start here.

PROGS := 2-pmu-test.c
PROGS := 1-pmu-test.c
PROGS := 0-pmu-test.c

COMMON_SRC := measure-fns.c
COMMON_SRC += measure-asm.S

# mess with the optimization level and see what happens!
OPT_LEVEL = -O2 -fno-optimize-sibling-calls 
# OPT_LEVEL = -Ofast -fno-builtin-memset

# used to get control reg 1.
LPI_STAFF_OBJS = $(CS240LX_2025_PATH)/libpi/staff-objs/
STAFF_OBJS += $(LPI_STAFF_OBJS)/staff-mmu-asm.o

BOOTLOADER=my-install
RUN = 1

include $(CS240LX_2025_PATH)/libpi/mk/Makefile.robust-v2
