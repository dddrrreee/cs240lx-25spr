PROGS := $(wildcard tests-eraser/0-eraser-test[12345]*.c)
PROGS += $(wildcard tests-eraser/1-eraser-test[12345]*.c)

# these require the dumb state machine.  careful about 
# the comments, i didn't double check.
COMMON_SRC += checker-eraser.c
PROGS := tests-eraser/0-eraser-test4.c
PROGS := tests-eraser/0-eraser-test3-bug.c
PROGS := tests-eraser/0-eraser-test2-bug.c
PROGS := tests-eraser/0-eraser-test1.c

# these require a sophisticated state machine:
#   STAFF_OBJS += staff-checker-eraser.o
# PROGS := tests-eraser/1-eraser-test0.c
# PROGS := tests-eraser/1-eraser-test1.c
# PROGS := tests-eraser/1-eraser-test2.c
# PROGS := tests-eraser/1-eraser-test3-bug.c 
# PROGS := tests-eraser/1-eraser-test4-bug.c 


# where memtrace is.
MEMT_DIR = $(CS240LX_2025_PATH)/labs/12-memcheck-trap-II/code


CFLAGS += -I$(MEMT_DIR) -I$(MEMT_DIR)/includes/

# basic common source.
COMMON_SRC += $(MEMT_DIR)/memmap-default.c
COMMON_SRC += $(MEMT_DIR)/sbrk-trap.c
# COMMON_SRC += $(MEMT_DIR)/memtrace.c
STAFF_OBJS += $(MEMT_DIR)/staff-memtrace.o

# part 3: switch
# COMMON_SRC += staff-ckalloc.c
STAFF_OBJS += $(MEMT_DIR)/staff-ckalloc.o

INC += -I./includes

LPI_STAFF_OBJS = $(CS240LX_2025_PATH)/libpi/staff-objs/
STAFF_OBJS += $(LPI_STAFF_OBJS)/staff-watchpoint.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/kmalloc.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/interrupts-vec-asm.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/staff-pinned-vm.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/staff-mmu-asm.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/staff-mmu.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/staff-armv6-except.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/staff-breakpoint.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/map-user-to-staff-fn.o

RUN = 1

include $(CS240LX_2025_PATH)/libpi/mk/Makefile.robust-v2
