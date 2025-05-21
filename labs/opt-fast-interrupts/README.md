## Making a fast interrupt-based digital analyzer.

***NOTE: I'm still editing this so the formatting/grammar is a mess***

For this lab, we give you a slow, trivial interrupt-based logic analyzer
--- a tool that records when the output from a pi  or sensor transitions
from 1-to-0 or 0-to-1 --- and you make it as fast as possible.  Being able
to make interrupts fast is generally useful.  And the hacks you do for
this, apply in general, making it an interesting puzzle.


----------------------------------------------------------------------
### Starter code: `code/scope.c` and `code/interrupt-asm.S`

We give you a slow and dumb, but simple interrupt based scope to start
with.  It sets up interrupts on rising and falling edges of a single
GPIO pin, and then uses a simple loop to trigger them by alternatively
writing 1 and 0.  The code tracks:

  1. How long it takes from the GPIO write until the interrupt handler
     reads the cycle counter.  The longer this time is, the worse
     accuracy will generally get: the longer it takes, the more things
     were done, and the more variance each can introduce.

     Your goal it to make this time as small and as low variance as
     possible.

  2. How long the entire interrupt handler takes.  The longer it
     takes to completely handle an interrupt, the fewer we can handle
     per second.  More is more, and more is better.

     Your goal it to (also) make this time as small as possible.  

  3. Our tests are only as good as our test generation.  We also
     want to improve how this works --- if we don't we could have a
     perfect scope, but its output would vary widely b/c the input would
     vary and we wouldn't know if the scope sucked or the test gen sucked.


Our first run is awful:
```
int cost = 4120 [604 from gpio write until cycle_cnt_read()]
int cost = 3554 [597 from gpio write until cycle_cnt_read()]
int cost = 3577 [525 from gpio write until cycle_cnt_read()]
int cost = 3526 [603 from gpio write until cycle_cnt_read()]
int cost = 3575 [526 from gpio write until cycle_cnt_read()]
int cost = 3557 [606 from gpio write until cycle_cnt_read()]
int cost = 3586 [529 from gpio write until cycle_cnt_read()]
int cost = 3568 [595 from gpio write until cycle_cnt_read()]
int cost = 3577 [525 from gpio write until cycle_cnt_read()]
int cost = 3559 [597 from gpio write until cycle_cnt_read()]
int cost = 3532 [525 from gpio write until cycle_cnt_read()]
int cost = 3557 [595 from gpio write until cycle_cnt_read()]
int cost = 3563 [529 from gpio write until cycle_cnt_read()]
int cost = 3517 [597 from gpio write until cycle_cnt_read()]
int cost = 3583 [529 from gpio write until cycle_cnt_read()]
int cost = 3559 [597 from gpio write until cycle_cnt_read()]
int cost = 3535 [525 from gpio write until cycle_cnt_read()]
int cost = 3543 [592 from gpio write until cycle_cnt_read()]
int cost = 3589 [529 from gpio write until cycle_cnt_read()]
int cost = 3565 [603 from gpio write until cycle_cnt_read()]
```

Bad things:
  1. Interrupt cost is huge: about 3500 cycles.  By default 
     the pi runs at 700MHz, so we can do *at best* 200,000 
     interrupts per second (200k sample per second = 200kHz).

     For comparison: as you saw in the PMU lab, a single cached
     add instruction takes 1 cycle.  3500 is equivalent to a lot of
     instructions.

  2. The time from when we write the GPIO pin until we can
     record when it happens is over 500 cycles and, even worse, it
     bounces around significantly.  It would be laughable to use
     this for something like the ws2812b.

### How to proceed.

After initial wins, performance hacking commonly:
  1. Has small steps that are hard to name for a git commit.
  2. Many dead ends.

What I would do:
  1. Have a text file where you write down what you do and then
     copy and paste the measured performance.

  2. Each time you make a real improvment make a subdirectory with
     a clearly sequential naming scheme (e.g., "version0", "version1",
     etc ) and copy the entire directory into it:

            % mkdir version4
            % cp * version4

I know this sounds incredibly boomer.  But it's simple-dumb in a way
that just works.  It's a lifesaver to have a series of clearly described
checkpoints that you can go back and forth between --- for example, when
you discover you broke something and need to narrow down when it was.
The fact you can just change directories and run the checkpoint with a
simple "make" is much much much better than trying to figure out some
git commit log.


