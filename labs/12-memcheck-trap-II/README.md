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
### Extensions.

There are tons of extensions.  If you see this sentence do a pull.
