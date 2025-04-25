PROGS = fp-example.c

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = my-install

# uncomment if you want it to automatically run.
RUN=1

# add these two if you want to use floating point.
LIBS += $(LIBM)
CFLAGS += $(LIBM_INC)

include $(CS240LX_2025_PATH)/libpi/mk/Makefile.robust-v2