Everytime we do something:
  1. Measure it.  People who don't know how to code are notorious
     for doing something that "should speed things up" and yet never
     measure that it does, thereby making the code more complex /
     fragile for nothing.

  2. Constantly look at the machine code.  This will tell you where
     cycles are being wasted.  In today's lab: if you don't see multiple
     times where the compiler has done something dumb, you're not
     looking often enough.

Do a single change and measure.  Don't do 10 or even 2.

----------------------------------------------------------------------
### Step 1: do the easy stuff.

It's pretty common that when you decide to optimize  there's a bunch 
of dumb extra stuff in the code that you can cut out immediately.
This code is no different.

  1. Cut out the global variables in the interrupt handler.  (This
     dropped a couple hundred cycles or so to 3300)
  2. Got rid of `gpio_event_detected` since we only expect to handle
     a single interrupt source --- this required some expensive reads
     so dropped us down to about 2700 cycles.
  3. Got rid of the `dev_barrier` calls since we know the test
     generation only triggers GPIO interrupts while manipulating
     GPIO state.
  4. Get rid of the assert in the interrupt handler.

This takes about a minute, and improves performance by about
a third:


```
int cost = 2637 [594 from gpio write until cycle_cnt_read()]
int cost = 2277 [576 from gpio write until cycle_cnt_read()]
int cost = 2342 [582 from gpio write until cycle_cnt_read()]
int cost = 2294 [576 from gpio write until cycle_cnt_read()]
int cost = 2275 [509 from gpio write until cycle_cnt_read()]
int cost = 2305 [587 from gpio write until cycle_cnt_read()]
int cost = 2300 [509 from gpio write until cycle_cnt_read()]
int cost = 2325 [579 from gpio write until cycle_cnt_read()]
int cost = 2312 [513 from gpio write until cycle_cnt_read()]
int cost = 2294 [576 from gpio write until cycle_cnt_read()]
int cost = 2303 [509 from gpio write until cycle_cnt_read()]
int cost = 2325 [582 from gpio write until cycle_cnt_read()]
int cost = 2270 [513 from gpio write until cycle_cnt_read()]
int cost = 2336 [601 from gpio write until cycle_cnt_read()]
int cost = 2306 [515 from gpio write until cycle_cnt_read()]
int cost = 2294 [587 from gpio write until cycle_cnt_read()]
int cost = 2297 [509 from gpio write until cycle_cnt_read()]
int cost = 2288 [581 from gpio write until cycle_cnt_read()]
int cost = 2303 [509 from gpio write until cycle_cnt_read()]
int cost = 2325 [582 from gpio write until cycle_cnt_read()]
```

And then, finally, we switch to a higher compiler optimization level,
`-Ofast`.  This which shaves another hundred or two cycles off:

```
int cost = 2358 [536 from gpio write until cycle_cnt_read()]
int cost = 2191 [641 from gpio write until cycle_cnt_read()]
int cost = 2130 [546 from gpio write until cycle_cnt_read()]
int cost = 2127 [577 from gpio write until cycle_cnt_read()]
int cost = 2130 [546 from gpio write until cycle_cnt_read()]
int cost = 2138 [588 from gpio write until cycle_cnt_read()]
int cost = 2132 [546 from gpio write until cycle_cnt_read()]
int cost = 2183 [591 from gpio write until cycle_cnt_read()]
int cost = 2130 [546 from gpio write until cycle_cnt_read()]
int cost = 2130 [580 from gpio write until cycle_cnt_read()]
int cost = 2132 [546 from gpio write until cycle_cnt_read()]
int cost = 2169 [580 from gpio write until cycle_cnt_read()]
int cost = 2127 [546 from gpio write until cycle_cnt_read()]
int cost = 2163 [582 from gpio write until cycle_cnt_read()]
int cost = 2088 [543 from gpio write until cycle_cnt_read()]
int cost = 2127 [577 from gpio write until cycle_cnt_read()]
int cost = 2088 [543 from gpio write until cycle_cnt_read()]
int cost = 2127 [577 from gpio write until cycle_cnt_read()]
int cost = 2093 [546 from gpio write until cycle_cnt_read()]
int cost = 2139 [580 from gpio write until cycle_cnt_read()]
```

Not bad for some pretty quick changes.


----------------------------------------------------------------------
### Step 2: inline inline inline.

