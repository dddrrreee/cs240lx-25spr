## Memcheck trap II: the system.

Last time we did an ad hoc memory trapping hack so you could grab
all loads and stores using domain tricks and watchpoint faults.

This lab we'll:

  1. Clean last lab's code up and use it to build a simple little
     memory tracing system (`memtrace.c`) that can run different client
     checkers on memory operations.

  2. Write a simple checker (`checker-purify.c`) that runs on this
     system and flags whens a load or store references outside of 
     blocks of memory allocated using a slightly modified version
     of your checking allocator.

  3. Slightly tweak your ckalloc.c so that it works with (1) and (2).

Checkoff:
  - With your `memtrace.c` and `ckalloc.c` you pass the tests with
    `staff-purify-checker.o`.

  - With your own `purify-checker.c`: you flag the same errors in
    the purify tests though perhaps not with text identical error formats.

------------------------------------------------------------------------
### Part 1.  write `code/memtrace.c`

You'll wrap up your code from  last lab into a simple memory tracing
system.  The interface is in `code/memtrace.h`. It has two main actions:
(1) initialization (`memtrace_init`) and (2) turning trapping on
(`memtrace_trap_enable`) and off (`memtrace_trap_disable`).

The initialization routine:
```
    void memtrace_init( void *data,
        memtrace_fn_t pre,
        memtrace_fn_t post,
        unsigned trap_dom);
```

Takes:
  - `data`: a pointer to data that gets passed to client handlers
    `pre` and `post`.
  - `pre`: called when a memory instruction traps, before running the
     instruction.
  - `post`: called when a memory instruction traps, after running the
     instruction.
  - `trap_dom`: the domain id associated with all trapping memory.

