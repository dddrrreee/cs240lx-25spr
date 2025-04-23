## Building a redzone allocator.

Memory corruption bugs suck.  While some people use Rust, we're going
to try to make C less bad by attempting to check every load and store
for safety.

The next few labs will build a series of increasingly aggressive memory
corruption checkers.  Memory errors were a big source of cs140e bugs.
After the sequence of labs you should have tools that can detect many
(most?  all?) of the ones you hit.

Today's lab is a light one so that people that need to can finish
the previous.  There are a ton of possible extensions for anyone 
that finishes early.

We will build a debugging `malloc` that detects when a client writes
beyond (or before) an allocated block.  It works by embedding each
allocated block between two "redzones" (one immediately above it, one
immediately below it) set to known values and then periodically checks if
any redzone has been corrupted --- for example you can check if a block's
redzones have corrupt values when you `free` the block, or check the
entire heap at some regular interval (e.g., during garbage collection).

While the approach is a good first defense against bugs, it 
does have weaknesses:
  - it can't find when a read goes out of bounds (a load changes no
    redzone value).
  - it can't detect if an illegal write goes so far out of bounds that
    it misses the redzone and goes into another allocated block.
  - errors are only detected with a delay.

However, redzone allocators are simple, have few moving parts (no
interrupts, minor amount of code) while giving you a good way to find
a bunch of dumb errors easily.

The features we have will also be useful for the follow
on labs, which will focus on removing the listed weaknesses.


#### Checkoff

  1. You should pass all the tests.
  2. You should write a couple new tests, put your name
     in the comment, and add them to the ed group so everyone can use
     them.  Ideally these are tests we get wrong!  (this counts towards
     extension) Sadly this shouldn't be a huge challenge since I didn't
     test the code well.
  3. Do some kind of extension, ideally something useful that you
     would use yourself. 

------------------------------------------------------------------------------
### Checking equivalance

Writing tests is a pain.  A big genre of technical cleverness that I
feel most don't do well is squeezing more out of a sequence of tests.

For us, we missed an obvious clever trick:
  1. What the GC scans will change (sometimes significantly) change with 
     different optimization levels.
  2. For our tests what the GC *does* should not change.

So we can do the following:
  1. Do `make emit` at whatever optimization level you used.
  2. `make clean` and remake with a different optimization flag.
  3. `make check`: we should still pass.

I automated this by adding the following to the end of my `code/Makefile`:

    checkopt:
        @echo "going to check opt level: -O:"
        @make -s clean && make -s "OPT_LEVEL=-O" RUN=0
        @make check
     
        @echo "going to check opt level: -O2:"
        @make -s clean && make -s "OPT_LEVEL=-O2" RUN=0
        @make check
    
        @echo "going to check opt level: -O3:"
        @make -s clean && make -s "OPT_LEVEL=-O3" RUN=0
        @make check
    
        @echo "going to check opt level: -Ofast:"
        @make -s clean && make -s "OPT_LEVEL=-Ofast" RUN=0
        @make check

------------------------------------------------------------------------------
### Part 1: Redzone allocation.

In last lab, each block for GC has a header:

        +--------+--------+------------------+
        | header | data   | unused remainder |
        +--------+--------+------------------+

Where:
  - the header holds information about the allocation.
  - the `data` is the user data.
  - the unused remainder is extra bytes the underlying allocator allocated
    (for the K and R allocator we have, this should just be b/c of alignment).

Today's lab simply adds two additional chunks of bytes (the "redzones") 
where each memory block is laid out as follows:

        +--------+-----------------+------+------------------+-------------------+
        | header | redzone1        | data | redzone2          |  unused remainder | 
        +--------+-----------------+------+------------------+-------------------+

Simplifyin assumptions:
  - Both redzones are the same size (in the header: `REDZONE` bytes).
  - Both redzones are filled with the same byte pattern (in the header: `SENTINAL`).
  - When users call free() we mark the block as free but do not reuse it
    (to minimize chance reuse can cause us to miss errors).  This also
    simplifies the implementation.
  - When we free, the `data` bytes are set to the sentinal value so that we
    can detect some use-after-free read and writes.
  - To simplify address calculations we absorb `redzone1` into the header 
    structure: this prevents dynamic resizing, so you may want to change
    this after your code works.

The header now contains:
  - the first redzone `redzone1`.
  - a location for the free site (for reporting of use-after-free).

What you do:
  1. On allocation: allocate enough for the header, redzones and data.
     Set the redzones to the sentinal value.
  2. On free: check that the redzones are not corrupted, set the data to 
     the same sentinal value.
  3. On user heap check: walk through both freed and allocated blocks, 
     checking their redzones (and, for freed: their data).

We discuss these differences a bit more next.

#### What to do first.

tl;dr:
  1. Make sure `make check` passes.
  2. Copy your `ckalloc.c` over, switch the Makefile.
  3. Start working through the parts below --- you'll initially
     get some linker errors until you implement the missing routines.

#### Change: Add a freelist

Change `ckfree` to just add a block to a free list, rather than actually freeing
it with `kr_free`.

We need this list so that we can scan free blocks later (after a given free)
looking for errors. 



#### Change: add redzone checking

