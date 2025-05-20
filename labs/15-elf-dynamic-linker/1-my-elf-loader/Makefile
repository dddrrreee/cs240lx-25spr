# Binaries
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJDUMP = arm-none-eabi-objdump
OBJCOPY = arm-none-eabi-objcopy
READELF = arm-none-eabi-readelf
HEXDUMP = hexdump

# Directories
OBJDIR = ./objs
SRCDIRS += ./
SRCDIRS += ./static-deps

# Flags
CFLAGS = -D__RPI__ -Og -Wall -nostdlib -nostartfiles -ffreestanding \
         -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -std=gnu99 \
         -ggdb -Wno-pointer-sign -Werror -Wno-unused-function -Wno-unused-variable \
         -mno-unaligned-access -mtp=soft
ASFLAGS = -nostdlib -nostartfiles -ffreestanding \
		  -Wa,--warn -Wa,--fatal-warnings -Wa,-mcpu=arm1176jzf-s -Wa,-march=armv6zk
INCLUDES = -I. \
           -I./static-deps \
           -I../0-my-libpi/ \
           -I../0-my-libpi/libc \
           -I../0-my-libpi/include
LDFLAGS = --strip-debug -T memmap

# C/ASM source files
C_SRCS += $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
ASM_SRCS += $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.S))

# Object files
C_OBJS := $(patsubst %.c,$(OBJDIR)/%.o,$(notdir $(C_SRCS)))
ASM_OBJS := $(patsubst %.S,$(OBJDIR)/%.o,$(notdir $(ASM_SRCS)))
OBJS := $(C_OBJS) $(ASM_OBJS)

# Lookup paths
vpath %.c $(SRCDIRS)
vpath %.S $(SRCDIRS)

# Default target
TARGET := notmain
all: $(TARGET)
	pi-install ./$(TARGET).bin

# Link all object files into the final executable
$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET).elf ../0-my-libpi/libpi.a
	$(OBJCOPY) $(TARGET).elf -O binary $(TARGET).bin
	$(OBJDUMP) -d $(TARGET).elf > $(TARGET).list

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
	rm -rf $(OBJDIR) $(TARGET) $(TARGET).elf $(TARGET).hexdump $(TARGET).list $(TARGET).readelf $(TARGET).bin
