PROGS := trap-example.c

INC += -I./includes

LPI_STAFF_OBJS = $(CS240LX_2025_PATH)/libpi/staff-objs/
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