In general, the second easy thing to do is to inline key calls.
This has the obvious benefit of getting rid of the procedure call and
return overhead.  It has the secondary (sometimes much more) benefit of
letting the compiler optimize  and specialize the function body to the
callsite.  

So make inline versions of:
  1. `gpio_read`
  2. `gpio_write`
  3. `gpio_event_clear`.

In addition, we cut out all error checking in the routines. 

I'd suggest copying your GPIO code here and make it a static
inline routine, with a slighly altered name (e.g., `gpio_read_raw`,
`gpio_set_off`, `gpio_set_on_raw`, etc).

This speeds up the code by a factor of two!  This is great, since it's
not hard:

```
int cost = 1224 [480 from gpio write until cycle_cnt_read()]
int cost = 953 [342 from gpio write until cycle_cnt_read()]
int cost = 911 [285 from gpio write until cycle_cnt_read()]
int cost = 931 [314 from gpio write until cycle_cnt_read()]
int cost = 914 [289 from gpio write until cycle_cnt_read()]
int cost = 930 [313 from gpio write until cycle_cnt_read()]
int cost = 908 [286 from gpio write until cycle_cnt_read()]
int cost = 925 [314 from gpio write until cycle_cnt_read()]
int cost = 911 [286 from gpio write until cycle_cnt_read()]
int cost = 925 [311 from gpio write until cycle_cnt_read()]
int cost = 917 [288 from gpio write until cycle_cnt_read()]
int cost = 931 [314 from gpio write until cycle_cnt_read()]
int cost = 914 [289 from gpio write until cycle_cnt_read()]
int cost = 930 [313 from gpio write until cycle_cnt_read()]
int cost = 911 [286 from gpio write until cycle_cnt_read()]
int cost = 925 [314 from gpio write until cycle_cnt_read()]
int cost = 911 [286 from gpio write until cycle_cnt_read()]
int cost = 931 [314 from gpio write until cycle_cnt_read()]
int cost = 923 [288 from gpio write until cycle_cnt_read()]
int cost = 930 [313 from gpio write until cycle_cnt_read()]
```

Somehow it also almost halved the cost of the initial cycle count read.

----------------------------------------------------------------------
### Step 3: specialize the event queue.

Currently we save the value read in the event queue.  If you run a
GPIO read in isolation you find out that its expensive, and the logic
to deal with is responsible for about half of the instructions in the
interrupt handler.  So we get rid of it and just store the cycle counter
values in a 32-bit array.

This makes a modest difference, but more importantly makes 
the machine code listing of the interrupt handler much 
easier to understand (see below):

```
int cost = 1238 [514 from gpio write until cycle_cnt_read()]
int cost = 875 [303 from gpio write until cycle_cnt_read()]
int cost = 976 [373 from gpio write until cycle_cnt_read()]
int cost = 878 [303 from gpio write until cycle_cnt_read()]
int cost = 903 [300 from gpio write until cycle_cnt_read()]
int cost = 878 [303 from gpio write until cycle_cnt_read()]
int cost = 903 [300 from gpio write until cycle_cnt_read()]
int cost = 878 [303 from gpio write until cycle_cnt_read()]
int cost = 903 [300 from gpio write until cycle_cnt_read()]
int cost = 875 [300 from gpio write until cycle_cnt_read()]
int cost = 901 [298 from gpio write until cycle_cnt_read()]
int cost = 878 [303 from gpio write until cycle_cnt_read()]
int cost = 903 [300 from gpio write until cycle_cnt_read()]
int cost = 878 [303 from gpio write until cycle_cnt_read()]
int cost = 900 [297 from gpio write until cycle_cnt_read()]
int cost = 878 [303 from gpio write until cycle_cnt_read()]
int cost = 897 [294 from gpio write until cycle_cnt_read()]
int cost = 878 [303 from gpio write until cycle_cnt_read()]
int cost = 903 [300 from gpio write until cycle_cnt_read()]
int cost = 878 [303 from gpio write until cycle_cnt_read()]
```

If we look at `scope.list` (something you'll have to do for the rest of
the speedups) the interrupt handler has the following machine code:

