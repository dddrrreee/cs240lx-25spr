# very simple tests. start here.

PROGS := tests/0-nop-test.c
PROGS += tests/0-prof-test.c
PROGS += tests/0-reg-test.c

# when you want to start profiling
# PROGS := tests/1-prof-test.c

COMMON_SRC := ss-pixie.c
COMMON_SRC += ss-pixie-asm.S

# use a different start so we have routines in known
# locations.
OUR_START = staff-start.S

# you can swap in your 140e
STAFF_OBJS += staff-breakpoint.o

BOOTLOADER=my-install
RUN = 1
# CAN_EMIT=1

EXCLUDE ?= grep -v simple_boot
include $(CS240LX_2025_PATH)/libpi/mk/Makefile.robust-v2
