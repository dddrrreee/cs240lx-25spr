#pragma once
#include "support.h"
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define REG(off, ty, name)                                           \
    struct {                                                         \
        uint32_t CONCAT(__pad, __COUNTER__)[off / sizeof(uint32_t)]; \
        ty name;                                                     \
    }
#define PAD(from, to) \
    uint8_t CONCAT(__pad, __COUNTER__)[(to) - (from)]

enum {
    PSM_BASE = 0x4001'8000,
    RESETS_BASE = 0x4002'0000,
    IO_BANK0_BASE = 0x4002'8000,
    PADS_BANK0_BASE = 0x4003'8000,
    UART0_BASE = 0x4007'0000,
    UART1_BASE = 0x4007'8000,
    WATCHDOG_BASE = 0x400d'8000,
    SIO_BASE = 0xd000'0000,
};

typedef volatile uint32_t io32;

typedef struct {
    struct {
        io32 status;
        io32 ctrl;
    } pins[47];
} hw_io_bank_t;
static hw_io_bank_t* io_bank0 = (hw_io_bank_t*)IO_BANK0_BASE;

typedef struct {
    io32 bank0_voltage_select;
    io32 pins[47];
    io32 swclk;
    io32 swd;
} hw_pads_bank_t;
static hw_pads_bank_t* pads_bank0 = (hw_pads_bank_t*)PADS_BANK0_BASE;

typedef struct {
    io32 cpuid; // 00
    io32 gpio_in; // 04
    io32 gpio_hi_in; // 08
    io32 __pad0[1]; // 0c

    io32 gpio_out; // 10
    io32 gpio_hi_out; // 14
    io32 gpio_out_set; // 18
    io32 gpio_hi_out_set; // 1c
    io32 gpio_out_clr; // 20
    io32 gpio_hi_out_clr; // 24
    io32 gpio_out_xor; // 28
    io32 gpio_hi_out_xor; // 2c

    io32 gpio_oe; // 30
    io32 gpio_hi_oe; // 34
    io32 gpio_oe_set; // 38
    io32 gpio_hi_oe_set; // 3c
    io32 gpio_oe_clr; // 40
    io32 gpio_hi_oe_clr; // 44
    io32 gpio_oe_xor; // 48
    io32 gpio_hi_oe_xor; // 4c

    io32 __pad1[(0x1a0 - 0x50) / sizeof(io32)]; // 50
    io32 riscv_softirq; // 1a0
    io32 mtime_ctrl; // 1a4
    io32 mtime; // 1a8
    io32 mtimeh; // 1ac
} hw_sio_t;
static hw_sio_t* sio = (hw_sio_t*)SIO_BASE;
// -- Ensure padding correctness
_Static_assert(__builtin_offsetof(hw_sio_t, gpio_out) == 0x10);
_Static_assert(__builtin_offsetof(hw_sio_t, riscv_softirq) == 0x1a0);

// From page 973 of rp2350-datasheet.pdf
enum {
    UART_LCR_H_BRK_BITS = 1 << 0,
    UART_LCR_H_PEN_BITS = 1 << 1,
    UART_LCR_H_EPS_BITS = 1 << 2,
    UART_LCR_H_STP2_BITS = 1 << 3,
    UART_LCR_H_FEN_BITS = 1 << 4,
    UART_LCR_H_WLEN_BITS = 3 << 5,
    UART_LCR_H_WLEN_8 = 0b11 << 5,
    UART_LCR_H_WLEN_7 = 0b10 << 5,
    UART_LCR_H_WLEN_6 = 0b01 << 5,
    UART_LCR_H_WLEN_5 = 0b00 << 5,
    UART_LCR_H_SPS_BITS = 1 << 7,
};

// From page 974-975 of rp2350-datasheet.pdf
enum {
    UART_CR_UARTEN_BITS = 1 << 0,
    UART_CR_SIREN_BITS = 1 << 1,
    UART_CR_SIRLP_BITS = 1 << 2,
    UART_CR_LBE_BITS = 1 << 7,
    UART_CR_TXE_BITS = 1 << 8,
    UART_CR_RXE_BITS = 1 << 9,
    UART_CR_DTR_BITS = 1 << 10,
    UART_CR_RTS_BITS = 1 << 11,
    UART_CR_OUT1_BITS = 1 << 12,
    UART_CR_OUT2_BITS = 1 << 13,
    UART_CR_RTSEN_BITS = 1 << 14,
    UART_CR_CTSEN_BITS = 1 << 15,
};

