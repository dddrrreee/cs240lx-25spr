LIBS += $(CS240LX_2025_PATH)/lib/libgcc.a $(CS240LX_2025_PATH)/libpi/libpi.a

COMMON_SRC := mailbox.c

PROGS := 2-mandelbrot.c

# PROGS := tests/3-test-fire.c

COMMON_SRC += mandelbrotshader.c mandelbrot-helpers.c

# COMMON_SRC += fat32/code/pi-sd.c fat32/code/mbr-helpers.c fat32/code/fat32-helpers.c fat32/code/fat32-lfn-helpers.c fat32/code/external-code/unicode-utf8.c fat32/code/external-code/emmc.c fat32/code/fat32.c fat32/code/mbr.c

LIBS += ./libfat32.a

STAFF_OBJS += $(CS240LX_2025_PATH)/libpi/staff-objs/staff-hw-spi.o
STAFF_OBJS += $(CS240LX_2025_PATH)/libpi/staff-objs/kmalloc.o

# uncomment if you want it to automatically run.
RUN=1

BOOTLOADER = my-install
include $(CS240LX_2025_PATH)/libpi/mk/Makefile.robust

clean::
	rm -f mandelbrotshader.[hc]