```
    8068:   e52de004    push    {lr}        ; (str lr, [sp, #-4]!)
    806c:   ee1fcf3c    mrc 15, 0, ip, cr15, cr12, {1}
    8070:   e3a00302    mov r0, #134217728  ; 0x8000000
    8074:   e59f201c    ldr r2, [pc, #28]   ; 8098 <int_vector+0x30>
    8078:   e59f101c    ldr r1, [pc, #28]   ; 809c <int_vector+0x34>
    807c:   e5923000    ldr r3, [r2]
    8080:   e283e001    add lr, r3, #1
    8084:   e0823103    add r3, r2, r3, lsl #2
    8088:   e582e000    str lr, [r2]
    808c:   e583c004    str ip, [r3, #4]
    8090:   e5810040    str r0, [r1, #64]   ; 0x40
    8094:   e49df004    pop {pc}        ; (ldr pc, [sp], #4)
    8098:   00009a60    .word   0x00009a60
    809c:   20200000    .word   0x20200000
```

The worse thing here are the load instructions (the `ldr` instructions
at 8074, 8078 and 807c).  These will be cache misses since we aren't
running with the data cache.  We could  enable the data cache, but 
that will:
  1. Require virtual memory and a bunch of complexity.
  2. Generally only usually, not always work --- thereby introducing
     big spikes of jitter / error.

Instead we just try to get rid of the load instructions.  In this case,
if you look at the code, the first two loads are of the constants at
8098 and 809c -- the address of the event queue pointer and the GPIO
event clear address.

So we use a trick from the pixie lab: load these into the scratch
registers!
  1. Before running the test code, load the address of the event
     clear into one scratch register and the address of the global
     pointer into another.
  2. Change the test generation to use the helpers.

My interrupt code looks like:
```
void int_vector(uint32_t pc) {
    unsigned s = cycle_cnt_read();

    let c = cur_time_get();
    *c = s;
    cur_time_set(c+1);

    let e = event0_get();
    *e = 1 << in_pin;
}
```
Where `cur_time_get` and `cur_time_set` uses one scratch register,
and the `event0_get` uses a second one.

This makes a huge difference!  Almost 2x again!

```
int cost = 704 [277 from gpio write until cycle_cnt_read()]
int cost = 580 [297 from gpio write until cycle_cnt_read()]
int cost = 597 [282 from gpio write until cycle_cnt_read()]
int cost = 583 [300 from gpio write until cycle_cnt_read()]
int cost = 599 [282 from gpio write until cycle_cnt_read()]
int cost = 583 [300 from gpio write until cycle_cnt_read()]
int cost = 602 [282 from gpio write until cycle_cnt_read()]
int cost = 574 [291 from gpio write until cycle_cnt_read()]
int cost = 599 [282 from gpio write until cycle_cnt_read()]
int cost = 583 [300 from gpio write until cycle_cnt_read()]
int cost = 594 [279 from gpio write until cycle_cnt_read()]
int cost = 580 [297 from gpio write until cycle_cnt_read()]
int cost = 600 [282 from gpio write until cycle_cnt_read()]
int cost = 580 [297 from gpio write until cycle_cnt_read()]
int cost = 597 [282 from gpio write until cycle_cnt_read()]
int cost = 581 [297 from gpio write until cycle_cnt_read()]
int cost = 597 [282 from gpio write until cycle_cnt_read()]
int cost = 583 [300 from gpio write until cycle_cnt_read()]
int cost = 594 [279 from gpio write until cycle_cnt_read()]
int cost = 583 [300 from gpio write until cycle_cnt_read()]
```

If you look at the interrupt handler code (our favorite theme song)
you can see this one changed removed the two loads we wanted, but as
a extravagant bonus removed alot more (this is not uncommon):
  1. B/c we use less registers, the compiler could eliminate the
     push and pop (dcache miss) of the lr register.
  2. Reduced all the memory operations to exactly two stores:
     one to clear the interrupt and one to write the cycle counter.

```
    8084:   ee1f2f3c    mrc 15, 0, r2, cr15, cr12, {1}
    8088:   ee1d3f70    mrc 15, 0, r3, cr13, cr0, {3}
    808c:   e4832004    str r2, [r3], #4
    8090:   ee0d3f70    mcr 15, 0, r3, cr13, cr0, {3}
    8094:   ee1d3f50    mrc 15, 0, r3, cr13, cr0, {2}
    8098:   e3a02302    mov r2, #134217728  ; 0x8000000
    809c:   e5832000    str r2, [r3]
    80a0:   e12fff1e    bx  lr
```

----------------------------------------------------------------------
### Step 4: housekeeping

