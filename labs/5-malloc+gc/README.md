## Lab: Leak detection, garbage collection

-------------------------------------------------------------------------
### tl;dr: BUGS and clarifications.
  
  - BUG: you do not need to implement `dump_regs` --- we used to use this
    until 10 minutes before class when I realized the approach was
    fundamentally broken.  Unfortunately there are still a couple old
    comments that mention it.  You just need to do the two trampolines in
    `gc-asm.S` (`ck_gc` and `ck_find_leaks`).

  - If you get an error "not handling that" in `staff-ck-gc.c` just
    have your kr_malloc allocate more memory on each call (for
    the tests 64k should be enough, but can do 512k).

-------------------------------------------------------------------------
In this lab you'll:
  1. Adapt the K&R `malloc/free` implementation to the rpi so we have
     a correct (albeit slow) `free` implementation.

  2. Write a simple leak detector detector) that detects when
     an allocated block *must* have been lost.  It works by enumerating all
     memory locations that could contain a pointer and marking the blocks
     the point to.  It then sweeps through all allocated blocks, flagging any
     that were not marked: since no pointer anywhere in the program points to
     them, they must be lost.  

  3. Extend this to be a conservative garbage collector: a tool that
     finds allocated blocks that no live pointers references and then
     frees it.

There's a bunch of useful background reading in [PRELAB](PRELAB.md).

---------------------------------------------------------------------------
### Part 0: adapt K&R malloc/free from `docs/kr-malloc.pdf`.

You'll import the K&R `malloc` into libpi:

  1. Copy and paste and adapt the code in `kr-malloc.pdf` so that it 
     runs on Unix.
  2. Check that the tests pass (these are weak: write some more if you can!).
  3. Modify it so it will work on the pi.  In particular: implement `sbrk()`
     on the pi by calling `kmalloc`.

---------------------------------------------------------------------------
### Part 1: write a `ckalloc` wrapper on `kr_malloc`: 

tl;dr:
  - Code is in: `code-leak+gc/ckalloc.c`
  - There are three tests:

        PROGS := tests/ck-test1-ok.c
        PROGS += tests/ck-test2-ok.c
        PROGS += tests/ck-test3-ok.c


For GC and leak detection we need a way to determine:
  1. The set of allegedly allocated blocks (so we know what to free
     if it's not reachable).
  2. That allocated block a pointer `p` points to (so that we can 
     mark that block as reachable).

For this part you'll do simple veneer on `kr_malloc` to:
  1. Put all allocated blocks on a list.
  2. Add a header that lets us determine block size and other information.

We are going to do it in a stupid way so that we're more sure it's
correct.  A great extension is doing it more intelligently.

What we are doing is:
  1. For each allocation call to `ckalloc`: allocate enough to prepend
     a `ckalloc.h:hdr_t` to the block and add that block to a list of
     allocated pointers.

  2. On each deallocation call to `ckfree`: check that the block is
     allocated, remove it from the allocated list, an call `kr_free`
     on it.

What to write:
  1. The interface is in `code-leak+gc/ckalloc.h` the starter code in 
     `ckalloc.c`.
  2. There are some simple accessors you should implement.

Here's a dumb linked list removal to save you time if that's an issue:

    static void list_remove(hdr_t **l, hdr_t *h) {
        assert(l);
        hdr_t *prev = *l;
     
        if(prev == h) {
            *l = h->next;
            return;
        }

        hdr_t *p;
        while((p = ck_next_hdr(prev))) {
            if(p == h) {
                prev->next = p->next;
                return;
            }
            prev = p;
        }
        panic("did not find %p in list\n", h);
    }

Invariants:
  1. `ckalloc`: before you add it to the allocated list `ck_ptr_is_alloced`
     should fail, after you add it `ck_ptr_is_alloced` should succeed.
  2. `ckfree`: before you free it, `ck_ptr_is_alloced(ptr)` should
     return 1; after you remove it from the allocated list, it should
     return 0.  The state should be `ALLOCED`.

---------------------------------------------------------------------------
### Part 2: leak detection `ck-gc.c` and `gc-asm.S`

We will build a simple leak detector based on the approaches described in
the Purify paper and Boehm's GC paper.  The tool will attempt to detect
blocks of memory that *must* be lost because there are no pointers to
them anywhere in the program.

The leak detector works by using simple mark-and-sweep algorithm to
detect all blocks no longer reachable in the program and then, instead
of reclaiming them, emits error messages that they are unreachable.
(You can try running `valgrind` with leak detection on to see what this
looks like in practice.)

The classic mark-and-sweep algorithm:

  1. Scan through the set of objects pointed-to by the "root set" ---
     pointers contained in the data segment (bss and data: 
     see `libpi/include/memmap.h`), stack locations, and
     registers.

  2. When you see an object for the first time you mark it (the
     "mark" in "mark-and-sweep") and, also, recursively scan all
     pointers it contains.  If an object has already been marked,
     skip it.

  3. When steps (1) and (2) finish, scan all blocks in the heap: if a
     block has not been marked, you know it cannot be legally reached
     by the program and so must be garbage (in our case: lost).

At risk of overkill, but in an attempt to reduce confusion, here's
simple pseudo-code:

    mark_and_sweep()
       # all other memory locations must be reachable 
       # from these starting points.
       root_set = { valid pointers in static data }
                U { valid pointers in registers }
                U { valid pointers on the stack };
        mark(root_set);
        sweep();
    }

    mark(pointer_set) {
       foreach p in pointer_set {
            if !marked(p) {
                marked(p) = 1;
                mark({ valid pointers in *p })
            }
        }
    }
    sweep() {
        foreach blk in heap {
            # reachable
            if(marked(blk))
                marked(blk) = 0;
            # not reachable
            else
                free(blk);
        }
    }


