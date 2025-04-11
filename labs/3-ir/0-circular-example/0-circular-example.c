/*
 * engler: trivial example to illustrate passing characters from interrupt to 
 * client code.
 *
 * this should look very familiar, since it's mostly a less chatty version of 
 * timer.c from the previous lab.
 */
#include "rpi.h"
#include "timer-interrupt.h"
#include "libc/circular.h"

static char hello[] = "hello world";
static cq_t putQ;
static volatile int ninterrupt, n_extra_ints;

// client has to define this.
void interrupt_vector(unsigned pc) {
    dev_barrier();
    unsigned pending = GET32(IRQ_basic_pending);

    // if this isn't true, could be a GPU interrupt (as discussed in Broadcom):
    // just return.  [confusing, since we didn't enable!]
    if((pending & RPI_BASIC_ARM_TIMER_IRQ) == 0) {
        n_extra_ints++;
        return;
    }

    // clear timer.
    PUT32(ARM_Timer_IRQ_Clear, 1);

    static int pos = 0;
    uint8_t c = hello[pos];
    if(c) { 
        if(!cq_push(&putQ, c))
            panic("can't push?\n");
        pos++;
    }
    ninterrupt++;
    dev_barrier();    
}

void notmain() {
    // setup general interrupt subsystem.
    interrupt_init();

    // if you mess with this value and prescaler, will change 
    // the number of interrupts.
    timer_init(16, 0x100);

    // Q: if you do this after enabling?
    cq_init(&putQ, 1);

    cpsr_int_enable();

    int n = strlen(hello), i;

    printk("enabled: about to send string from int handler:\n");
    char recv[128];
    assert(sizeof recv > strlen(hello));
    for(i = 0; i < n; i++) {
        uint8_t c = cq_pop(&putQ);
        recv[i] = c;
        if(!c)
            break;
    }
    printk("\tgot string <%s> from interrupt handler\n", recv);
    printk("\ttotal timer interrupts = %d, extra ints=%d\n", 
            ninterrupt, n_extra_ints);
    assert(n == i);
    assert(strcmp(hello, recv) == 0);
    printk("SUCCESS!\n");
}