Now that we reduced our timings down so much, we do some basic housekeeping
so that we don't get spurious speedups and slowdowns because of changing
alignment.  Recall that the instruction prefetch buffer is 32-bytes.  If 
our code can be read in a single prefetch it will run noticably faster
than if it takes two.  Unfortunately if we don't force alignment, random
changes in the code can change the alignment of unrelated locations leading
to big timing swings.  (We should have done this sooner, but I forgot and
am too lazy to remeasure.)

We care about:
  1. The interrupt trampoline.
  2. The interrupt handler itself.
  3. And, the two locations in the test gen where we write to the GPIO
     and read the cycle counter.

We force alignment by:
  1. In the assembly add an ".align 32" before the interrupt trampoline
     `interrupt`.
     
  2. In the C code before the interrupt handler:

        __attribute__((aligned(32))) void int_vector(uint32_t pc) {

  3. In the tst gen, add a `asm volatile(".align 5")` before both
     cycle count reads in `test_gen`:

        asm volatile(".align 5");
        s = cycle_cnt_read();
        gpio_set_on_raw(pin);

You can check that all of these worked by looking for the addresses of
each and making sure that they are divisible by 32.

I'm a bit surprised, but it didn't seem to make much difference
for me.  But on the positive side, it does reduce fluctuations
when we cut more cycles.

```
int cost = 731 [308 from gpio write until cycle_cnt_read()]
int cost = 588 [307 from gpio write until cycle_cnt_read()]
int cost = 596 [279 from gpio write until cycle_cnt_read()]
int cost = 591 [308 from gpio write until cycle_cnt_read()]
int cost = 596 [279 from gpio write until cycle_cnt_read()]
int cost = 591 [308 from gpio write until cycle_cnt_read()]
int cost = 604 [285 from gpio write until cycle_cnt_read()]
int cost = 588 [308 from gpio write until cycle_cnt_read()]
int cost = 605 [286 from gpio write until cycle_cnt_read()]
int cost = 591 [308 from gpio write until cycle_cnt_read()]
int cost = 602 [285 from gpio write until cycle_cnt_read()]
int cost = 588 [307 from gpio write until cycle_cnt_read()]
int cost = 596 [279 from gpio write until cycle_cnt_read()]
int cost = 591 [308 from gpio write until cycle_cnt_read()]
int cost = 604 [285 from gpio write until cycle_cnt_read()]
int cost = 588 [308 from gpio write until cycle_cnt_read()]
int cost = 605 [286 from gpio write until cycle_cnt_read()]
int cost = 591 [307 from gpio write until cycle_cnt_read()]
int cost = 601 [282 from gpio write until cycle_cnt_read()]
int cost = 591 [308 from gpio write until cycle_cnt_read()]
```


----------------------------------------------------------------------
### Step 4: accurate cycle counter reads

We still have a big source of error for the cycle counter.  We will again
use a trick from our pixie lab and read the cycle counter on the first
instruction of the assembly trampoline.    As with pixie, since we are
in the interrupt handler we only have a single private register (`sp`) so:

  1. We read the cycle counter into SP.
  2. We then move SP into the third scratch register.  (This works
     because the machine has three scratch registers and we only 
     used two so far.)
  3. The trampoline works as before, except instead of 
     passing the PC as the first argument, we pass in the 
     cycle count value (by reading it from the scratch register).
  4. Change the interrupt handler to remove the cycle count read.
  
This removes about 1/3 of the overhead of our initial read.  It also
makes the variance go almost to 0 except for the first read (I'm not
sure what is going on, since the cache is disabled.)  This was
a great result for such a simple change:

```
int cost = 756 [254 from gpio write until cycle_cnt_read()]
int cost = 588 [199 from gpio write until cycle_cnt_read()]
int cost = 583 [199 from gpio write until cycle_cnt_read()]
int cost = 583 [198 from gpio write until cycle_cnt_read()]
int cost = 580 [196 from gpio write until cycle_cnt_read()]
int cost = 582 [195 from gpio write until cycle_cnt_read()]
int cost = 585 [198 from gpio write until cycle_cnt_read()]
int cost = 585 [198 from gpio write until cycle_cnt_read()]
int cost = 585 [198 from gpio write until cycle_cnt_read()]
int cost = 585 [198 from gpio write until cycle_cnt_read()]
int cost = 585 [198 from gpio write until cycle_cnt_read()]
int cost = 582 [195 from gpio write until cycle_cnt_read()]
int cost = 585 [198 from gpio write until cycle_cnt_read()]
int cost = 585 [198 from gpio write until cycle_cnt_read()]
int cost = 585 [198 from gpio write until cycle_cnt_read()]
int cost = 585 [198 from gpio write until cycle_cnt_read()]
int cost = 585 [198 from gpio write until cycle_cnt_read()]
int cost = 582 [195 from gpio write until cycle_cnt_read()]
int cost = 585 [198 from gpio write until cycle_cnt_read()]
int cost = 585 [198 from gpio write until cycle_cnt_read()]
```

----------------------------------------------------------------------
### Step 5: do it all in assembly


One nice thing about trimming so many instructions is that now the
interrupt handler machine code is tiny, which means we can easily just
write it directly in assembly code.  This will eliminate:
  1. Most of the overhead the trampoline has of saving and restoring the
     registers before calling the C code.

My current interrupt handler looks like:

```
    80a0:   ee1d3f70    mrc 15, 0, r3, cr13, cr0, {3}
    80a4:   e4830004    str r0, [r3], #4
    80a8:   ee0d3f70    mcr 15, 0, r3, cr13, cr0, {3}
    80ac:   ee1d3f50    mrc 15, 0, r3, cr13, cr0, {2}
    80b0:   e3a02302    mov r2, #134217728  ; 0x8000000
    80b4:   e5832000    str r2, [r3]
    80b8:   e12fff1e    bx  lr
```

So I took most of this code (not the "bx lr") and inlined it into
`interrupt-asm.S`.  After doing so, I could whittle down the code to
only use two registers (in addition to sp).


Removing the memory operations makes a big difference!

```
int cost = 560 [314 from gpio write until cycle_cnt_read()]
int cost = 388 [198 from gpio write until cycle_cnt_read()]
int cost = 380 [196 from gpio write until cycle_cnt_read()]
int cost = 388 [198 from gpio write until cycle_cnt_read()]
int cost = 382 [198 from gpio write until cycle_cnt_read()]
int cost = 388 [198 from gpio write until cycle_cnt_read()]
int cost = 382 [198 from gpio write until cycle_cnt_read()]
int cost = 388 [198 from gpio write until cycle_cnt_read()]
int cost = 382 [198 from gpio write until cycle_cnt_read()]
int cost = 388 [198 from gpio write until cycle_cnt_read()]
int cost = 382 [198 from gpio write until cycle_cnt_read()]
int cost = 388 [198 from gpio write until cycle_cnt_read()]
int cost = 382 [198 from gpio write until cycle_cnt_read()]
int cost = 388 [198 from gpio write until cycle_cnt_read()]
int cost = 382 [198 from gpio write until cycle_cnt_read()]
int cost = 388 [199 from gpio write until cycle_cnt_read()]
int cost = 382 [198 from gpio write until cycle_cnt_read()]
int cost = 388 [198 from gpio write until cycle_cnt_read()]
int cost = 380 [196 from gpio write until cycle_cnt_read()]
int cost = 388 [198 from gpio write until cycle_cnt_read()]
```

(Note one easy fix we could have done earlier was only save the
caller-saved registers in the trampoline.)  


----------------------------------------------------------------------
### Step 6: do it as a "fast interrupt" (FIQ)

Looking at the machine code, we still push an pop two registers,
which means we have to have a stack pointer, as well as some 
extra management.    We can eliminate all of this by using  
"fast interrupt" mode.  If you look in chapter 4 of the
armv6 document you can see that FIQ mode has six shadow
registers, R8-R14.

<img src="images/shadow-regs-pA2-5.png" width="450" />

How do we do this?  If you look at the BCM interrupt chapter (see
lab 4 "interrupts" and lab 8 "device interrupts" of 140e), you can 
see how to set up the FIQ.

The FIQ reg itself:
<img src="images/fiq-reg-p116.png" width="450" />

So we have to set the 7th bit to 1 and write the interrupt
source into the lower 6 bits.  The interrupt source is
given in:
<img src="images/irq-table-p113.png" width="450" />
Since we want one of the first 32 GPIO pins, this is GPIO0, which is 49.

There's different ways to do this.  The easiest way for me was to make
versions of my gpio interrupt routines that setup the FIQ instead.


    void gpio_fiq_rising_edge(unsigned pin);
    void gpio_fiq_falling_edge(unsigned pin);


And use these during setup.  I also made a special FIQ table, and an 
FIQ initialization routine in assembly.  

So `notmain` becomes:

```
    void notmain(void) {
        ...
    
        // setup FIQ
        extern uint32_t fiq_ints[];
        vector_base_set(fiq_ints);
        output("assigned fiq_ints\n");

        gpio_fiq_rising_edge(in_pin);
        gpio_fiq_falling_edge(in_pin);

        fiq_init();
```

To initialize the FIQ registers I used the "cps" instruction to switch
into `FIQ_MODE` and setup the FIQ registers to hold the pointers and
values I want, and then back to `SUPER_MODE` (make sure you prefetch
flush!).  I then put a panic in the original `int_handler` to verify we
weren't calling it.

And finally as a hack I used the preprocessor to give the different
registers "variable names" so I didn't do any stupid mistakes.

    // in interrupt-asm.S
    #define event0      r8
    #define event0_val  r9
    #define cur_time    r10
    #define cur_cycle   r11


```
int cost = 326 [149 from gpio write until cycle_cnt_read()]
int cost = 294 [154 from gpio write until cycle_cnt_read()]
int cost = 321 [145 from gpio write until cycle_cnt_read()]
int cost = 291 [151 from gpio write until cycle_cnt_read()]
int cost = 321 [145 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 322 [145 from gpio write until cycle_cnt_read()]
int cost = 294 [153 from gpio write until cycle_cnt_read()]
int cost = 322 [145 from gpio write until cycle_cnt_read()]
int cost = 283 [142 from gpio write until cycle_cnt_read()]
int cost = 321 [145 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 321 [145 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 321 [145 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 322 [145 from gpio write until cycle_cnt_read()]
int cost = 294 [154 from gpio write until cycle_cnt_read()]
int cost = 321 [145 from gpio write until cycle_cnt_read()]
int cost = 294 [154 from gpio write until cycle_cnt_read()]
```

The variance was weird, so I looked into the code and
somehow I'd deleted the align 5 in the test gen code.
(The great thing about using perforance counters is that
there is no room to hide.  Also, the more details you see,
the more you have a chance to see "that's weird").

When I fixed this, the variance went very flat, justifying
why we intended to do it in the first place :)

