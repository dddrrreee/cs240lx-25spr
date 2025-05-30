.attribute arch, "rv32imac"

#define MSTATUS_MEIE (1 << 3)

#define IO_FUNCSEL_OFFSET 0
#define IO_FUNCSEL_SIO 5
#define IO_FUNCSEL(funcsel) (funcsel) << IO_FUNCSEL_OFFSET

#define PAD_DRIVE_12mA 3
#define PAD_DRIVE_OFFSET 4
#define PAD_DRIVE_CURRENT(current) (current) << PAD_DRIVE_OFFSET

#define IO_CTRL_REG(gpio_pin) (gpio_pin * 8) + 4
#define PADS_CTRL_REG(gpio_pin) (gpio_pin * 4) + 4

#define SIO_OE_ENABLE(pin) (1 << (pin))
#define SIO_GPIO_OE 0x030
#define SIO_GPIO_OUT_XOR 0x028

.equ IO_BANK0_BASE, 0x40028000
.equ PADS_BANK0_BASE, 0x40038000
.equ SIO_BASE, 0xd0000000

#define LED_PIN 25

.section ".text.boot"
.globl _start
_start:
        lui s0, %hi(IO_BANK0_BASE)
        lui s1, %hi(PADS_BANK0_BASE)
        lui s2, %hi(SIO_BASE)

        li t0, IO_FUNCSEL(IO_FUNCSEL_SIO)
        sw t0, IO_CTRL_REG(LED_PIN) (s0)
        li t0, PAD_DRIVE_CURRENT(PAD_DRIVE_12mA)
        sw t0, PADS_CTRL_REG(LED_PIN) (s1)
        li t0, SIO_OE_ENABLE(LED_PIN)
        sw t0, SIO_GPIO_OE (s2)

loop:
        li t1, 1000000
delay:
        addi t1, t1, -1
        bnez t1, delay
        sw t0, SIO_GPIO_OUT_XOR (s2)
        j loop
