## Lab: dynamic code generation

Since we do systems, of course we will build a tool to generate
instruction encodings rather than do by hand.

We already saw a trivial example of reverse engineering instruction
encodings in the first machine code
lab:
  - [2-dynamic-code-gen/prelab-code-pi/5-derive-add.c](../2-dynamic-code-gen/prelab-code-pi/5-derive-add.c)

Today we will generalize this a bit more.  This approach is based
on the papers "Reverse engineering instruction encodings"
I wrote with Wilson Hsieh and Godmar Back way back in the day:

  - [2-dynamic-code-gen/docs/derive-short.pdf](../2-dynamic-code-gen/docs/derive-short.pdf)
  - [2-dynamic-code-gen/docs/derive-usenix.pdf](../2-dynamic-code-gen/docs/derive-usenix.pdf)

----------------------------------------------------------------------------
### Part 1: reverse engineer instructions (`code/derive.c`)

The code you'll hack on today is in `code/`.  Feel free to refactor it
to make it more clean and elegant.    I ran out of time to iterate over
it, unfortunately.

The key files:
  - `check-encodings.c` this has the code you'll write and modify
    for today's lab.  Just start in `main` and build anything it
    needs.

  - `armv6-insts.h`: this has the `enum` and functions needed to encode
    instructions.  You'll build this out.

  - `code-gen.[ch]`: this has the runtime system needed to generate code.
    You'll build this out next time.

If you look at `main` you'll see five parts to build --- each is pretty
simple, but it gives you a feel for how the more general tricks are played:

  1. Write the code (`insts_emit`) to emit strings of assembly 
     and read in machine code.  You'll call out to the ARM cross compiler
     linker etc on your system.  `make.sh` has some examples. One way to get
     the encoding for a particular instruction is to, for example, put that
     instruction into `test.s` and then run
     ```
     arm-none-eabi-as test.s -o temp1 && arm-none-eabi-objcopy -O binary temp1 temp2
     ```
     and now a file named `temp2` will contain the encoding

     NOTE: I emitted special sentinals at the beginning
     and end of the emitted code so I could find the emitted
     instructions reliably.

  2. Use (1) to implement a simple cross-checker that takes a
     machine code value and cross checks it against what is produced
     by the assembler.
  3. Implement a single encoder `arm_add` and use (2) to cross-check it.
  4. Finish building out the code in `derive_op_rrr` to reverse engineer
     three register operations.  It shows how to determine the encoding 
     for `dst` --- you will have to do `src1` and `src2`.  You should
     pull your generated function back in and cross check it.
  5. Do: load, stores, some ALU and jump instructions.

Congratulations, you have the `hello world` version of a bunch of neat
tricks.  We will build these out more next lab and use them.

----------------------------------------------------------------------------
### Part 2: use to implement `2-dynamic-code-gen`

For the first lab machine code lab, you hand-wrote the encodings.
Use your derive code to automatically generate the encoders.

You probably want to check your code using something like:

    check_one_inst("b label; label: ", arm_b(0,4));
    check_one_inst("bl label; label: ", arm_bl(0,4));
    check_one_inst("label: b label; ", arm_b(0,0));
    check_one_inst("label: bl label; ", arm_bl(0,0));
    check_one_inst("ldr r0, [pc,#0]", arm_ldr(arm_r0, arm_r15, 0));
    check_one_inst("ldr r0, [pc,#256]", arm_ldr(arm_r0, arm_r15, 256));
    check_one_inst("ldr r0, [pc,#-256]", arm_ldr(arm_r0, arm_r15, -256));

Where my routines are:

    // <src_addr> is where the call is and <target_addr> is where you want
    // to go.
    inline uint32_t arm_b(uint32_t src_addr, uint32_t target_addr);
    inline uint32_t arm_bl(uint32_t src_addr, uint32_t target_addr);
----------------------------------------------------------------------------
#### Extension: write a simple linker

When we emit branches, we often always know where the branch target will
be in memory.  This means you'll need to make a note that the branch
uses a label, and when you know where that label resides, go back
and patch the code.

  1. use this to do if-statements.
  2. then do loops.
  3. then do recursive functions
  4. then do functions you haven't emit yet.