```
int cost = 295 [154 from gpio write until cycle_cnt_read()]
int cost = 306 [166 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 292 [151 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 297 [157 from gpio write until cycle_cnt_read()]
int cost = 292 [151 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 297 [157 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 297 [157 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 297 [156 from gpio write until cycle_cnt_read()]
int cost = 297 [157 from gpio write until cycle_cnt_read()]
```


We are now at more than 10x a performance improvement.  And, crucially
for a digital scope, almost always 1 cycle or less in variance (I'm
still not sure why the initial readings bounce around so much).

----------------------------------------------------------------------
### Step 7: Icache.

This is easy.  We turn on the icache.  This make a big difference
in performance, but also increases variance:

```
int cost = 247 [138 from gpio write until cycle_cnt_read()]
int cost = 170 [97 from gpio write until cycle_cnt_read()]
int cost = 147 [75 from gpio write until cycle_cnt_read()]
int cost = 142 [70 from gpio write until cycle_cnt_read()]
int cost = 147 [75 from gpio write until cycle_cnt_read()]
int cost = 142 [70 from gpio write until cycle_cnt_read()]
int cost = 147 [75 from gpio write until cycle_cnt_read()]
int cost = 142 [70 from gpio write until cycle_cnt_read()]
int cost = 147 [75 from gpio write until cycle_cnt_read()]
int cost = 142 [70 from gpio write until cycle_cnt_read()]
int cost = 147 [75 from gpio write until cycle_cnt_read()]
int cost = 141 [72 from gpio write until cycle_cnt_read()]
int cost = 147 [75 from gpio write until cycle_cnt_read()]
int cost = 142 [70 from gpio write until cycle_cnt_read()]
int cost = 147 [75 from gpio write until cycle_cnt_read()]
int cost = 141 [72 from gpio write until cycle_cnt_read()]
int cost = 147 [75 from gpio write until cycle_cnt_read()]
int cost = 142 [70 from gpio write until cycle_cnt_read()]
int cost = 147 [75 from gpio write until cycle_cnt_read()]
int cost = 142 [70 from gpio write until cycle_cnt_read()]
```

