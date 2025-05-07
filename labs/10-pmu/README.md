## Performance counters

Since multiple people remarked that the profiler lab was fun, we'll 
double-down and do some performance counter puzzles.  
It will have a similar style:
  1. Short README
  2. Short starter code.
  3. Real puzzles.

The arm1176 has a bunch of interesting performance counters, such as cache
misses, TLB misses, prediction misses, procedure calls.  You'll write
a simple library that exposes these, which will make it much easier to
optimize code.

The counters are provided by the "performance monitor unit" (PMU)
described on pages 3-133 --- 3-140. Their main unfortunate limit
is that:
  1. Only two can be enabled at any time (in addition to the "always
     on" cycle counter).
  2. You can't read them "all at once" but need to read them
     one at a time, which can introduce measurement perturbation.

These counters make it much easier to speed up your code --- it's hard to
know the right optimization when you don't know what the bottleneck is.

In addition, they can be used to test your understanding of the hardware.
--- if you believe you understand how the BTB, TLB, or cache works, write
code based on this understanding and measure if the expected result is
the actual.  In almost all cases, the initial measurements will show
something was wrong --- either in your belief, in the code that you
wrote to measure, in how you read the docs.  But while counters take away
happiness, they also typically give a way to work towards enlightenment,
in that you can use them to differentially debug what is going on,
and arrive at a better place.

A big empirical motivation for this lab is that with a small amount of
infrastructure and tiny measurement programs you can get into interesting
territory in minutes.  Over the weekend, pretty much every time I tried
to measure something I'd blurt out some permutation of:
 - "that's weird";
 - "that's interesting";
 - "wtf"
 - some pacing + "oh cool!"

General heuristic: when you are doing something, can it surprise you?
The more yes, the more exothermic it will be in terms of insight and
results.

Enough yap, let's code.

### Checkoff

Base checkoff:
   - Part 1: Write the PMU routines.
   - Part 2: write the smallest program to show the effect of 
             alignment on the prefetch buffer.
   - Part 2 and Part 3: write 4-5 small programs that cause the
     counters to do what you intend.   Pick whatever you want.

     Note: if your result is surprising enough, we'll take one.

Daniel mode:
  - Daniel mode for this lab is easy since we don't depend on it
    later.  Write the code yourself using whatever interfaces you
    want and figure out 4-5 interesting things.

------------------------------------------------------------------
#### Warning: you'll waste time b/c of this.

A major, constant source of problems here is that our observations
can easily perturb the experiment.  This is especially true since the
compiler transforms them:
  - If your code behaves weirdly, it could be because of what the 
    compiler did to the code *doing* the measurement, not that
    the measured code does something weird.

Incomplete set of Ways to prevent:
  - Always look at the disassembled code in the list.  It's going to
    be confusing, but you can generally scan for the repeated "mcr" and
    "mrc" instructions to see where things start and end.

  - If we want to measure X and aren't careful the compiler will
    smear X outside of the measurement.  We partially use compiler
    memory barriers for this, but they aren't guaranteed.  You can also
    put the code in another file (our version of gcc won't do inter-file
    optimization).  Or, if you want to really be sure, in a separate
    assembly file.  We provide empty files `measure-fns.c` and `measure-asm.S`
    for this.

  - Alignment can have a big impact that changes each time you re-link.
    Using the ".align" directive can help.

  - If you have the i-cache on --- each access it will bring in an
    entire cache blocks.  This can cause weird things (why?)

  - Ideally if you figure out something, cross-check it a second
    way.  Sometimes bullshit gives you the answer you expected
    and you declare success when in fact all you had was nonsense.
    (Be me, today.)

  - As mentioned, one easy way to defeat the compiler is to write your
    own assembly. In addition when you are trying to have precise
    control over layout it can be easier to allocate a large
    block and JIT into it.

------------------------------------------------------------------
#### Part 1: Implement `code/rpi-pmu.h`

***NOTE: In the interests of getting people writing the weird programs
we actually pushed an implementation of this code/rpi-pmu.h.staff***

The first thing to do is to define the PMU routines.  In the interest of
getting to puzzle quickly we give you a bunch of the definitions and some
helper macros --- you have to write the low-level manipulation routines.

Fill in the `todo` routines in `code/rpi-pmu.h`.  I'd suggest
using the `cp_asm` macros in

        #include "asm-helpers.h"

The header defines some helper macros for you; the tests below show
how to use them.

What you need:
   1. Performance monitor control register (3-133): write this to
      select which performance counters to use (the values are in table
      3-137) and to enable the PMU at all.
   2. Cycle counter register (3-137): read this to get the current
      cycle count.
   3. Count register 0 (3-138): read this to get the 32-bit 0-event
      counter (set in step 1).
   4. Count register 1 (3-139): read this to get the 32-bit 1-event
      counter (set in step 1).


When you are done, the following tests should run:
 1. `0-pmu-test.c`: this does a simple check that your PMU code
    works and then measures the cost of some nops using the
    cycle counter.  You can see the difference when it runs 
    uncached and cached.
```    
    cyc_s       = pmu_cycle_get();      // always on cycle counter

    asm volatile("nop");  // 1
    asm volatile("nop");  // 2
    asm volatile("nop");  // 3
    asm volatile("nop");  // 4
    asm volatile("nop");  // 5

    ...

    cyc_e       = pmu_cycle_get();      // always on cycle counter
```    

   NOTE: We'll discuss this code below, since it already has some
   weirdness.


 2. `1-pmu-test.c`: this uses the event counters to also count
    instructions and the number of stalls.  

    Puzzle: can you rewrite this code so we get cleaner measurements?

```
    pmu_enable(0, PMU_inst_cnt);
    pmu_enable(1, PMU_inst_stall);

    cyc_s       = pmu_cycle_get();      // always on cycle counter
    inst0_s     = pmu_event_get(0);     // instruction count
    stall1_s    = pmu_event_get(1);     // stalls

    asm volatile("nop");  // 1
    asm volatile("nop");  // 2
    asm volatile("nop");  // 3
    asm volatile("nop");  // 4
    asm volatile("nop");  // 5

    cyc_e       = pmu_cycle_get();      // always on cycle counter
    inst0_e     = pmu_event_get(0);     // instruction count
    stall1_e    = pmu_event_get(1);     // stalls

```


 3. `2-pmu-test.c`: as you  can see from the above code,
    there's a lot of repetitive code for measuring.  This test uses the
    helper macro `pmu_stmt_measure` measure the code with less typing.

    The macro takes a message to print, two types of events, and then
    a statement block that it runs.

    The definition is in `rpi-pmu.h`.   You can change it if you want a
    different output!  In fact you probably should rewrite it a bit to
    make results cleaner (why?).

```
    pmu_stmt_measure(msg,
            inst_cnt,
            inst_stall,
    {
        asm volatile("nop");  // 1
        asm volatile("nop");  // 2
        asm volatile("nop");  // 3
        asm volatile("nop");  // 4
        asm volatile("nop");  // 5

        asm volatile("nop");  // 1
        asm volatile("nop");  // 2
        asm volatile("nop");  // 3
        asm volatile("nop");  // 4
        asm volatile("nop");  // 5
    });
```

### Weirdness!

Just from the trivial little `0-pmu-test.c` program we already run into
something weird.  In the code that measures the cycle counts there is
a commented out directive:

```
    // asm volatile(".align 4");
    cyc_s       = pmu_cycle_get();      // always on cycle counter
```

If you uncomment it out, it will cause the code to be aligned to a 16-byte
alignment (check using `0-pmu-test.list`).  Oddly, when I did this, the
code slows down.  Usually we would expect the code to run faster if it
had good alignment, which cycle counting turns into a testable hypothesis
(whether we want it to or not :).

First puzzle: why would it slow it down?  If you can figure this out,
lmk --- it's a good representative puzzle :) 

------------------------------------------------------------------
#### Part 2: write tiny programs to show these counters.

Use the counters to figure out:

  1. Using cycle counters: With the i-cache off, the arm1176 will still
     fetch multiple instructions in a small prefetch buffer.  Use the
     cycle counter and the align directive to figure out how alignment
     interacts with this.

     Bonus: do you have a clever trick to figure out the prefetch
     buffer size?

  2. Chapter 3 of 1176, p 3-74 has the instruction to invalidate the
     i-cache.  Implement it, and use the counters to show that your 
     implementation works.

     JIT some code to show that without this you get stale results,
     and with it, you get accurate ones.

  3. Chapter 3 of 1176: p 3-75: has instructions to invalidate a i-cache
     block by "mva" and to prefetch by mva.  Implement these
     and use the counters to show that your implementation works as
     expected.

     JIT some code to show that without this you get stale results,
     and with it, you get accurate ones.

  4. Chapter 3 of 1176: 3-22: has the instruction to get the 
     cache types.  Get the type, print it out, and write
     some code that uses the counters to test the claimed
     cache attributes.

  5. Use the counters to figure out the branch prediction algorithm.  

  6. Since the d-cache and tlb counters require virtual memory, we
     pushed a virtual memory implementation `code-vm/dcache-test.c` that
     sets up a simple identity VM mapping and does some simple tests to
     validate expected behavior using the PMU counters: 
        1. That the first data access to cached memory misses in 
           the both the d-cache and micro d-tlb (compulsory misses).
        2. That subeqeuent accesses hit;
        3. After invalidating the tlb,  everything misses again.
     You can easily mess with the test to measure other things.


------------------------------------------------------------------
#### Part 3: write tiny programs to show other counters.

This is a choose-your-own adventure:  look
through the counters and write the smallest programs you can to shows
off something they can measure.  You'll notice that some of them don't
do exactly what they claim.

Some easy ones:
  1. Count the write back drained.  AFAIK, both the "dsb" and "dmb"
     barriers increment.  We have function calls for these in libpi.
     You can also inline them:

        cp_asm_set_raw(cp15_dmb, p15, 0, c7, c10, 5)
        cp_asm_set_raw(cp15_dsb, p15, 0, c7, c10, 4)
        #define cp15_dmb cp15_dmb_set_raw
        #define cp15_dsb cp15_dsb_set_raw

  2. Data accesses (`data_access`): seems to work as expected.  I
     used `volatile` accesses to defeat the compiler.  

     Puzzle (that I don't know): why does the `wb_drain` (0x12) also
     increase?  this doesn't make sense to me.

  3. call-ret: the call (`call_cnt`) and return (`ret_hit`) counters
     for me only seem to work with the branch prediction turned on.
     And the return hits seem to max out at 3.  Let me know if you figure
     out anything more than this!

  4. Branch: branches include at least "branch and link" (function calls).
     The branch counters are unclear to me, tbh.

I'll add some suggestions (which you can ignore) as the lab goes.
If you see this sentence do a pull!

Ideally, you can use the counters to infer and validate things about the
the arm1176 chip --- cache size, associativity, prefetching instructions,
etc.

For cases where the code behaves weirdly, figure out what is going on,
ideally using some of the other counters.

------------------------------------------------------------------
### Extension: Use PMU counters in your profiler

For this, add some PMU counters to your profiler and show that 
they actually give sensible results.

------------------------------------------------------------------
### Extension: Do a hierarchical profiler

Seeing that a given instruction is run alot, is great, but if it's in
a routine called by many other routines you can't easily figure out how
to optimize.   A very useful tool is a hierarchical profiler that tracks
who called what, and when they did, how much it cost.  You can do a full
graph, or do 2 deep, 3 deep, etc.  Any of them will be extremely useful.

A great use of this is to apply it to your fat32 file system and use it
to speed it up.  Massive improvements are possible!

[single-step]: https://github.com/dddrrreee/cs140e-25win/tree/main/labs/9-debug-hw
[interrupts]: https://github.com/dddrrreee/cs140e-25win/tree/main/labs/4-interrupts
