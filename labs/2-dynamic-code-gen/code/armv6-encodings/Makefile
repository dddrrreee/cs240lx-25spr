# define this if you need to give the device for your pi
TTYUSB = 

# if you didn't write a bootloader change to pi-install or copy your
# pi-install to my-install
# BOOTLOADER = pi-install
BOOTLOADER = my-install

# RUN=0 if you don't want it to automatically run.
RUN=1

# from easiest to hardest
PROGS = 0-test-bx.c
PROGS = 1-test-imm8.c
PROGS = 2-test-imm8-rot4.c 
PROGS = 2-test-orr-imm8-rot4.c
PROGS = 3-test-imm32.c
PROGS = 4-test-ldr-off12.c

# all of them.
PROGS := $(wildcard [01234]-test*.c)

# CFLAGS = -I../hand-coded

LIB_POST := $(CS240LX_2025_PATH)/lib/libgcc.a
LIBS += $(LIB_POST)

include $(CS240LX_2025_PATH)/libpi/mk/Makefile.robust-v2
