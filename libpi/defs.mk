# setup useful variables that can be used by make.

# this must be defined in your shell's startup file.
ifndef CS240LX_2025_PATH
$(error CS240LX_2025_PATH is not set: this should contain the absolute path to where this directory is.  Define it in your shell's initialiation.  For example, .tcshrc for tcsh or .bashrc for bash)
endif

# OPT_LEVEL = -O3
ARM = arm-none-eabi
CC = $(ARM)-gcc
LD  = $(ARM)-ld
AS  = $(ARM)-as
AR = $(ARM)-ar
OD  = $(ARM)-objdump
OCP = $(ARM)-objcopy
CS240LX_2025_LIBPI_PATH = $(CS240LX_2025_PATH)/libpi
LPP = $(CS240LX_2025_LIBPI_PATH)
LPI ?= $(LPP)/libpi.a
LGCC ?= $(CS240LX_2025_PATH)/lib/libgcc.a

# let the client override these.
START ?= $(LPP)/staff-start.o
DEFAULT_START := $(LPP)/staff-start.o
MEMMAP ?= $(LPP)/memmap

LIBM_DIR ?=  $(CS240LX_2025_PATH)/lib/libm/
LIBM ?=  $(LIBM_DIR)/libm-pi.a
LIBM_INC ?=  -I$(LIBM_DIR) -I$(LIBM_DIR)/include/


# include path: user can override
INC += -I. -I$(LPP)/include -I$(LPP)/ -I$(LPP)/src  -I$(LPP)/libc # -I$(LPP)/staff-private
# optimization level: client can override
OPT_LEVEL ?= -Og

# various warnings.  any warning = error, so you must fix it.
CFLAGS += -D__RPI__ $(OPT_LEVEL) -Wall -nostdlib -nostartfiles -ffreestanding -mcpu=arm1176jzf-s -mtune=arm1176jzf-s  -std=gnu99 $(INC) -ggdb -Wno-pointer-sign  -Werror  -Wno-unused-function -Wno-unused-variable 

# -ffunction-sections -fdata-sections -Wl,--gc-sections

# LDFLAGS += --gc-sections
LDFLAGS += 

ifdef CFLAGS_EXTRA
CFLAGS += $(CFLAGS_EXTRA)
endif


# workaround for gcc struct assignment bug 
CFLAGS += -mno-unaligned-access

# disable the thread-local pointer coprocessor register (for Thumb support)
CFLAGS += -mtp=soft

# for backtrace
# CFLAGS += -fno-omit-frame-pointer -mpoke-function-name -DBACKTRACE


# for assembly file compilation.
# ASFLAGS = --warn --fatal-warnings  -mcpu=arm1176jzf-s -march=armv6zk $(INC)

# for .S compilation so we can use the preprocessor.
CPP_ASFLAGS =  -nostdlib -nostartfiles -ffreestanding   -Wa,--warn -Wa,--fatal-warnings -Wa,-mcpu=arm1176jzf-s -Wa,-march=armv6zk   $(INC)


CFLAGS += -DRPI_FP_ENABLED  -mhard-float -mfpu=vfp 
CPP_ASFLAGS += -DRPI_FP_ENABLED  -mhard-float -mfpu=vfp
