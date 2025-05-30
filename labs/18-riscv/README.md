# Introduction

In this lab, we'll be getting started with the Raspberry Pi Pico 2. It's a dual-mode processor, which basically means that it has both ARM and RISC-V CPUs that we can decide to use. For this lab, we'll be using the RISC-V one, which is called the Hazard3.

> Quick aside on terminology:
>  - the Pico 2 is the board
>  - it has an RP2350 SBC; this is similar to the BCM2835 on the normal pis
>  - the RP2350 has a pair of Hazard3 RISC-V CPUs (equivalent to the ARM1176JZF-S on the normal pis)

All that being said, the RP2350 is a pretty different thing from the BCM2835; it's much more on the microprocessor end of the spectrum than the BCM2835. The BCM2835 is designed as an application processor: you can use it in a desktop and it'll work fine; this is in fact the intended use (or was, at least). Therefore, it has the trappings of a such a processor: lots of RAM, it boots from an SD, has a cache system, page tables, etc.

The RP2350 is not built for this. It has 520KB (!!) of RAM, which is much less than the 512MB available on the zero, has no caches, no page tables (only a considerably simpler PMP), and is designed to boot from flash.
The Pico 2 has a certain amount of onboard flash memory (4MB), which it can execute code from.

This functions somewhat similarly to an SD card, but is similarly cumbersome to write to. Different chips expose their flash for _flashing_ in different ways; on the RP2350, it exposes its flash memory as a sort of pseudo-flash-driver. That is, when you boot your pi in a special way and plug it into your computer, it will pretend to be a flash drive, which you can then copy a firmware file to, which must be in a special UF2 format that describes how different parts of the file should be written to flash memory.
For the Pico 2, the exact process is:
 1. Unplug the Pico to power it off
 2. Hold down the BOOTSEL button on the Pico
 3. Plug the Pico back in to power
 4. Unpress the BOOTSEL.
 5. Copy a UF2 file

Additionally, there are some additional complications with booting; so far, we've talked about how files are written to flash, but actually getting the RP2350's hardwired boot ROM to actually start executing those is another matter, which we'll be getting into in Part 3.

However, before then, to keep things simple, we'll be using our normal pi-install workflow, though in order to do this, we'll need to flash a UART bootloader to the pico, which we cover lower down.

# Getting the toolchain

### macOS

Use homebrew (https://brew.sh);

```sh
brew tap riscv-software-src/riscv
brew install riscv-tools
```

### Linux

With `apt`, you can use:
```
sudo apt install gcc-riscv64-unknown-elf
```
Otherwise, see https://github.com/riscv-collab/riscv-gnu-toolchain.

# Flashing initial bootloader

To recap, for the Pico 2, the exact process to flash the bootloader is:
 1. Unplug the Pico to power it off
 2. Hold down the BOOTSEL button on the Pico
 3. Plug the Pico back in to power
 4. Unpress the BOOTSEL.
 5. Copy a UF2 file

The UF2 file in question is `18-riscv/bin/okboot.uf2`, which is a bootloader that implements the normal pi-install protocol (but runs on the pico 2).

### macOS

```sh
cp bin/okboot.uf2 /Volumes/RP2350`
```

### Linux

When I tested this on my server tower (Ubuntu), this is what I needed to get it flashed:

```sh
mkdir /mnt/pico
sudo mount /dev/sda1 /mnt/pico
cp bin/okboot.uf2 /mnt/pico
```

We'll only need to do this once: once the bootloader has been flashed, we can use it as we would a pi zero, though instead of pressing a reset button, we need to unplug/replug the Pico from its USB power.

# Wiring

We need two USB connections here; the USB port on the Pico doesn't speak UART like the one on the parthiv-board, so we need to find something to take the place of the parthiv-board. In this case, that's a USB-UART converter (the small red or blue boards). These will allow us to use the UART bootloader we just flashed onto the Pico.

Attach your UART FTDI converter to the PICO like so:
```
PICO TX (GP0) <-> FTDI RX
PICO RX (GP1) <-> FTDI TX
GND           <-> GND
```

Now, if you run `ls /dev | grep -i usb`, you should see at least one result.

# Key Documentation

There are a few useful datasheets that we've included in the `18-riscv/doc` directory. We have:
- `rp2350-datasheet.pdf`, which describes the RP2350 and its peripherals
- `riscv-spec-20191213.pdf`, which describes the version of the RISC-V specification that the Hazard3 processor on the RP2350 implements. We only really care about chapter 2 and chapter 25, the rest either isn't implemented or isn't necessary.
- `riscv-privileged-20211203.pdf` describes the privileged-mode ISA, which isn't super relevent here.

# Part 0: Check that everything works

Now that the infrastructure has been taken care of (hopefully), we'll run a quick check to make sure everything works.
`0-basic blink` contains a very simple blinker program; the source is in blink.S. We'll upload it to the Pico, and if we see the onboard LED blinking, then the previous steps probably worked alright.

You should be able to simply `make` from the `0-basic-blink` directory and see your pico start blinking its onboard LED.

# Part 1: GPIO

Our goal is to, like we did in 140e, implement the basic GPIO building blocks, but this time for the RP2350 instead of the BCM2835.
We'll be filling out `1-gpio/gpio.c`; look for the `todo` points.

To do this, we'll be using the SIO register block (chapter 3.1 in `rp2350-datasheet.pdf`), though setting up pins as input/output needs some bits from chapter 9 (specifically, 9.4 function selection, which uses the IO register bank (9.11.1), and the pad controls (9.11.3).

In here, we can run with `make part1` to run our code; this will use `part1.c` as the `notmain` file.

### Checkoff:

This is fairly simple: we just want to see that your code works and you can blink an LED.

# Part 2: UART

In part 1, we had access to the staff UART code; now, we'll rewrite it.

We can find relevant documentation in section 12.1 of `rp2350-datasheet.pdf`.

This should look fairly familiar.

For this, we just want to see that your UART works; we're not being too careful about implementation details.
We'll write the code in `uart.c`.

Here, we can use `make part2` to run our code; this will use `part2.c` as the `notmain` file.

### Checkoff:

We want to see that your UART code can print, nothing more.

# Part 3: BootROM

For a final step, we want to (be able to) remove our dependence on the `okboot` bootloader. To do this, we'll build a very minimal bootable flash image in `part3.c`.

Basically, we'll want to create an `IMAGE_DEF` block in what's called a "block loop" in the first 4K of the flash image.
We can do this by creating a static array in a C file, and putting it into a special `.text.boot_header` section that we force to be early in the image.

For information on block loops, check 5.5 and 5.9 in `rp2350-datasheet.pdf`. Basically, they're magic-number-delimited structures that are spread across flash; when the Pico boots, it'll search the first 4K for an block with the special `IMAGE_DEF` type, which will allow us to specify how we want the image we've written to flash to be treated.

IMPORTANT: Look at Erratum RP2350-E10 on page 1350 in `rp2350-datasheet.pdf`.

# Major differences with the BCM

- We don't need `dsb`'s; their presence in the BCM was a result of the way the BCM's bus worked, but the rp2350 doesn't have those issues.
