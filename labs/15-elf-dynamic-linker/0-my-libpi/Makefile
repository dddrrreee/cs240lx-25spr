# Binaries
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
AR = arm-none-eabi-ar
OBJDUMP = arm-none-eabi-objdump
READELF = arm-none-eabi-readelf
HEXDUMP = hexdump

# Directories
OBJDIR = ./objs
SRCDIRS += ./src
SRCDIRS += ./libc 
SRCDIRS += ./staff-src
STAFF_OBJDIR += ./staff-objs
# STAFF_OBJDIR += ./staff-objs/fp

# Flags
CFLAGS = -D__RPI__ -Og -Wall -nostdlib -nostartfiles -ffreestanding \
         -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -std=gnu99 \
         -ggdb -Wno-pointer-sign -Werror -Wno-unused-function -Wno-unused-variable \
         -mno-unaligned-access -mtp=soft -fPIC
ASFLAGS = -nostdlib -nostartfiles -ffreestanding \
		  -Wa,--warn -Wa,--fatal-warnings -Wa,-mcpu=arm1176jzf-s -Wa,-march=armv6zk -fPIC
INCLUDES = -I. \
           -I./src \
           -I./libc \
           -I./include \
           -I./staff-src
LDFLAGS = -shared --strip-debug -T memmap

# C/ASM source files
C_SRCS += $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
ASM_SRCS += $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.S))

# Object files
C_OBJS := $(patsubst %.c,$(OBJDIR)/%.o,$(notdir $(C_SRCS)))
ASM_OBJS := $(patsubst %.S,$(OBJDIR)/%.o,$(notdir $(ASM_SRCS)))
STAFF_OBJS := $(foreach dir, $(STAFF_OBJDIR), $(wildcard $(dir)/*.o))
OBJS := $(C_OBJS) $(ASM_OBJS) $(STAFF_OBJS)

# Lookup paths
vpath %.c $(SRCDIRS)
vpath %.S $(SRCDIRS)

# Default target
all: libpi.so

# Link all object files into the final executable
libpi.so: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o libpi.so
	$(AR) crf libpi.a $(OBJS)
	$(READELF) -a libpi.so > libpi.so.readelf
	$(OBJDUMP) -D libpi.so > libpi.so.list
	$(HEXDUMP) -C libpi.so > libpi.so.hexdump

# Pattern rule for compiling C source files to object files
$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
#   Uncomment to see the object file ELF, disassembly, and hexdump
#   $(READELF) -a $@ > $@.readelf
#   $(OBJDUMP) -D $@ > $@.list
#   $(HEXDUMP) -C $@ > $@.hexdump

# Pattern rule for compiling assembly source files to object files
$(OBJDIR)/%.o: %.S
	@mkdir -p $(OBJDIR)
	$(CC) $(ASFLAGS) $(INCLUDES) -c $< -o $@
#   Uncomment to see the object file ELF, disassembly, and hexdump
#   $(READELF) -a $@ > $@.readelf
#   $(OBJDUMP) -D $@ > $@.list
#   $(HEXDUMP) -C $@ > $@.hexdump

# Clean up generated files
clean:
	rm -rf $(OBJDIR) *.a *.so *.hexdump *.list *.readelf
