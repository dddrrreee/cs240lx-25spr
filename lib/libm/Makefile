# our math library.
TARGET= libm-pi.a

# note: we are only compiling a small number of the possible routines in this 
# directory.   it's usually just a matter of adding the .o here (possibly pulling
# in additional .o dependencies).   we do it conservatively to attempt to minimize
# bad interactions with libpi.
LIB_OBJS = s_cos.o k_cos.o k_sin.o e_rem_pio2.o k_rem_pio2.o s_scalbn.o s_floor.o s_copysign.o s_fabs.o s_sin.o s_atan.o e_atan2.o s_round.o e_sqrt.o e_atan2f.o e_asinf.o s_fabsf.o s_atanf.o e_sqrtf.o s_cosf.o k_cosf.o k_sinf.o

LIB_OBJS += e_powf.o e_pow.o
LIB_OBJS += s_sinf.o
LIB_OBJS += s_scalbnf.o
LIB_OBJS += s_copysignf.o
LIB_OBJS += e_rem_pio2f.o
LIB_OBJS += s_cimag.o  s_cimagf.o  s_cimagl.o
LIB_OBJS += s_creal.o  s_crealf.o  s_creall.o
LIB_OBJS += e_exp.o   # k_exp.o  #   s_cexp.o  




# k_rem_pio2.o
SRC = $(LIB_OBJS:.o=.c)

USE_FP=1

# add all the .h's as dependencies, libpi and the makefile.
DEPS := $(wildcard ./*.h) ./Makefile $(LPI)

include $(CS240LX_2025_PATH)/libpi/defs.mk

#ARM = arm-none-eabi
#CC = $(ARM)-gcc
#LD  = $(ARM)-ld
#AS  = $(ARM)-as
#OCP = $(ARM)-objcopy

# CFLAGS=  -fno-builtin -mhard-float -Og -nostdlib -nostartfiles -ffreestanding  -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mhard-float -mfpu=vfp -march=armv6zk -std=c99 -Wall -Iinclude -I. -D__BSD_VISIBLE -Wno-implicit-function-declaration

CFLAGS += -Iinclude

all : $(TARGET)

protos: 
	cproto -I. -Iinclude $(SRC) > math-protos.h

.c.o: $(DEPS)
	$(CC) $(CFLAGS) -c $< 

$(TARGET): $(LIB_OBJS)  $(DEPS)
	$(AR) crf $(TARGET) $^ 

clean :
	rm -f $(LIB_OBJS) *~ $(TARGET)

.PHONY: clean protos 

