# Introduction

In this lab, we'll be getting started with the Raspberry Pi Pico 2. It's a dual-mode processor, which basically means that it has both ARM and RISC-V CPUs that we can decide to use. For this lab, we'll be using the RISC-V one, which is called the Hazard3.

> Quick aside on terminology:
>  - the Pico 2 is the board
>  - it has an RP2350 SBC; this is similar to the BCM2835 on the normal pis
>  - the RP2350 has a pair of Hazard3 RISC-V CPUs (equivalent to the ARM1176JZF-S on the normal pis)

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

### macOS

```sh
cp bin/okboot.uf2 /Volumes/RP2350`
```

### Linux

```sh
mkdir /mnt/pico
sudo mount /dev/sda1 /mnt/pico
cp bin/okboot.uf2 /mnt/pico
```

# Wiring

Attach your UART FTDI converter to the PICO like so:
```
PICO TX (GP0) <-> FTDI RX
PICO RX (GP1) <-> FTDI TX
GND           <-> GND
```

Now, if you run `ls /dev | grep -i usb`, you should see at least one result.

# Part 0: Check that everything works

`0-basic blink` contains a very simple blinker program; the source is in blink.S.

You should be able to simply `make` and see your pico start blinking its onboard LED.

# Key Documentation

In `doc/`, we have:
- `rp2350-datasheet.pdf`, which describes the RP2350 and its peripherals
- `riscv-spec-20191213.pdf`, which describes the version of the RISC-V specification that the Hazard3 processor on the RP2350 implements. We only really care about chapter 2 and chapter 25, the rest either isn't implemented or isn't necessary.
- `riscv-privileged-20211203.pdf` describes the privileged-mode ISA, which isn't super relevent here.

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

# Major differences with the BCM

- We don't need `dsb`'s; their presence in the BCM was a result of the way the BCM's bus worked, but the rp2350 doesn't have those issues.