The same principles here are how the linker takes all your .o's and 
links them together into an executable.

----------------------------------------------------------------------------
#### Extension: reverse engineer for your laptop

Instead of just reverse engineering for the pi, you can do the same for
your laptop.  Which might actually be more useful over the course
of your life.  In any case: The workflow is the same, only the
names of the commands change (you'll use your native `gcc` and
`objdump` rather than the cross compilation ones for the ARM).

Note, that some modern OSes prevent running dynamically code as an attempt
to make the security exploits more difficult.  If you're on linux the
code in `2-dynamic-code-gen/prelab-code-unix` shows how defeat these
measures by compiling as follows:

	    gcc -O2 -z execstack dcg.c -o dcg

Macos will likely need a different approach.

I would first get the dcg example to work on your laptop and then start
deriving.  You'll have to reverse engineer how to pass parameters.  You
can do this by writing different C routines and looking at how they
get compiled.  The 140E note discusses this:
[https://github.com/dddrrreee/cs140e-25win/tree/main/notes/using-gcc-for-asm](https://github.com/dddrrreee/cs140e-25win/tree/main/notes/using-gcc-for-asm).

----------------------------------------------------------------------------
#### Extension: write a more general deriver

The current approach hard-codes a ton of things that should be variables.
It's an interesting puzzle to make something more general where
you can:
  1. specify the instruction format expected by the compiler using
     special variable names for different types of operatands (registers
     8-bit immediates, 12 bit immediates, privileged registers etc).
  2. have the deriver automatically solve for instructions that
     take 0, 1, 2, 3 operands where the operands can be registers,
     immediates,  labels.
  3. Automatically generate routines (or macros) to encode them.
  4. Automatically generate routines (or macros) to decode them).

The two derive papers might help.

----------------------------------------------------------------------------
#### Extension: making a `curry` routine

One big drawback of C is it's poor support for context and generic arguments.

For example, if we want to pass a routine `fn` to an `apply` routine to 
run on each entry in some data structure:
  1. Any internal state `fn` needs must be explicitly passed.  
  2. Further, unless we want to write an `apply` for each time, we have
     to do some gross hack like passing a `void \*` (see: `qsort` or
     `bsearch`).

So, for example even something as simple as counting the number of
entries less than some threshold becomes a mess:

        struct ctx {
            int thres;  // threshold value;
            int sum;    // number of entries < thres
        };

        // count the number of entries < val
        void smaller_than(void *ctx, const void *elem) {
            struct ctx *c = ctx;
            int e = *(int *)elem;

            if(e < ctx->thres)
                ctx->thres++;
        }


        typedef void (*apply_fn)(void *ctx,  const void *elem);

        // apply <fn> to linked list <l> passing <ctx> in each time.
        void apply(linked_list *l, void *ctx, apply_fn fn);

        ...

        // count the number of entries in <ll> less than <threshold
        int less_than(linked_list *ll, int threshold) {
            struct ctx c = { .thres = 10 };
            apply(ll, &c, smaller_than);
            return c.sum;
        }
        

This is gross.

Intuition: if we generate code at runtime, we can absorb each argument
into a new function pointer.  This means the type disppears.  Which 
makes it all more generic.


To keep it simple:

 1. allocate memory for code and to store the argument.
 2. generate code to load the argument and call the original
    routine.

            int foo(int x);
            int print_msg(const char *msg) {
                printk("%s\n", msg);
            }

            typedef (*curry_fn)(void);
        
            curry_fn curry(const char *type, ...) {
                p = alloc_mem();
                p[0] = arg;
                code = &p[1];

                va_list args;
                va_start(args, fmt);
                switch(*type) {
                case 'p':
                        code[0] = load_uimm32(va_arg(args, void *);
                        break;
        
                case 'i':
                        code[0] = load_uimm32(va_arg(args, int);
                        break;
                default:
                        panic("bad type: <%s>\n", type);
                }
                code[1] = gen_jal(fp);
                code[2] = gen_bx(reg_lr);
                return &code[0];
            }


            curry_fn foo5 = curry("i", foo, 5);
            curry_fn hello = curry("i", bar, 5);
