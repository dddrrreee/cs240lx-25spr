// test framework to check and time runtime inlining of GET32 and PUT32.
//
// at runtime we do binary rewriting to replace every call to 
//  - GET32_inline with an equivalent load instruction
//  - and PUT32_inline with an equiv store instruction.
//
// we do it in this kind of roundabout way to make debugging easier.
// after you get this working, you can do something more aggressive
// as an extension!
#include "rpi.h"
#include "cycle-count.h"

// prototypes: these are identical to get32/put32.

// *(unsigned *)addr
unsigned GET32_inline(unsigned addr);
unsigned get32_inline(const volatile void *addr);

// *(unsigned *)addr = v;
void PUT32_inline(unsigned addr, unsigned v);
void put32_inline(volatile void *addr, unsigned v);

static int inline_on_p = 0, inline_cnt = 0;

// turn off inlining if we are dying.
#define die(msg...) do { inline_on_p = 0; panic("DYING:" msg); } while(0)

// inline helper: called from runtime-inline-asm.S:GET32_inline
// <lr> holds the value of the lr register, which will be 4 
// bytes past the call instruction to GET32_inline
uint32_t GET32_inline_helper(uint32_t addr, uint32_t lr) {
    if(inline_on_p) {
        todo("smash the call instruction to just do: ldr r0, [r0]\n");
        uint32_t pc = lr - 4;

        inline_cnt++;

        // turn inlining off while we print to make debugging easier.
        inline_on_p = 0;
        output("GET: rewriting address=%x, inline count=%d\n", pc, inline_cnt);
        inline_on_p = 1;
    }

    // manually do the load and return the first time.
    return *(volatile uint32_t*)addr;
}

// go through and rewrite similar to GET32_inline -- you have to 
// modify <runtime-inline-asm.S> as well.
void PUT32_inline_helper(uint32_t addr, uint32_t val, uint32_t lr) {
    inline_on_p = 0;
    todo("implement this\n");
}

/********************************************************************
 * simple tests that check results.
 */

// this should get rerwitten
void test_get32_inline(unsigned n) {
    for(unsigned i = 0; i < n; i++) {
        uint32_t got = GET32_inline((uint32_t)&i);
        if(got != i) {
            inline_on_p = 0;
            panic("got %d, expected %d\n", got, i);
        }
    }
}

// this should not get rewritten initially.
void test_get32(unsigned n) {
    for(unsigned i = 0; i < n; i++) {
        uint32_t got = GET32((uint32_t)&i);
        if(got != i)
            panic("got %d, expected %d\n", got, i);
    }
}


// test using our runtime inline version.
void test_put32_inline(unsigned n) {
    uint32_t x;

    for(unsigned i = 0; i < n; i++) {
        PUT32_inline((uint32_t)&x, i);
        if(x != i)
            panic("got %d, expected %d\n", x, i);
    }
}

// test using regular put32: this won't get rewritten initially.
void test_put32(unsigned n) {
    uint32_t x;

    for(unsigned i = 0; i < n; i++) {
        PUT32((uint32_t)&x, i);
        if(x != i)
            panic("got %d, expected %d\n", x, i);
    }
}

/*************************************************************
 * versions without loops or checking: more sensitive to speedup
 */

// use our inline GET32: all of these calls should get inlined.
// a good check as an extension: after running this onece, check
// in GET32_inline that we never get called with an <lr> in this
// routine.
void test_get32_inline_10(void) {
    GET32_inline(0);
    GET32_inline(0);
    GET32_inline(0);
    GET32_inline(0);
    GET32_inline(0);

    GET32_inline(0);
    GET32_inline(0);
    GET32_inline(0);
    GET32_inline(0);
    GET32_inline(0);
}

// use the raw GET32: these won't get inlined, which
// gives us a comparison point.
void test_get32_10(void) {
    GET32(0);
    GET32(0);
    GET32(0);
    GET32(0);
    GET32(0);

    GET32(0);
    GET32(0);
    GET32(0);
    GET32(0);
    GET32(0);
}

// we time things a bunch of different ways.
void notmain(void) {
    assert(!inline_cnt);

    output("about to test get32 inlining\n");
    inline_on_p = 1;
    uint32_t t_inline = TIME_CYC(test_get32_inline(1));
    uint32_t t_inline_run10 = TIME_CYC(test_get32_inline(100));
    uint32_t t_run10 = TIME_CYC(test_get32(100));
    inline_on_p = 0;

    output("time to run w/ inlining overhead:   %d\n", t_inline);
    output("time to run 10 times inlined:       %d\n", t_inline_run10);
    output("time to run 10 times non-inlined:   %d\n", t_run10);
    output("total inline count=%d\n", inline_cnt);


    output("about to test get32 inlining without loop or checking\n");

    // test without a loop
    inline_on_p = 1;
    t_inline       = TIME_CYC(test_get32_inline_10());
    t_inline_run10 = TIME_CYC(test_get32_inline_10());
    t_run10        = TIME_CYC(test_get32_10());
    inline_on_p = 0;

    output("time to run w/ inlining overhead:   %d\n", t_inline);
    output("time to run 10 times inlined:       %d\n", t_inline_run10);
    output("time to run 10 times non-inlined:   %d\n", t_run10);
    output("total inline count=%d\n", inline_cnt);

    todo("smash the original get32: should get same speedup\n");
    
    // smash the GET32 code to call the GET32_inline_helper, identically
    // how GET32_inline does.  you can' just copy the code since the branch
    // is relative to the destination.
    // 
    //      note: 0xe1a0100e  =  mov r1, lr
    uint32_t *get_pc = (void *)GET32;
    uint32_t orig_val0 = get_pc[0];
    uint32_t orig_val1 = get_pc[1];

    // 1. rewrite the GET32 code to call GET32_inline.
    // 2. this will result in all caller sites to GET32 getting
    //    inlined as well.
    // 3. thus: when it runs below, should have the same speedup
    todo("assign the new instructions to get_pc[0] and get_pc[1]\n");

    output("after rewriting get32!\n");
    output("    inst[0] = %x\n", get_pc[0]);
    output("    inst[1] = %x\n", get_pc[1]);
    inline_on_p = 1;

    // this test should now have inline overhead.
    t_inline       = TIME_CYC(test_get32_10());
    // this is our original: should have the same speedup.
    t_inline_run10 = TIME_CYC(test_get32_inline_10());
    // after inlining this should be same as the GET32_inline overhead.
    t_run10        = TIME_CYC(test_get32_10());
    inline_on_p = 0;

    output("time to run w/ inlining overhead:   %d\n", t_inline);
    output("time to run 10 times inlined:       %d\n", t_inline_run10);
    output("time to run 10 times non-inlined:   %d\n", t_run10);
    output("total inline count=%d\n", inline_cnt);

    todo("now implement the same thing for PUT32\n");
}
