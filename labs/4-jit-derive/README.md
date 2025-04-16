## Lab: dynamic code generation

***Make sure you read the [PRELAB](./PRELAB.md) for context.***

Generating machine code is fun, kind of sleazy, and most people have
no idea how to do it.  So it makes sense as a lab for our class.

As discussed in the `PRELAB` its used for many things, from the original
VMware, machine simulators (closely related!), JIT compilers you may
well use everyday, and even nested functions in gcc.

A second nice thing about generating machine code is that it really
gives you a feel for how the machine actually operates.  When you do
today's lab you will realize that --- surprisingly --- assembly code is
actually high-level in many ways(!) since the assembler and linker do many
tricky things for you.  (E.g., linking jumps to their destinations.)
You will implement a bunch of these things (next lab).

The theory behind today's lab is that we'll do concrete examples of 
a bunch of different things so that it removes a bunch of the mystery.
Subsequent lab(s?) will go more in depth, where you can build out the things
you do today.

As a quick review, you should look at
`1-dynamic-code-gen/prelab-code-pi/5-derive-add.c` to see a simple
example of how to solve for an instruction encoding.

----------------------------------------------------------------------------
### Part 1: reverse engineer instructions

The code you'll hack on today is in `code/`.  Feel free to
refactor it to make it more clean and elegant.    I ran out of time to
iterate over it, unfortunately.
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
### Part 2: use to implement `1-dynamic-code-gen`

For the first lab, you hand-wrote the encodings.  Use your 
derive code to automatically generate the encoders.

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
