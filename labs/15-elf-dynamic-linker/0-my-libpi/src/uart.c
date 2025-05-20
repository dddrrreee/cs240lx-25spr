// simple mini-uart driver: implement every routine 
// with a <todo>.
//
// NOTE: 
//  - from broadcom: if you are writing to different 
//    devices you MUST use a dev_barrier().   
//  - its not always clear when X and Y are different
//    devices.
//  - pay attenton for errata!   there are some serious
//    ones here.  if you have a week free you'd learn 
//    alot figuring out what these are (esp hard given
//    the lack of printing) but you'd learn alot, and
//    definitely have new-found respect to the pioneers
//    that worked out the bcm eratta.
//
// historically a problem with writing UART code for
// this class (and for human history) is that when 
// things go wrong you can't print since doing so uses
// uart.  thus, debugging is very old school circa
// 1950s, which modern brains arne't built for out of
// the box.   you have two options:
//  1. think hard.  we recommend this.
//  2. use the included bit-banging sw uart routine
//     to print.   this makes things much easier.
//     but if you do make sure you delete it at the 
//     end, otherwise your GPIO will be in a bad state.
//
// in either case, in the next part of the lab you'll
// implement bit-banged UART yourself.
#include "rpi.h"
#include <stdint.h>

// change "1" to "0" if you want to comment out
// the entire block.
#if 0
//*****************************************************
// We provide a bit-banged version of UART for debugging
// your UART code.  delete when done!
//
// NOTE: if you call <emergency_printk>, it takes 
// over the UART GPIO pins (14,15). Thus, your UART 
// GPIO initialization will get destroyed.  Do not 
// forget!   

// header in <libpi/include/sw-uart.h>
#include "sw-uart.h"
static sw_uart_t sw_uart;

// if we've ever called emergency_printk better
// die before returning.
static int called_sw_uart_p = 0;

// a sw-uart putc implementation.
static int sw_uart_putc(int chr) {
    sw_uart_put8(&sw_uart,chr);
    return chr;
}

// call this routine to print stuff. 
//
// note the function pointer hack: after you call it 
// once can call the regular printk etc.
static void emergency_printk(const char *fmt, ...) {
    // track if we ever called it.
    called_sw_uart_p = 1;


    // we forcibly initialize each time it got called
    // in case the GPIO got reset.
    // setup gpio 14,15 for sw-uart.
    sw_uart = sw_uart_default();

    // all libpi output is via a <putc>
    // function pointer: this installs ours
    // instead of the default
    rpi_putchar_set(sw_uart_putc);

    printk("NOTE: HW UART GPIO is in a bad state now\n");

    // do print
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);

}

#undef todo
#define todo(msg) do {                      \
    emergency_printk("%s:%d:%s\nDONE!!!\n",      \
            __FUNCTION__,__LINE__,msg);   \
    rpi_reboot();                           \
} while(0)

// END of the bit bang code.
#endif


//*****************************************************
// the rest you should implement.

// Device registers
static const uint32_t AUX_REG_BASE = 0x20215000;
struct AUX_REG_STRUCT {
    uint32_t AUX_IRQ;
    uint32_t AUX_ENABLES;
    uint32_t __gap[14]; // gap in the address space
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

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
    // NOTE: make sure you delete all print calls when
    // done!
    // emergency_printk("start here\n");

    // perhaps confusingly: at this point normal printk works
    // since we overrode the system putc routine.
    // printk("write UART addresses in order\n");
    dev_barrier();

    // GPIO 14, 15 (remember we are using UART "1") -> set to alt5 (0b010)
    gpio_set_function(14, GPIO_FUNC_ALT5);
    gpio_set_function(15, GPIO_FUNC_ALT5);

    dev_barrier();
    
    // Enable miniUART without disrupting SPIs (must; bc device doesn't even allow write if not)
    put32(&aux_regs->AUX_ENABLES, (get32(&aux_regs->AUX_ENABLES) & 0b110) | 0b1); // the first bit = mini UART enable. Must write 0 to bits 3-31 & preserve bits 1-2

    dev_barrier();

