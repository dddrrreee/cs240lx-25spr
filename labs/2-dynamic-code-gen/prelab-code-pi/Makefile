# SUPPORT_OBJS := 

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = my-install

# uncomment if you want it to automatically run.
RUN=1

PROGS := 2-bug-self-modify.c
PROGS += 0-ident-dcg.c
PROGS += 1-self-modifying.c
PROGS += 4-derive-pc-reg.c
PROGS += 5-derive-add.c

include ../../../libpi/mk/Makefile.robust-v2
