/*
    Stuart: Very minimal version of uart functions without dependencies

    The goal is to have as little as possible, and dynamically link the rest.
*/

#include "rpi.h"

static const uint32_t AUX_REG_BASE = 0x20215000;
struct AUX_REG_STRUCT {
    uint32_t AUX_IRQ;
    uint32_t AUX_ENABLES;
    uint32_t __gap[14];
    uint32_t AUX_MU_IO_REG;
    uint32_t AUX_MU_IER_REG;
    uint32_t AUX_MU_IIR_REG;
    uint32_t AUX_MU_LCR_REG;
    uint32_t AUX_MU_MCR_REG;
    uint32_t AUX_MU_LSR_REG;
    uint32_t AUX_MU_MSR_REG;
    uint32_t AUX_MU_SCRATCH;
    uint32_t AUX_MU_CNTL_REG;
    uint32_t AUX_MU_STAT_REG;
    uint32_t AUX_MU_BAUD_REG;
};
static struct AUX_REG_STRUCT *aux_regs = (struct AUX_REG_STRUCT *)AUX_REG_BASE;

void uart_init(void) {
    dev_barrier();
    gpio_set_function(14, GPIO_FUNC_ALT5);
    gpio_set_function(15, GPIO_FUNC_ALT5);
    dev_barrier();
    put32(&aux_regs->AUX_ENABLES, (get32(&aux_regs->AUX_ENABLES) & 0b110) | 0b1); 
    dev_barrier();

    put32(&aux_regs->AUX_MU_CNTL_REG, 0); 
    put32(&aux_regs->AUX_MU_IER_REG, 0); 
    put32(&aux_regs->AUX_MU_IIR_REG, 0b110); 
    put32(&aux_regs->AUX_MU_LCR_REG, 0b11); 
    put32(&aux_regs->AUX_MU_MCR_REG, 0); 
    put32(&aux_regs->AUX_MU_BAUD_REG, 270); 
    put32(&aux_regs->AUX_MU_CNTL_REG, 0b11); 

    dev_barrier();
}

void uart_disable(void) {
    dev_barrier();
    // Flush out transmits in progress
    uart_flush_tx();
    // Disable miniUART (preserve bits 1, 2)
    put32(&aux_regs->AUX_ENABLES, get32(&aux_regs->AUX_ENABLES) & 0b110);
    dev_barrier();
}

int uart_can_put8(void) {
    dev_barrier();
    int ret_val = (get32(&aux_regs->AUX_MU_LSR_REG) >> 5) & 0b1; // tx space available bit
    dev_barrier();
    return ret_val;
}

int uart_has_data(void) {
    dev_barrier();
    int ret_val = get32(&aux_regs->AUX_MU_LSR_REG) & 0b1; // rx byte available bit
    dev_barrier();
    return ret_val;
}

int uart_put8(uint8_t c) {
    while (!uart_can_put8());

    put32(&aux_regs->AUX_MU_IO_REG, c);
    dev_barrier();

    return 1;
}

int uart_get8(void) {
    // Block until UART RX Q has at least 1 byte
    while (!uart_has_data());

    int ret_val = get32(&aux_regs->AUX_MU_IO_REG) & 0xff;
    dev_barrier();

    return ret_val; 
}

int uart_get8_async(void) { 
    if(!uart_has_data())
        return -1;
    return uart_get8();
}

int uart_tx_is_empty(void) {
    dev_barrier();
    int val = (get32(&aux_regs->AUX_MU_LSR_REG) >> 6) & 0x1; // TX Q empty && idle bit
    dev_barrier();
    return val;
}

void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        ;
}