The first miss we can eliminate by either doing a warmup or a prefetch.
While we've brough down the time, we also increased the initial cache
read variance --- interesting!  Worth looking into.

----------------------------------------------------------------------
### Now what?

We are currently at about 23x performance improvement (3500/147) and
about 7x improvement of accuracy (529/75), which is a great improvement.
This is one of the reasons I really like the bare metal code we do:
  1. It would be hopeless to do this lab in Linux or MacOS.  The 
     time, the constant fight, the complexity.  Big NFW.
  2. Since the code is small, and written by us it's easy to quickly get
     into a flow state, directly touching hardware reality, doing
     interesting things.  That's not the norm.

Fair enough, so now what.

There's still some obvious stuff we can do or at least try:
  1. Overclock the pi.  We can push the CPU higher and also the 
     BCM speed (IIRC), though we then have to adjust the UART or
     we won't see anything.
  2. We could possibly use the DMA to clear the event and write the
     cycle count out.   Given some prep work I think we can do this with
     a single store. 
  3. One potential method: turn on VM and do traps of the GPIO memory.
     This *might* be faster + lower variance for getting the initial
     cycle count read since it doesn't go through the bcm at all, which
     runs at 250mhz).  We could alias the GPIO memory at a different
     offset so we can just write to it without having to play domain
     tricks.
  4. We could get a cleaner test signal by using the clock to generate
     it.  This would require checking when we have too many samples.
     A hack to do this without an if statement is to set a watchpoint
     on the end of the queue.

  5. We'll give a major extension if you make a significant improvement
     using some alternative method!

