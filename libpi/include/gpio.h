/*************************************************************************
 * You need to implement these.
 */
#ifndef __GPIO_H__
#define __GPIO_H__

// GPIO pin mappings for UART
#define GPIO_TX 14
#define GPIO_RX 15

// different functions we can set GPIO pins to.
typedef enum {
    GPIO_FUNC_INPUT   = 0,
    GPIO_FUNC_OUTPUT  = 1,
    GPIO_FUNC_ALT0    = 4,
    GPIO_FUNC_ALT1    = 5,
    GPIO_FUNC_ALT2    = 6,
    GPIO_FUNC_ALT3    = 7,
    GPIO_FUNC_ALT4    = 3,
    GPIO_FUNC_ALT5    = 2,
} gpio_func_t;

// set GPIO function for <pin> (input, output, alt...).  settings for other
// pins should be unchanged.
void gpio_set_function(unsigned pin, gpio_func_t function);

// set <pin> to input or output.
void gpio_set_input(unsigned pin);
void gpio_set_output(unsigned pin);

// write <pin> = <val>.  1 = high, 0 = low.
// <pin> must be in output mode, other pins will be unchanged.
void gpio_write(unsigned pin, unsigned val);

// read <pin>: 1 = high, 0 = low.
int gpio_read(unsigned pin);

// turn <pin> on.
void gpio_set_on(unsigned pin);

// turn <pin> off.
void gpio_set_off(unsigned pin);

// broadcom doc claims we can get the current state of the device
// but that seems incorrect.

// set <pin> as a pullup
void gpio_set_pullup(unsigned pin);
// set <pin> as a pulldown.
void gpio_set_pulldown(unsigned pin);
// set <pin> back to the default state: no pull up, no pulldown.
void gpio_pud_off(unsigned pin);

int gpio_get_pud(unsigned pin);

/*****************************************************************
 * use the following to configure interrupts on pins.
 */


// gpio_int_rising_edge and gpio_int_falling_edge (and any other) should
// call this routine (you must implement) to setup the right GPIO event.
// as with setting up functions, you should bitwise-or in the value for the 
// pin you are setting with the existing pin values.  (otherwise you will
// lose their configuration).  you also need to enable the right IRQ.   make
// sure to use device barriers!!
enum { GPIO_INT0 = 49, GPIO_INT1, GPIO_INT2, GPIO_INT3 };

// returns 1 if there is currently a GPIO_INT0 interrupt, 
// 0 otherwise.
//
// note: we can only get interrupts for <GPIO_INT0> since the
// (the other pins are inaccessible for external devices).
int gpio_has_interrupt(void);

// p97 set to detect rising edge (0->1) on <pin>.
// as the broadcom doc states, it  detects by sampling based on the clock.
// it looks for "011" (low, hi, hi) to suppress noise.  i.e., its triggered only
// *after* a 1 reading has been sampled twice, so there will be delay.
// if you want lower latency, you should us async rising edge (p99)
void gpio_int_rising_edge(unsigned pin);

// p98: detect falling edge (1->0).  sampled using the system clock.  
// similarly to rising edge detection, it suppresses noise by looking for
// "100" --- i.e., is triggered after two readings of "0" and so the 
// interrupt is delayed two clock cycles.   if you want  lower latency,
// you should use async falling edge. (p99)
void gpio_int_falling_edge(unsigned pin);

// p98: detect when input pin=1.  must clear the source of the 
// interrupt before clearing the event or it will just retrigger.
void gpio_enable_hi_int(unsigned pin);

// p99: detect when input pin=0.  if pin is still =0 when you clear,
// will just retrigger.
void gpio_int_low(unsigned pin);


// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an interrupt.
// if you configure multiple events to lead to interrupts, you will have to 
// read the pin to determine which caused it.
int gpio_event_detected(unsigned pin);

// p96: have to write a 1 to the pin to clear the event.
void gpio_event_clear(unsigned pin);

void gpio_int_async_falling_edge(unsigned pin);
void gpio_int_async_rising_edge(unsigned pin) ;

void gpio_fiq_async_falling_edge(unsigned pin);
void gpio_fiq_async_rising_edge(unsigned pin);
void gpio_fiq_falling_edge(unsigned pin);
void gpio_fiq_rising_edge(unsigned pin);

#endif
