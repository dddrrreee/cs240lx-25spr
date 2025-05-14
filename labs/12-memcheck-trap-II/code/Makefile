# basic memtrace tests
PROGS := tests-memtrace/0-test-basic.c
PROGS += tests-memtrace/1-test-multi-faults.c

# simple purify tests
PROGS := tests-purify/1-purify-bug.c
PROGS := tests-purify/2-purify-bug.c
PROGS := tests-purify/3-purify-free-bug.c
PROGS := tests-purify/4-purify-bug.c
PROGS := tests-purify/5-purify-no-bug.c
PROGS := tests-purify/6-purify-no-bug-slow.c
PROGS := $(wildcard tests-purify/[0123456]-purify-*.c)

# run all tests
PROGS := $(wildcard tests-purify/[0123456]-purify-*.c)
PROGS += tests-memtrace/0-test-basic.c
PROGS += tests-memtrace/1-test-multi-faults.c

# basic common source.
COMMON_SRC += memmap-default.c
COMMON_SRC += sbrk-trap.c

# part 1: switch
# COMMON_SRC += memtrace.c
STAFF_OBJS += staff-memtrace.o

# part 2: switch
# COMMON_SRC += checker-purify.c
STAFF_OBJS += staff-checker-purify.o

# part 3: switch
# COMMON_SRC += staff-ckalloc.c
STAFF_OBJS += staff-ckalloc.o

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

include $(CS240LX_2025_PATH)/libpi/mk/Makefile.robust
