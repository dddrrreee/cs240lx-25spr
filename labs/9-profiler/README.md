## Instruction level profiler

When I was a kid, we had a useful instruction-level profiler "pixie" on
SGI workstations that would do binary rewriting and count the exact number
of times you ran each individual machine code instruction in your code.
It was exact in that it counted everything, rather than a statistical profiler
like `gprof` that only sampled.

Pixie was great.  There was nothing I couldn't speed up by many factors
if I used it.  There is some chance that without it I wouldn't have
gotten into MIT for grad school (and thus here to this lab) since my
demonstrated competence was making stuff fast and without pixie this
would have been hard.  So I have a fondness for it.

Of course, SGI died and binaries became hard to rewrite so you probably
haven't used anything like it.  Also, such things generally don't run
on kernel code.

So today we'll build a simple version.  Using the ARM single-stepping
hardware makes it pretty easy.  If you add the counters from the ARM
performance monitor, things get interesting in that its easy to see why
different parts of the code are taking longer than others.  And it's
not to hard to make it run on kernel code.

External motivation:
  - It's midterm week so we are keeping labs a bit light.
  - You'll need to understand single stepping next week when we build 
    our memory tracing tools (they use it).  A profiler is a nice
    way to kick the hardware around and see how things work.

Useful 140e labs:
  - [single-step][single-step]: this discusses the single step hardware
    and how to build the breakpoint veneer.
  - [interrupt][interrupts]: this gives a working interrupt lab and discusses
    shadow registers.

Checkoff:
  1. Write a simple profiler similar to the `gprof` in 140e.
  2. Add performance counters and display both the number of times 
     each instruction runs, the cycle counts, and the value of some 
     interesting hardware counters.

There are tons of extensions.  Will add!  Or ask.

------------------------------------------------------------------
### Single step example: `code/`

We give a complete working single-step example in the `code/` directory. It's
called "ss-pixie" (single-step pixie) in honor of its ancestor.

The code has two public routines:
  - `pixie_start()`: start single step tracing
  - `unsigned pixie_stop()`: stop tracing and return the number of instructions
    executed.

At a high level it works by using "mismatch faults" to single-step
through code between these two calls.

As you might recall from 140e, mismatch faults only happen when for code
running at user level, so at a high level the code works as follows:

  1. `pixie_start`: sets up the exception vectors to catch mismatch
     faults and to handle system calls (see below), enables the debug
     hardware, and switches to user mode.

  2. As soon as the code run at user level, it will get mismatch faults
     (which get vectored to the "prefetch abort" handler).  This handler
     counts the instructions and sets the next fault.

  3. When the code wants to turn off tracing it calls `pixie_stop` to turn
     off mismatching and switch back to privileged mode.

     This routine can't do either directly since it is at user level ---
     instead it does a system call to elevate privileges, and in the
     system call handler turns off the faults, switch back to privileged
     mode, and return back to the calling code.


You should poke around the example.  It's heavily commented.

------------------------------------------------------------------
### Part 1: turn `ss-pixie` into an instruction profiler

Similar to the `gprof` code you wrote in 140e, you'll add an 
array that is the size of the code segment and increment it
each time an instruction runs.

The basic algorithm:

 1. Use `kmalloc` (or, better: your ckalloc!) to allocate an array at
    least as big as the code.  Compute the code size
    using the labels defined in `libpi/memmap` (we give C definitions in
    `libpi/include/memmap.h`).

 2. In the fault handler, use the program counter value (register 15)
    to index into this array and increment the associated count.  NOTE:
    its very easy to mess up sizes.  Each instruction is 4 bytes, so
    you'll divide the `pc` by 4.  You'll want to subtract where the code
    starts (0x800).

 3. You should define some way to print out the top, sorted, non-zero
    values in this array along with the `pc` value they correspond to.
    You should be able to look in the disassembled code (the `.list`
    file for each routie) to see which instruction these correspond to.

 4. For the simple test `tests/1-prof-test.c` that just repeatedly prints,
    the expected results are most counts should be in `PUT32`, `GET32`,
    and various `uart` routines.

Write a couple tests and validate that your profiler eats them and spits
out interesting values.  For interesting tests, please post to Ed so we
can steal them (add your name / year).