This trivial graph traversal algorithm would be on the easier end of CS107
assignments.  The thing that makes it tricky for C is that it requires
being able to grab a random chunk of memory and enumerate all the legal
pointers it contains.  Of course, for C we have no way of knowing the
types of each byte and, thus, we can't tell which 32-bit words on the
pi contain pointers or not.    If we miss even a single pointer we can
potentially think a block is free  when it is not.  Such omissions will
cause false error reports (false positives) for our leak detector and,
worse, very nasty use-after-free errors for the garbage collector if we
reclaim memory that is still being used.

Fortunately, we can use a hack from Boehm:

  - Treat all any pointer-sized and pointer-aligned word (on the pi:
    4-byte aligned and 4-byte sized) as a potential pointer and its
    contained value as an address.  If this potential-address falls
    within the address range of any allocated block, we consider the
    block potentially reachable, and mark it.

For example, assume:
  1. An aligned 4-byte word `w` contains the value `0x10000f8`.
  2. Our allocator has allocated a block `b` that starts at address
     `0x1000000` and ends at address `0x1000100`.

Therefore, we will consider `w` potentially a valid pointer and mark `b`.

Of course, if `w` was just a random integer, this means we falsely
marked `b`.  However, for our purposes this is a "conservative" mistake
in that while it will cause us to not reclaim unused memory (which
is inaccurate) it does not cause us to violate harm program correctness.

In contrast, if we did not simply mark and reclaim memory but instead also
compacted it (moved it) to reduce fragmentation, then falsely treating
a location as a pointer when it was not would violate correctness,
since after compacting we update pointers to refer to the new location.
For this reason, without additional tricks, we can't do a compacting
collector.

#### Implementation notes: `gc-asm.S`

NOTE: 
  - you can initially use staff.

Since our mark-and-sweep has no integration with the compiler
it is extremely vulnerable to the old values of points that
the compiler leaves laying around after use.  To attempt to 
minimize this we want to be as accurate as possible about
ignoring garbage registers and stack entries before doing the sweep.
In particular:
  - If we grab the stack pointer inside the GC code, the compiler
    will have already decremented it, including stack entries we
    don't need.
  - We only want to consider the callee-saved registers: the 
    caller only hold garbage.

