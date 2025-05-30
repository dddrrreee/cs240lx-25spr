.SUFFIXES:
.DEFAULT_GOAL := run

CC := riscv64-unknown-elf-gcc
LD := riscv64-unknown-elf-ld
OBJCOPY := riscv64-unknown-elf-objcopy
INSTALL := pi-install

MEMMAP := ../pico2.ld

blink.o: blink.S
	$(CC) -march=rv32imac_zicsr -mabi=ilp32 $^ -o $@ -c
blink.elf: blink.o | $(MEMMAP)
	$(LD) -melf32lriscv $^ -o $@ -T$(MEMMAP) --gc-sections -z noexecstack -flto=thin -static
blink.bin: blink.elf
	$(OBJCOPY) $< -O binary $@

# Indicate to make that `clean` is not an actual file on the FS
.PHONY: clean
clean:
	rm -f blink.{o,elf,bin}

.PHONY: run
run: blink.bin
	$(INSTALL) $<
