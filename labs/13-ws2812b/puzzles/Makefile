# do this last.
PROGS = 0-ns-to-cycles.c
PROGS += 1-const-problems.c
PROGS += 2-weird-timing.c

# not really a puzzle, but an example of how to do.
PROGS += cycles-per-sec.c

# worth playing with different ones.
OPT_LEVEL = -Ofast
OPT_LEVEL = -O3
OPT_LEVEL = -O2


RUN = 1

# in case divide, pull in libgcc.
L := ../../../libpi/
LIB_POST := $(L)/../lib/libgcc.a
LIBS += $(LIB_POST)

include $(L)/mk/Makefile.robust
