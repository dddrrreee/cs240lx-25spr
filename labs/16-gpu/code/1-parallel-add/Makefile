LIBS += $(CS240LX_2025_PATH)/lib/libgcc.a $(CS240LX_2025_PATH)/libpi/libpi.a

COMMON_SRC := mailbox.c 

PROGS := 1-test-add.c


#SWAP THESE
#COMMON_SRC += parallel-add.c
STAFF_OBJS += staff-parallel-add.o

#AND THESE
#COMMON_SRC += addshader.c
STAFF_OBJS += staffaddshader.o

#ALSO SWAP THE INCLUDES IN PARALLEL-ADD.H

STAFF_OBJS += $(CS240LX_2025_PATH)/libpi/staff-objs/staff-hw-spi.o
STAFF_OBJS += $(CS240LX_2025_PATH)/libpi/staff-objs/kmalloc.o

# uncomment if you want it to automatically run.
RUN=1

BOOTLOADER = my-install
EXCLUDE ?= grep -v simple_boot
GREP_STR := 'HASH:\|ERROR:\|PANIC:\|SUCCESS:\|NRF:'
include $(CS240LX_2025_PATH)/libpi/mk/Makefile.robust

checkoff:
	make -f ./Makefile PROGS="$(wildcard tests/[0123]-*.c)" check
	# make -f ./Makefile PROGS="$(ALL)" check