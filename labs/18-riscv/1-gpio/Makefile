.SUFFIXES:
.DEFAULT_GOAL := run

FLASH_MOUNT := /Volumes/RP2350
# FLASH_MOUNT := /mnt/pico

INSTALL := pi-install
ENGRAVE_UF2 := ../bin/engrave-uf2.macos

COMMON_SRC := gpio.c
COMMON_SRC += printk.c

# --- FOR PART 2:
# COMMON_SRC += uart.c
# --- END FOR

CC := riscv64-unknown-elf-gcc
LD = riscv64-unknown-elf-ld
OBJCOPY := llvm-objcopy

COMMON_OBJS := $(COMMON_SRC:.c=.o) lib.o

MEMMAP := ../pico2.ld

HEADERS := ../lib/rp2350.h ../lib/support.h
LIBPATH := ../libgcc-riscv64-elf-15.1.0

CFLAGS := -march=rv32imac_zicsr -mabi=ilp32 -c -std=gnu23 -O2 -ffreestanding -nostdlib
LDFLAGS := -melf32lriscv -T$(MEMMAP) --gc-sections -z noexecstack -flto=thin -static -lgcc -L$(LIBPATH)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

lib.o: ../lib/core.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

part1.elf: part1.o staff-uart.o $(COMMON_OBJS) | $(MEMMAP)
	$(LD) $(LDFLAGS) $^ -o $@

part2.elf: part2.o staff-uart.o $(COMMON_OBJS) | $(MEMMAP)
	$(LD) $(LDFLAGS) $^ -o $@

part3.elf: part2.o uart.o $(COMMON_OBJS) | $(MEMMAP)
	$(LD) $(LDFLAGS) $^ -o $@

part3.uf2: part3.elf
	$(ENGRAVE_UF2) $< -o $@

%.bin: %.elf
	$(OBJCOPY) $< -O binary $@

# Indicate to make that `clean` is not an actual file on the FS
.PHONY: clean
clean:
	rm -f {part1,part2,part3}.{o,elf,bin} lib.o $(COMMON_OBJS)

.PHONY: part1 part2 part3
part1: part1.bin
	$(INSTALL) $<
part2: part2.bin
	$(INSTALL) $<
part3: part3.uf2
	cp $< $(FLASH_MOUNT)
