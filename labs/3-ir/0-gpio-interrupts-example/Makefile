# to use your gpio-int.c swap these.

# uncomment this to use our code and comment out yours.
L := $(CS240LX_2025_PATH)/libpi/

STAFF_OBJS += $(L)/staff-objs/gpio-int.o

PROGS = simple-gpio-int.c

COMMON_SRC += interrupt-vec.c interrupt-asm.S 

include $(L)/mk/Makefile.robust