    // Immediately disable tx/rx (you don't want to send garbage).
    put32(&aux_regs->AUX_MU_CNTL_REG, 0); // disable transmitter/receiver

    // Find and clear all parts of its state (e.g., FIFO queues) since we are not absolutely positive they do not hold garbage. Disable interrupts.
    // Reset everything that we don't touch elsewhere in this function & is writable
    put32(&aux_regs->AUX_MU_IER_REG, 0); // disable interrupts for rx && tx
    put32(&aux_regs->AUX_MU_IIR_REG, 0b110); // clear rx/tx FIFOs; other bits are read-only so doesn't matter

    // 8 bits, 1 start bit, 1 stop bit. No flow control.
    put32(&aux_regs->AUX_MU_LCR_REG, 0b11); // set 8-bit mode (other bits should be 0)
    put32(&aux_regs->AUX_MU_MCR_REG, 0); // no flow control (undocumented)

    // Configure: 115200 Baud,
    put32(&aux_regs->AUX_MU_BAUD_REG, 270); // closest we can get with systemclock/8(reg+1) = 115200

    // Enable tx/rx. It should be working!
    put32(&aux_regs->AUX_MU_CNTL_REG, 0b11); // enable transmitter/receiver (other bits can be 0)

    dev_barrier();
    // delete everything to do w/ sw-uart when done since
    // it trashes your hardware state and the system
    // <putc>.
    // demand(!called_sw_uart_p, 
    //     delete all sw-uart uses or hw UART in bad state);
}

// disable the uart: make sure all bytes have been
// 
void uart_disable(void) {
    dev_barrier();
    // Flush out transmits in progress
    uart_flush_tx();
    // Disable miniUART (preserve bits 1, 2)
    put32(&aux_regs->AUX_ENABLES, get32(&aux_regs->AUX_ENABLES) & 0b110);
    dev_barrier();
}

// returns 1 if the hardware TX (output) FIFO has room
// for at least one byte.  returns 0 otherwise.
int uart_can_put8(void) {
    dev_barrier();
    int ret_val = (get32(&aux_regs->AUX_MU_LSR_REG) >> 5) & 0b1; // tx space available bit
    dev_barrier();
    return ret_val;
}

// returns:
//  - 1 if at least one byte on the hardware RX FIFO.
//  - 0 otherwise
int uart_has_data(void) {
    dev_barrier();
    int ret_val = get32(&aux_regs->AUX_MU_LSR_REG) & 0b1; // rx byte available bit
    dev_barrier();
    return ret_val;
}

// put one byte on the TX FIFO, if necessary, waits
// until the FIFO has space.
int uart_put8(uint8_t c) {
    // Block until UART TX Q has at least 1 byte free
    while (!uart_can_put8());

    put32(&aux_regs->AUX_MU_IO_REG, c); // must write 0 to bits 8-31
    dev_barrier();

    // Return fixed value for now
    return 1;
}

// returns one byte from the RX (input) hardware
// FIFO.  if FIFO is empty, blocks until there is 
// at least one byte.
int uart_get8(void) {
    // Block until UART RX Q has at least 1 byte
    while (!uart_has_data());

    int ret_val = get32(&aux_regs->AUX_MU_IO_REG) & 0xff;
    dev_barrier();

    return ret_val; 
}

// returns:
//  -1 if no data on the RX FIFO.
//  otherwise reads a byte and returns it.
int uart_get8_async(void) { 
    if(!uart_has_data())
        return -1;
    return uart_get8();
}

// returns:
//  - 1 if TX FIFO empty AND idle.
//  - 0 if not empty.
int uart_tx_is_empty(void) {
    dev_barrier();
    int val = (get32(&aux_regs->AUX_MU_LSR_REG) >> 6) & 0x1; // TX Q empty && idle bit
    dev_barrier();
    return val;
}

// return only when the TX FIFO is empty AND the
// TX transmitter is idle.  
//
// used when rebooting or turning off the UART to
// make sure that any output has been completely 
// transmitted.  otherwise can get truncated 
// if reboot happens before all bytes have been
// received.
void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        rpi_wait();
}