You should change your `ckalloc` implementation to:
  1. Allocate enough space for the second redzone (`ckalloc.h:REDZONE_NBYTES`)
     We put the first redzone at the end of the header so it's already
     included.
    
  2. Initialize both redzones to their special values `ckalloc.h:RESZONE_VAL`.
     You'll check for corruption by scanning both redzones for any byte that
     is not this value.  

  3. As a sanity check: after you do step 2, run your redzone check routine
     on the block to make sure it passes.

You should change your `ckfree` to:

  1. check the redzones immediately before you change the block's state.
  2. Set the data portion of the allocation to the redzone value so you
     can check that it isn't modified after.

To keep our errors consistent, when you find a redzone corruption
error print it out using the following format:

            ck_error(h, "%s block %u [%p] corrupted at offset %d\n",
                    h->state == FREED ? "Freed" : "Allocated",
                        h->block_id, addr, offset);

Where `offset` is a negative number if the corruption was in 
the initial redzone (before the data) and positive if its 
in the subsequent rezone (after the data).

When you find a use after free I did:

            trace("\tWrote block after free!\n");

Also, some of the error messages encode the routine name in them (e.g.,
`mem_check` or `check_list`) so you'll have to call your routine the
same thing or modify the test case.  Sorry about that --- we probably
should have stripped the internal name out.

#### Add `ck_heap_errors`

This walks over the free and allocated lists checking the blocks for
errors.  To make it easy, here's mine: you'll have to write `check_list`.


```
// integrity check the allocated / freed blocks in the heap
int ck_heap_errors(void) {
    trace("going to check heap\n");

    unsigned nblks = 0;

    unsigned nerrors = check_list(&nblks, alloc_list,0)
                     + check_list(&nblks, free_list, 1);

    if(nerrors)
        trace("checked %d blocks, detected %d errors\n", nblks, nerrors);
    else
        trace("SUCCESS: checked %d blocks, detected no errors\n", nblks);
    return nerrors;
}
```


------------------------------------------------------------------------------
### Extension: make error messages more useful

Curently we don't have backtraces, which can make it hard to figure out
where the actual problem is.

   1. Take your 140e backtrace code and modify `src-loc.h` to construct and
      store the backtrace.
   2. Recompile libpi to use the frame pointer, etc.
   3. Make sure things still work!

As a cooler hack: get `gcc` to emit "exception tables" that you can use
to look up where the backtrace is.  This elminates the need to have the
`fp` register.  You'll have to search around for blog posts on this.
I know you can do this trick but I'm embarassed to see that after an
hour looking around I didn't figure out how to get the headers. (You'll have
to do some kind of linker script hack.)

------------------------------------------------------------------------------
### Extension: make a fast arena allocator.  

Based on the arena paper, write a fast arena allocator that can allocate in
a few cycles and free all memory in roughly constant time.  You'll want
to measure overhead before tuning, and then try to cut out as much as you 
can.

------------------------------------------------------------------------------
### Make your current `ckalloc` faster

The easiest, common method to speed up `ckalloc` will be to have an 
array in front of it holding lists to past allocated blocks of different
sizes (e.g., blocks of size 8, 16, 24, 32 etc up to some fixed size).
Good things:
   - Free is fast: put on a list.
   - Allocation can be fast: fast a free, it's a list removal.

------------------------------------------------------------------------------
### Make scanning faster.

Currently our main bottleneck is the `is_ptr` routine which determines the
header a pointer correspeonds to.  You can use a version of "shadow memory"
to make this faster: 
  1. Make an array as large as your heap where each word as a pointer to that
     blocks header (if it exists).
  2. Lookup is now O(1).

You can do other tricks after that.

------------------------------------------------------------------------------
### Setup memory in a more realistic way

Right now we just grab random memory in the pi address space.  This 
is amateur hour.   What we should do (and will, but you can get a 
jump on it) is to:
  1. Use the mailbox interface to get the number of MB on the pi 
     available to the ARM (you did this in lab 1).
  2. Make a page allocator keeps track of which of the MB pages are
     free or allocated.
  3. Change the allocators to get their heap memory from this page allocator.

We will do this pretty soon, but it's much more informative if you figure
it out yourself.

------------------------------------------------------------------------------
### Extension: make adult allocators

Several adult moves that we should have done but didn't:

  1. Parameterize your `redzone` allocator so that it can
     be used to wrap up these allocations (or any allocation) in a
     clean way.  I'd suggest passing in allocation and free functoin
     pointers (the latter will be an empty routine).

  2. Redo `kmalloc` as an arena allocator (currently it uses a single
     large slab) where you pass in a context so you can have different
     allocators simultaneously.  This will be very useful later: for
     example, to allocate from special persistent memory in one, and
     from our usual heap in another.

  3. Similarly, parameterize `ckalloc` and `ckfree` so that they can take
     a structure describing the heap they manage so that you can have
     several simultaneous, independent heaps at the same time.  You can 
     have a global descriptor for a default heap (so that the signatures
     of `ckalloc` and `ckfree` don't change) but add additional methods that
     take a pointer to a heap structure of some kind.

  4. Adapt the redzone allocator to Unix so you can quickly drop it in 
     for your regular classes.  Ideally: run it on some old code and
     find a bug!