// From page 971-972 of rp2350-datasheet.pdf
enum {
    UART_FR_CTS = 1 << 0,
    UART_FR_DSR = 1 << 1,
    UART_FR_DCD = 1 << 2,
    UART_FR_BUSY = 1 << 3,
    UART_FR_RXFE = 1 << 4,
    UART_FR_TXFF = 1 << 5,
    UART_FR_RXFF = 1 << 6,
    UART_FR_TXFE = 1 << 7,
    UART_FR_RI = 1 << 8,
};
typedef union {
    REG(0x00, io32, dr);
    REG(0x04, io32, rsr);
    REG(0x18, io32, fr);
    REG(0x20, io32, ilptr);
    REG(0x24, io32, ibrd);
    REG(0x28, io32, fbrd);
    REG(0x2c, io32, lcr_h);
    REG(0x30, io32, cr);
    REG(0x48, io32, dmacr);
} hw_uart_t;
static hw_uart_t* uart0 = (hw_uart_t*)UART0_BASE;
static hw_uart_t* uart1 = (hw_uart_t*)UART1_BASE;

// From Page 503 of rp2350-datasheet.pdf.
enum {
    RESET_DMA = 1 << 2,
    RESET_PIO0 = 1 << 11,
    RESET_PIO1 = 1 << 12,
    RESET_PIO2 = 1 << 13,
    RESET_PWM = 1 << 16,
    RESET_UART0 = 1 << 26,
    RESET_UART1 = 1 << 27,
};
typedef union {
    REG(0x00, io32, reset);
    REG(0x04, io32, wdsel);
    REG(0x08, io32, reset_done);
} hw_resets_t;
static hw_resets_t* resets = (hw_resets_t*)RESETS_BASE;

enum {
    WDOG_CTRL_TRIGGER_BITS = 1 << 31,
    WDOG_CTRL_ENABLE_BITS = 1 << 30,
    WDOG_CTRL_TIME_BITS = 0x00ff'ffff,
};
typedef union {
    REG(0x00, io32, ctrl);
    REG(0x04, io32, load);
    REG(0x08, io32, reason);
    REG(0x08, io32, scratch[8]);
} hw_wdog_t;
static hw_wdog_t* wdog = (hw_wdog_t*)WATCHDOG_BASE;

enum {
    PSM_WDSEL_BITS = 0x00ff'ffff,
    PSM_WDSEL_ROSC_BITS = 1 << 2,
    PSM_WDSEL_XOSC_BITS = 1 << 3,
};
typedef union {
    REG(0x00, io32, frce_on);
    REG(0x04, io32, frce_off);
    REG(0x08, io32, wdsel);
    REG(0x0c, io32, done);
} hw_psm_t;
static hw_psm_t* psm = (hw_psm_t*)PSM_BASE;

typedef uint8_t pin_t;
typedef uint8_t io_funcsel_t;

void gpio_set_function(pin_t pin, io_funcsel_t funcsel);
void gpio_set_output(pin_t pin);
void gpio_set_on(pin_t pin);
void gpio_set_off(pin_t pin);
void gpio_write(pin_t pin, bool level);
void gpio_toggle(pin_t pin);

void uart_init(hw_uart_t* uart, uint32_t baud_rate, uint32_t clk_peri_freq);
void uart_write_byte(hw_uart_t* uart, uint8_t c);
void uart_write_dma(hw_uart_t* uart, const uint8_t* bytes, size_t len);
void uart_flush(hw_uart_t *uart);
void uart_write(hw_uart_t* uart, const uint8_t* bytes, size_t len);
void uart_putc(hw_uart_t* uart, char c);
void uart_puts(hw_uart_t* uart, const char* str);

[[noreturn]]
static inline void
pico2_reboot()
{
    uart_puts(uart0, "\nDone. Rebooting.\n");
    uart_flush(uart0);
    
    wdog->ctrl &= !WDOG_CTRL_ENABLE_BITS;
    psm->wdsel = PSM_WDSEL_BITS & ~(PSM_WDSEL_ROSC_BITS | PSM_WDSEL_XOSC_BITS);
    wdog->ctrl |= WDOG_CTRL_TRIGGER_BITS;

    while (1)
        __asm__ volatile("");
}

#define panic(msg, args...)                            \
    do {                                               \
        (printk)("PANIC:%s:%s:%d:" msg "\n",           \
            __FILE__, __FUNCTION__, __LINE__, ##args); \
        pico2_reboot();                                \
    } while (0)
#define todo(msg...) panic("TODO:" msg)
#define unimplemented() panic("implement this function!\n");

int printk(const char *format, ...);