Note: I did try to not use the interrupt vector and copy the interrupt
code but it didn't seem to improve anything.

--------------------------------------------------------------------
--------------------------------------------------------------------
--------------------------------------------------------------------
--------------------------------------------------------------------
--------------------------------------------------------------------
LEft over text.  Ignore.

### Why?

We spent a fair amount of time trying to tune our addressable light array
code, focusing on micro-benchmarks.   However, these microbenchmarks
don't guarantee that when you compose the code together that its timing
stays what we need it to be. Unfortunately ours does not: if you look at
your code, there are if-statements, loops, and other sources of overhead
that will push all our timings up.  The true test of the code is what happens
on the jumper that connects the pi to the light array:  
  - Does the jumper go to 1 (3v) when it should, and for the right number of
    nanoseconds?
  - Does it go 0 when it should, and for the right number of nanoseconds?

Whatever happens inside the pi is one thing, but what happens on the outside
is what happens in reality.     Today you'll build a fast interrupt based
digital scope that can check these timings.  Historically we have used two
pi's to do this:
  1. A test pi running code we want to check, such as the ws2812b code.
  2. A scope pi connected by jumpers to (1) that records when the
     output from the test pi goes hi or low and at what time.  Ideally
     we have error in the low number of nanoseconds.  Note: given the 
     pi runs at 700mhz and a pi cycle is 1.4ns we can't do better than
     that.  

The downside of this approach is cost (we have to buy equipment) and,
worse, it's more than 2x the annoyance to get 2 pi's working on your
laptop and coordinating them.

So we'll start with a single pi:
  1. Connect a loopback jumper from an output GPIO pin (the test signal) 
     to an input GPIO pin (the scope input).
  2. Setup GPIO interrupts so we get an interrupt when the input pin
     goes from hi to low.

Because of the complexity they introduce, I'm much more blackpilled than
most on using interrupts at all.  Our group has made a reasonable amount
of money because people can't even write sequential code correctly.
Adding interrupts makes it much worse.

HOWEVER, this is one case where interrupts let us reduce error.  On our
arm1176 handling a GPIO interrupt is much more expensive than simply
reading a GPIO pin.  There is a lot that happens from the initial action
of a GPIO pin switching from low to high (or high to low) all the way
until the first instruction in interrupt.  But (and this is key), all
of these actions occur in hardware and --- at least for the arm1176 ---
appear to follow a deterministic, predicatable path with stable timing.




a GPIO pin was written until the GPIO pin was


I'm generally down on using interrupts unless you have to.



A digital scope --- detecting when the signal goes high and low and for
how many nanoseconds,


it seens the right signal at the 

However, the true test of the code
is if it sends the signals at the right time.  This 
