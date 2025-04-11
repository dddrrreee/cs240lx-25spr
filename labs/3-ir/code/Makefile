# list out the driver program source 
PROGS := 0-dump-driver.c 

# if you have auxilary files, add them here.
COMMON_SRC :=


# for extensions if need.
L := ../../../libpi/
STAFF_OBJS += $(L)/staff-objs/staff-rpi-thread.o
STAFF_OBJS += $(L)/staff-objs/staff-rpi-thread-asm.o

# set RUN = 0 if you don't want the code to automatically run after building.
RUN = 1
OPT_LEVEL = -O2

# in case divide, pull in libgcc.
LIB_POST := $(L)/../lib/libgcc.a
LIBS += $(LIB_POST)

include $(L)/mk/Makefile.robust
