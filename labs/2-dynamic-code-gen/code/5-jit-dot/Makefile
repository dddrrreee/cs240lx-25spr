# SUPPORT_OBJS := 

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = my-install

# uncomment if you want it to automatically run.
RUN=1

PROGS = test-pmu.c
COMMON_SRC = cache-support.S  jit-dotproduct.c
PROGS = 2-full-test.c
PROGS += 1-simple-test.c

CFLAGS += -I../armv6-encodings
LIB_POST := $(CS240LX_2025_PATH)/lib/libgcc.a
LIBS += $(LIB_POST)

include $(CS240LX_2025_PATH)/libpi/mk/Makefile.robust-v2
