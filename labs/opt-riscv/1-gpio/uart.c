#include "../lib/rp2350.h"

static void set_baud_rate_internal(hw_uart_t* uart, uint32_t baud_rate, uint32_t clk_peri_freq);

void uart_init(hw_uart_t* uart, uint32_t baud_rate, uint32_t clk_peri_freq)
{
    todo("deassert the UART reset");

    todo("set the baud rate");

    todo("Set up the UART in the LCR_H register");

    todo("Enable the UART in the CR register");
}

static bool can_write(hw_uart_t* uart)
{
    todo("implement me");
}

void uart_write_byte(hw_uart_t* uart, uint8_t c)
{
    todo("implement me");
}

void uart_flush(hw_uart_t* uart)
{
    // We want this to only return when the FIFO is drained and everything has been written out
    todo("implement me");
}

// Set the `ibrd` and `fbrd` registers on the given UART.
// IMPORTANT: for the baud rate to latch, there must be a write to the LCR_H
// register (at some point).
static void set_baud_rate_internal(hw_uart_t* uart, uint32_t baud_rate, uint32_t clk_peri_freq)
{
    // See 12.1.3.2.1 for the baud rate calculations
    todo("set uartibrd and uartfbrd");
}

// -- We give these to you

void uart_write(hw_uart_t* uart, const uint8_t* bytes, size_t len)
{
    for (; len; len--)
        uart_write_byte(uart, *bytes++);
}

void uart_putc(hw_uart_t* uart, char c)
{
    uart_write_byte(uart, c);
}

void uart_puts(hw_uart_t* uart, const char* str)
{
    while (*str != '\0') {
        uart_write_byte(uart, *str);
        ++str;
    }
}
