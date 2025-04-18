
PROGS := tests/gc-test6-no-leak.c
PROGS := tests/gc-test5-gc.c
PROGS := tests/gc-test4-no-leak.c  
PROGS := tests/gc-test2-no-leak.c  
PROGS := tests/gc-test1-no-leak.c  
PROGS := $(wildcard tests/gc-test[0-9]*.c)


# you should  comment out all staff objs and replace with your code.

#COMMON_SRC = kr-malloc.c
STAFF_OBJS += staff-kr-malloc.o

# COMMON_SRC += ckalloc.c
STAFF_OBJS += staff-ckalloc.o

#COMMON_SRC += ck-gc.c
STAFF_OBJS += staff-ck-gc.o

#COMMON_SRC += gc-asm.S
STAFF_OBJS += staff-gc-asm.o

RUN=1
# if you get issues, try swapping this.
# OPT_LEVEL = -O0
OPT_LEVEL = -O2
CFLAGS += -fno-inline

#LIB_POST := ../armv6-encodings/libgcc.a
#LIBS += $(LIB_POST)

EXCLUDE ?= grep -v simple_boot
include ../../../libpi/mk/Makefile.robust-v2
