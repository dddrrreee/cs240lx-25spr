PROGS := single-step-example.c

# use a different start so we have routines in known
# locations.
OUR_START = single-step-start.S

LPI_STAFF_OBJS = $(CS240LX_2025_PATH)/libpi/staff-objs/
STAFF_OBJS += $(LPI_STAFF_OBJS)/kmalloc.o
STAFF_OBJS += $(LPI_STAFF_OBJS)/staff-breakpoint.o

BOOTLOADER=my-install
RUN = 1

include $(CS240LX_2025_PATH)/libpi/mk/Makefile.robust-v2
