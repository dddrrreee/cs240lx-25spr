#include "stepper-int.h"
#include "timer-interrupt.h"
#include "cycle-count.h"
#include "rpi-inline-asm.h"

// you can/should play around with this
#define STEPPER_INT_TIMER_INT_PERIOD 100 

static int first_init = 1;

#define MAX_STEPPERS 16
static stepper_int_t * my_steppers[MAX_STEPPERS];
static unsigned num_steppers = 0;

void stepper_int_handler(unsigned pc) {
    // check and clear timer interrupt
    dev_barrier();
    unsigned pending = GET32(IRQ_basic_pending);
    if((pending & RPI_BASIC_ARM_TIMER_IRQ) == 0)
        return;
    PUT32(ARM_Timer_IRQ_Clear, 1);
    dev_barrier();  


    unimplemented();
}

void interrupt_vector(unsigned pc){
    stepper_int_handler(pc);
}

stepper_int_t * stepper_init_with_int(unsigned dir, unsigned step){
    if(num_steppers == MAX_STEPPERS){
        return NULL;
    }
    kmalloc_init(1);

    unimplemented();

    stepper_int_t *stepper = kmalloc(sizeof *stepper);

    //initialize interrupts; only do once, on the first init
    if(first_init){
        first_init = 0;
        interrupt_init();
        cycle_cnt_init();
        // prescale = 1.
        timer_init(0, STEPPER_INT_TIMER_INT_PERIOD);
        cpsr_int_enable();
    }

    return stepper;
}

stepper_int_t * stepper_int_init_with_microsteps(unsigned dir, unsigned step, unsigned MS1, unsigned MS2, unsigned MS3, stepper_microstep_mode_t microstep_mode){
    unimplemented();
}

/* retuns the enqueued position. perhaps return the queue of positions instead? */
stepper_position_t * stepper_int_enqueue_pos(stepper_int_t * stepper, int goal_steps, unsigned usec_between_steps){
    stepper_position_t *new_pos = 0;

    unimplemented();

    return new_pos;
}

int stepper_int_get_position_in_steps(stepper_int_t * stepper){
    return stepper_get_position_in_steps(stepper->stepper);
}

int stepper_int_is_free(stepper_int_t * stepper){
    return stepper->status == NOT_IN_JOB;
}

int stepper_int_position_is_complete(stepper_position_t * pos){
    return pos->status == FINISHED;
}