------------------------------------------------------------------
### Part 2: add suppot for cycle counters

Counting instructions is good, but we would also like to count the number
of cycles each instruction costs.  When there is a big difference between 
these it's interesting.

We have cycle counter routines in `libpi/include/cycle-count.h` so it
might seem easy.  Unfortunately, our single step handler does so much 
compared to running a single instruction that just recording them would
be useless.

Ideally, what we would want to do instead is:
  1. As the first line of the fault handler, record the cycle count.
  2. At the last line of the fault handler, record the cycle count.
  3. Subtracting (1) from (2)  gives the difference.  (Note, of course,
     by Heisenburg that there will still be perturbations, but if
     small enough, it gets more deterministic and we can subtrat it).


How can we do this?  Various problems:
  1. How can we read the cycle counter?  We don't have any registers!
     Fortunately, since the ARM registers are untyped we can use sp.
  2. Ok: so what about at the end?  If we put this in sp, we won't
     have any place to do the read in (1).  

     Fortunately, the arm1176 provides (at least) three coprocessor
     registers for "process and thread id's."  However, since the
     values are not interpreted by the hardware, they can be used to
     store arbitrary values.  The screenshot of the manual below gives
     the instructions.

     Thus, using these we can put (1) and (2) in them and use them in
     the handler.

What to do:
  1. Use the scratch registers to record the cycle counter at the 
     start and end of the handler.  Try to write the code so 
     the reads are as close to the start and end as possible.

  2. You'll have to subtract off a correction factor.  The cleaner
     step 1 is, the more stable this factor will be.

  3. Write some simple code that you know the answer to and validate
     that you get useful answers.


<p align="center">
  <img src="images/global-regs.png" width="800" />
</p>

Note: this is a good reason to reach chapter 3 of the arm1176:
there are all sorts of weirdo little operations that when you
add cleverness can let you do neat stuff not possible on a
general purpose OS.


------------------------------------------------------------------
### Part 3: Implement PMU counters

***When you get here do a pull: filling stuff in***

The arm1176 has a bunch of interesting performance counters
we can use to see what is going on, such as cache misses, TLB misses,
prediction misses, procedure calls.  You'll write a simple library that
exposes these, which will make it much easier to optimize code.

The arm1176 document describes the performance monitor unit (PMU) on pages
3-133 --- 3-140.  It has many useful performance counters, though with the
limit that only two can be enabled at any time in addition to the "always
on" cycle counter.  We'll write a simple interface to expose these.

These counters make it much easier to speed up your code --- it's hard to
know the right optimization when you don't know what the bottleneck is.
In addition, they can be used to test your understanding of the hardware
--- if you believe you understand how the BTB, TLB, or cache works, write
code based on this understanding and measure if the expected result is
the actual.

#### Implement `code/armv6-pmu.h`

Fill in the `unimplemented` routines in `code/armv6-pmu.h`.  I'd suggest
using the `cp_asm` macros in

        #include "asm-helpers.h"

You probably should have an enum for all the different types in
the header too.  E.g.,

        enum {
            PMU_INST_CNT = 0x7,
            ...
        };

What you need:
   1. Performance monitor control register (3-133): write this to
      select which performance counters to use (the values are in tabel
      3-137) and to enable the PMU at all.
   2. Cycyle counter register (3-137): read this to get the current
      cycle count.
   3. Count register 0 (3-138): read this to get the 32-bit 0-event
      counter (set in step 1).
   4. Count register 1 (3-139): read this to get the 32-bit 1-event
      counter (set in step 1).


### Write some code that shows off some of the other performance counters.

This is a choose-your-own adventure:  look through the counters and
write some code that shows off something they can measure (or run on your
140e or 240lx code!).  The easiest way is to copy the header files into
`libpi/include` directory and they should just work.

------------------------------------------------------------------
### Part 4: Use PMU counters in your profiler

***When you get here do a pull: filling stuff in***

For this, you'll take the assembly from part 3 and add the counters
to the prefetch abort fault handler trampoline.

[single-step]: https://github.com/dddrrreee/cs140e-25win/tree/main/labs/9-debug-hw
[interrupt]: https://github.com/dddrrreee/cs140e-25win/tree/main/labs/4-interrupts