After a couple of years of false starts, we do this using two 
trampolines (that you will write):
  - `gc-asm.S:ck_gc`: push all the callee saved registers and then
     pass the resultant stack pointer as the first argument to 
     `ck_gc_fn`.

     When it returns make sure you pop everything you saved.  You will
     want to save and restore the link register `lr` but not consider
     it when doing gc.

  - `gc-asm.S:ck_find_leaks`: do the same, except pass the stack
    pointe ras the second argument to `ck_find_leaks_fn`.

#### Implementation notes for `ck-gc.c`

For the pi, we implement the pseudo-code above as follows:

  1. `ckalloc.h`: Each block header has a `mark` bit and counters
     recording how many pointers are to the start of the block (`refs_start`)
     and to the middle of the block (`refs_middle`).  As the Purify
     paper describes, blocks that only have a single pointer to the
     middle are more likely actual leaks where a coincidental integer
     value has falsely caused the block to be marked.

     Most of our code will be devoted to computing correct values for
     these three fields.

  2. `mark_all` (in `ck-gc.c`): invokes the `mark` routine on each
     range of possible root pointers: (1) the initialized and
     uninitialized data segments, (2) registers, and (3) the stack.
     This is a complete set of all locations a root pointer could be in.
     All other pointers must be reachable by a pointer traversal from
     these locations.

     We compute the address range of the two data segments using variables
     inserted into the binary using the linker script `libpi/memmap`.

  3. `mark`: iterates over each 32-bit word in a given range (inclusively)
     and uses the routine `is_ptr` to resolve it to its associated header
     (if any).  For legal pointers: It increments the appropriate block
     reference counter, if the block was not already marked, marks it
     and recursively calls `mark` on the block's allocated memory (so:
     not the header, not the leftover part, not the redzones).

  4. `sweep_leak`: iterates over each header and warns if a block in state
     `ALLOCATED` has not been marked.  (In practice: each time you find
     a leaked block, it's probably useful to set a bit in its header
     indicating you've already reported this erro, so any subsequent
     leak runs do not obscure additional errors by repeatedly reporting
     old ones.)

  5. The tests are in `code-leak+gc/tests` --- you should just work through
     them one at a time like last time.  The tests are pretty simplistic,
     so please also write some of your own!  Any test that does not
     call `ck_gc()` (test5) should pass.

Note, that its very easy to write tests that behave unexpectedly:
   1. Because of gcc optimization, its easy to have pointers to blocks get 
      optimized out if you don't use them.  I typically pass them to `printk`
      to make sure they stay where they are.
   2. Because gcc will not zero-out dead locations, there can remaining
      pointers to a block even after the variable pointing to it dies
      (e.g., you leave its containing scope).  This is true even if you
      "overwrite" the pointer!  For example consider the trivial code:

            int *p = ckalloc(4);
            p = 0;
            ck_should_leak();  // might fail!

      If there are no further uses (reads) of `p`, `gcc` will likely elide
      the assignment `p = 0`, potentially leaving the original value of
      `p` in a register or stack location, which will cause your detector
      to falsely think there is still a pointer to the value returned by
      `ckalloc`.  As with previous labs, you will likely have to look
      at the `.list` files at times to see what is going on.

