PROGS := 4-test-vm-cache-mgmt.c
PROGS := dcache-test.c

# map the staff_ names to regular ones.
COMMON_SRC += map-user-to-staff-fn.S

LPI_STAFF_OBJS = $(CS240LX_2025_PATH)/libpi/staff-objs/
STAFF_OBJS += $(LPI_STAFF_OBJS)/kmalloc.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/interrupts-vec-asm.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/staff-pinned-vm.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/staff-mmu-asm.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/staff-mmu.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/staff-breakpoint.o

RUN = 1

include $(CS240LX_2025_PATH)/libpi/mk/Makefile.robust
