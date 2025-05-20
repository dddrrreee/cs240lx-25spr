# Binaries
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJDUMP = arm-none-eabi-objdump
OBJCOPY = arm-none-eabi-objcopy
READELF = arm-none-eabi-readelf
HEXDUMP = hexdump

# Directories
SRCDIRS += ./
OBJDIR = ./objs

# Flags
CFLAGS = -D__RPI__ -Og -Wall -nostdlib -nostartfiles -ffreestanding \
         -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -std=gnu99 \
         -ggdb -Wno-pointer-sign -Werror -Wno-unused-function -Wno-unused-variable \
         -mno-unaligned-access -mtp=soft
ASFLAGS = -nostdlib -nostartfiles -ffreestanding \
		  -Wa,--warn -Wa,--fatal-warnings -Wa,-mcpu=arm1176jzf-s -Wa,-march=armv6zk
INCLUDES = -I. \
           -I../0-my-libpi/ \
           -I../0-my-libpi/src \
           -I../0-my-libpi/libc \
           -I../0-my-libpi/include
LDFLAGS = --strip-debug -T memmap

# Files
SRCS += $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
OBJS := $(patsubst %.c,$(OBJDIR)/%.o,$(notdir $(SRCS)))
ELFS := $(patsubst $(OBJDIR)/%.o, %.elf, $(OBJS))

# Lookup paths
vpath %.c $(SRCDIRS)
vpath %.S $(SRCDIRS)

all: $(ELFS)

# Link each object file into its own ELF file
%.elf: $(OBJDIR)/%.o $(OBJDIR)/start.o
	$(LD) $(LDFLAGS) $< $(OBJDIR)/start.o -o $@ ../0-my-libpi/libpi.a
	$(READELF) -a $@ > $@.readelf
	$(OBJDUMP) -D $@ > $@.list
	$(HEXDUMP) -C $@ > $@.hexdump

# Pattern rule for compiling C source files to object files
$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/start.o: start.S
	@mkdir -p $(OBJDIR)
	$(CC) $(ASFLAGS) $(INCLUDES) -c $< -o $@

# Clean up generated files
clean:
	rm -rf $(OBJDIR) *.elf *.bin *.hexdump *.list *.readelf