Some code mistakes:
  1. It's easy to not scan far enough and miss the last word in a range.
     It's also easy to go too far.  Or start scanning a number of bytes
     rather than a given number of words.  The problem with these errors
     is that they will not necessarily change your detectors behavior
     on a given test!

  2. To minimize potential errors, we will initialize the `mark` and
     `ref` fields each time (you should rewrite the code later so it
     doesn't need this).  This will break your checksum unless you
     are careful: recompute it, or just don't checksum these fields.
     Kinda gross, sorry.

  3. By far the most common mistake:
     Freeing a block and then getting its next pointer in the sweep
     routine.  Everyone did this (me, too).

---------------------------------------------------------------------------
### Part 2: Garbage collection (30 minutes)

Currently just one test:
  - `tests/gc-test5-gc.c`

We can fairly easily flip our leak detector into a leap corrector
("garbage collector") that will free unreachable blocks.

To keep this part simple, we only flip the state of the block from
`ALLOCATED` to `FREED` and keep a count of the number of allocated bytes
that we free.
In practice, you'd do something more active.  Two common approaches:
  1. For low-level allocators: Coalesce adjacent freed blocks to reduce 
     fragmentation and so that your allocator can satisfy a larger range
     of allocation requests.  Add free blocks to a free list.

  2. For a higher-level allocator built on a low level one: rather than 
     spend cycles coalescing blocks, assume that "past = future" and that
     if code asked for this size previously it will do so again:  add freed
     blocks below a certain size to a size-segregated free list.  This makes
     both free and allocate fast.

If there is time I would suggest doing one of these.  It won't take much
code, but can be a bit tricky because you have to make sure you update
all necessary internal allocation pointers.

For this part: 
  1. Finish implementing `sweep_free` which will look alot like `sweep_leak` without
     some of the error checking.

----------------------------------------------------------------------------
### Simple useful extensions

Write more tests!  Make sure they give the same results with -O0, -O1, -O2.
TY TY.

----------------------------------------------------------------------------
### General limitations.

Both the leak detector and garbage collector do not accurately handle if:
  1. You write a pointer out to the SD card and then read it back later.
     Marking will not see such references.

  2. Marking will also miss references if you write addresses to DMA
     hardware on the pi since it does not traverse this memory.

  3. We assume pointers must be 4-byte aligned.  If they were allowed to be 
     only byte-aligned we would have to scan all memory using a sliding window 
     starting at the beginning of each memory region, then 
     starting at 1 byte in, then 2, then 3.  We would get many false references.
  4. You do funny things with the pointer.  One example:
     using the upper, unused bits of the address to hold extra
     type information.  Such pointers will likely look like random
     integers and will be ignored.  Another example: saving space by
     xor'ing pointers together (and old Microsoft interview question!).

The first two problems can typically be handled by allowing the client
to register pointers with the mark-and-sweep system. The last problem
is hard.

Some expedient limits we add just because we are doing a lab:
  1.  We assume you are not using threads and just have a known, single stack.
      (This limit is not bad to lift.)
  2. We are not checking self-modifying or dynamically generated code that 
     encodes pointers.

----------------------------------------------------------------------------
### Extension: gc your fat32 file system

In 140e we did fat32 in a very dumb way, with massive memory leaks at
basically every point.  You have a great way to fix that.  Plug in your
gc and run your fat32 for a long time, tracking that every allocation
gets freed.  This is a great stress test for your gc and should vastly
improve your fat32 performance / uptime.

----------------------------------------------------------------------------
### Extensions

In order to fit these topics in a short lab we are wildly over-simplifying
how to do memory allocation. Some obvious things to change:

  - Do a faster allocator object-based allocator or a an arena allocator (see the [PRELAB.md](./PRELAB.md))
  - Shrink down the space and speed overhead.
  - Decouple the debugging aspect from the garbage collector itself, so 
     you can use it during production on a raw, non-debugging allocator.
  - We have an incredibly slow approach to allocation and deallocation.  See
     the readings!
  - Don't just use a single byte sentinel.  Do something more sensitive.
  - Don't have just a single contiguous allocation for the heap.  Often
     you'll stitch a heap together from disjoint pages.
  - Many many many other things. 

With all that said, the biggest problem we have with code is correctness.
And the biggest source of complexity in memory allocation is introduced by
`free()`.  If you only need a bounded amount of memory, cut out `free()`
entirely, use the obvious 20-line allocator implementation (easy to make
correct!) and use `reboot` as a massive `free()`. I cannot emphasize
this enough!  Premature optimization is a virus on correctness, and
space optimization is even worse than speed.

But: if you need `free()`, you need it.  And the use of arenas and object
allocators are a reasonable way to get much of the benefits of a general
free without much of the complexity.
