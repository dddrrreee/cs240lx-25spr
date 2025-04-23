# if you want to work on one at a time.
PROGS := tests/part1-test0-bug.c     
PROGS := tests/part1-test1-no-bug.c  
PROGS := tests/part1-test2-bug.c     
PROGS := tests/part1-test3-bug.c	
PROGS := tests/part1-test4-bug.c
PROGS := tests/part1-test5-no-bug.c
PROGS := tests/part1-test6-bugs.c

# all the tests.
PROGS := $(wildcard tests/part1-test[0-9]*.c)

#COMMON_SRC = kr-malloc.c
STAFF_OBJS += staff-kr-malloc.o

# COMMON_SRC += ckalloc.c
STAFF_OBJS += staff-ckalloc.o

#COMMON_SRC += ck-gc.c
STAFF_OBJS += staff-ck-gc.o

#COMMON_SRC += gc-asm.S
STAFF_OBJS += staff-gc-asm.o


# define this if you need to give the device for your pi
TTYUSB = 

# set RUN = 1 if you want the code to automatically run after building.
RUN = 1

include $(CS240LX_2025_PATH)/libpi/mk/Makefile.robust-v2

checkopt:
	@echo "going to check opt level: -O:"
	@make -s clean && make -s "OPT_LEVEL=-O" RUN=0
	@make check
	
	@echo "going to check opt level: -O2:"
	@make -s clean && make -s "OPT_LEVEL=-O2" RUN=0
	@make check

	@echo "going to check opt level: -O3:"
	@make -s clean && make -s "OPT_LEVEL=-O3" RUN=0
	@make check

	@echo "going to check opt level: -Ofast:"
	@make -s clean && make -s "OPT_LEVEL=-Ofast" RUN=0
	@make check