At least one of `pre` and `post` should be defined.  For today, this 
routine calls the code to initialize the virtual memory system.
The handlers are called with a `memtrace.h:fault_ctx` structure
that takes provides:
  - `r`: a pointer to the current fault regs.
  - `pc`: the initial fault pc (note the fault regs pc value `r->regs[15]`
    will differ in post since that gets called after the instruction.
  - `addr`: the memory address of the fault.
  - `nbytes`: the size of the access.  NOTE: for right now we always pass 4, 
     but this is not correct.   A good extension is to make it not so stupid.
  - `load_p`: whether it was a load (`load_p=1`) or store (`load_p=0`).

All of this information is in your last lab (except nbytes).

Big picture:
  - For today, `memtrace_init` sets up virtual memory by calling
    `sbrk-trap.c:sbrk_init`.    This isn't how we'd to it for real since
    it makes things hard to compose, but keeps today more simple.

  - `sbrk_init` calls the same VM code as we used last lab
    (`vm_map_everything`).  `vm_map_everything` allocates a 1MB heap
    with its own private domain id (so we can easily trap accesses to it).

    `sbrk_init` also allocates a second 1MB used by its trivial 
    non-trapping heap allocator (`notrap_alloc`).  

    The only interesting thing about these two heaps is that there is
    a 1MB unmapped zone between them so overflows from the regular heap
    don't get into our non-trapping heap easily.

  - Extension: If you want to use shadow memory, the easiest thing is to
    map another 1MB (using `memmap-default.c:mb_map`) so you can add a
    constant offset to heap addresses to get their associated shadow.

    Alternatively you could cap the main heap size and devote an
    equivalant amount of memory from the non-trapping heap to it.

To understand the interface, the easiest thing is to look at the couple
of tests in `tests-memtrace`.

What is success:
  1. The few tests in `tests-memtrace` pass. 
  2. The tests `tests-purify` should pass when using `staff-ckalloc.o`
     and `staff-checker-purify.o`.

------------------------------------------------------------------------
### Part 2.  write `code/checker-purify.c` 

Now that you have memory tracing, you can write your memory checker.
On each trapping load and store, you will look up the trapped address
using your debug allocator code and flags illegal accesses.

One way to look at what we are doing: we can use traps to have your debug
allocator error checks always be on, rather than only occuring when the
client requests it.

A bit lower level:
  1. Register a `pre` `handler with `memtrace.c` (since we want the
     handler to run before the memory operation completes).

  2. On every load and store, look up the provided address using  your
     `ckalloc.c:ck_ptr_is_alloced` routine.  If this works, the access
     is legal: return.

  3. If the address is not legal, you should call the new routine
     `ck_get_containing_blk`.  It looks through the free and allocated
     lists for any block that has this address in either its header,
     redzones or data. You'll use this routine to give more precise
     errors: is the access before or after the block and by how many
     bytes?    There is a provided routine `ckalloc.h:ck_illegal_offset`
     that computes the byte offsets for you if you're lazy.

What is success:
  1. The tests `tests-purify` should still pass or give errors.
     You error messages should roughly match the out files: they should say
     if the illegal access was before or after the block and by how many
     bytes as well as whether the block was allocated or previously freed.

------------------------------------------------------------------------
### Part 3. make a modified `code/ckalloc.c` [10 min]

This last part is quick.  You should copy your `ckalloc.c` code into
this lab.  You'll have to make a couple of changes (sorry).

  1. Change it so that it calls `kmalloc` rather than `kr_malloc` by default, 
     unless a client provides their own malloc and free using a call to 
     `ckalloc_init`.  (We should actually have a structure capturing the
     heap context we want.  Sigh.):

            // ckalloc.c
            static alloc_t alloc_fn = kmalloc;
            static free_t free_fn = 0;
                
            void ckalloc_init(alloc_t allocfn, free_t freefn) {
                assert(allocfn);
                alloc_fn = allocfn;
                free_fn = freefn;
            }

  2. Write the new routine `ck_get_containing_blk` that you used above.
     It should walk through the allocated and free lists looking for
     a block that contains `addr`.  NOTE: by "contains" we mean addr
     can point into the header, the data, or either redzone.  Note,
     this is different behavior from `ck_ptr_in_block`.

With these changes, the original staff code (`staff-purify-checker.o`)
should pass all the tests as is.

You now have a simple, clean, kernel level memory corruption checker.
Very, very few people can say the same.

------------------------------------------------------------------------
#### Extension: compute the actual number of bytes accessed.

The biggest limit of the current code is that it doesn't correctly
compute how many bytes an instruction accesses.  For this you'll parse
the machine instruction and determine how many bytes it accesses.
You should write some test code that shows that you do this correctly.

(If you are blocked on this I do have a header for it.)

-------------------------------------------------------------------------------
#### Extension: replace a bunch of our `.o` files.

We use a bunch of code from old labs.  You should already have versions,
so can start dropping in yours instead of ours.

-------------------------------------------------------------------------------
#### Extension: add simple shadow to `check-purify.c`

Here you'll do a simple shadow memory.  We'll just allocate a single 4-byte word
to keep things easy.  There are three parts to this:

  1. For each byte of heap memory, we'll have a byte of shadow memory holding
     its state.  I put the shadow memory in the second half of the heap
     Create and map this during your own time initialization.

  2. In `purify_alloc`, turn checking off so the shadow memory can be
     written, mark its shadow memory as `ALLOCATED`, and then turn
     checking back off.

     How this is used: The exception handler will check if the memory
     being read or written to is `ALLOCATED` and give errors for
     everything else.  It needs to be a system call since eventually we
     will be protecting the shadow memory with its own domain id.

  3. In `purify_free` mark the state as `FREED`.

Measure how much things get sped up.  (For the slow test it should
be significant).

------------------------------------------------------------------------
#### Extension: volatile checker.

A very common, nasty problem in embedded is that the code uses pointers
to manipulate device memory, but either the programmer does not use
`volatile` correctly or the compiler has a bug.   

Device memory bugs are very nasty and also very easy to make since you
are doing stuff to memory that the compiler thinks is redundant.  For
example:
  - Multiple back-to-back writes to the same location, so the compiler
    believes it just needs to do the last write.  Examples: 
    both the UART and i2c fifo queue.
  - Multiple reads to the same location without an intervening
    write (so that the compiler believes it can remove them).  This
    came up when we did mailbox and UART "is there space" checks.

As a recent example, several people had device i2c bugs because they
were sloppy with not using `volatile`.

Using your memory tracing you can write a checker for this pretty
easily.  The key property we will exploit is that `volatile` acceses
are invariant across optimization levels.  The compiler cannot remove,
reorder, or add them.  

How:
  1. Modify the memtrace code so that it can do memory trapping on
     device memory.  This shouldn't require much work, but you will
     have to be careful for circularity problems.
  2. Run your device driver and log the device addresses read
     and written along with the values.
  3. As a sanity check: Re-run the device code against this log and
     for each read, return the value read, and for each write, check that
     the written value matches the log.  If the code is deterministic
     and your code doesn't have bugs, this replay should succeed no
     matter how many times you do it.
  4. Now recompile the code with different optimization levels ("-O0",
     "-O1", "-Ofast" etc) and rerun it against the log.  (You'll have to
     ship the log over with your binary.) If any read or write changes
     you know there is a bug.

------------------------------------------------------------------------
#### Extension: run code backwards.

This is based on a suggestion from Joseph Shetaye!

Your memtrace code makes it not-to-bad to run code
backwards.

Basic idea: 
  1. When you run forward append the registers and changed
     values in a log.  
  2. At the end, iterate over the log backwards restoring
     the registers and memory to their original values.

A bit more detail.


In the pre-handler:
  1. Record the registers.
  2. If the operation is a store, record the largest memory region
     the store could be to the log.

In the post handler:
  1. Record the registers.
  2. If the memory operation was a store, determine what addresses
     changed in the pre-snapshot.  When running backwards, you will 
     set these addresses (and no others!) to their pre-values.
  
To run backwards:
 1. run the log backwards.
 2. for each store, reset the memory to its original value.
 3. for each register, set it to its original value.  
 4. Before doing 2 and 3 make sure that the current register
    and memory values match the post-values you have in the log.
    If they do not match: either your code has a bug or there is some
    non-determinism you didn't anticipate. Or both!

For devices:  you'll have to special case status checks (for UART and
I2C "is there space") so that when you run backwards and write to the
device memory, it will work as expected.  


------------------------------------------------------------------------
#### Extensions.

Adding more extensions.  If you see this sentence do a pull.
