// engler,cs140e: we need a test RNG, and it's important that everyone gets the
// same answers (otherwise we can't compare code outputs).   
//
// removed a bunch of stuff we don't need and prefixed with pi_*.  anyone not in 
// the class: just use the original.
//
// test:
//      gcc -O -Wall -DTEST pi-random_r.c

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * This is derived from the Berkeley source:
 *	@(#)random.c	5.5 (Berkeley) 7/6/88
 * It was reworked for the GNU C Library by Roland McGrath.
 * Rewritten to be reentrant by Ulrich Drepper, 1995
 */

#include <stddef.h>
#include "random.h"


/* An improved random number generation package.  In addition to the standard
   rand()/srand() like interface, this package also has a special state info
   interface.  The initstate() routine is called with a seed, an array of
   bytes, and a count of how many bytes are being passed in; this array is
   then initialized to contain information for random number generation with
   that much state information.  Good sizes for the amount of state
   information are 32, 64, 128, and 256 bytes.  The state can be switched by
   calling the setstate() function with the same array as was initialized
   with initstate().  By default, the package runs with 128 bytes of state
   information and generates far better random numbers than a linear
   congruential generator.  If the amount of state information is less than
   32 bytes, a simple linear congruential R.N.G. is used.  Internally, the
   state information is treated as an array of longs; the zeroth element of
   the array is the type of R.N.G. being used (small integer); the remainder
   of the array is the state information for the R.N.G.  Thus, 32 bytes of
   state information will give 7 longs worth of state information, which will
   allow a degree seven polynomial.  (Note: The zeroth word of state
   information also has some other information stored in it; see setstate
   for details).  The random number generation technique is a linear feedback
   shift register approach, employing trinomials (since there are fewer terms
   to sum up that way).  In this approach, the least significant bit of all
   the numbers in the state table will act as a linear feedback shift register,
   and will have period 2^deg - 1 (where deg is the degree of the polynomial
   being used, assuming that the polynomial is irreducible and primitive).
   The higher order bits will have longer periods, since their values are
   also influenced by pseudo-random carries out of the lower bits.  The
   total period of the generator is approximately deg*(2**deg - 1); thus
   doubling the amount of state information has a vast influence on the
   period of the generator.  Note: The deg*(2**deg - 1) is an approximation
   only good for large deg, when the period of the shift register is the
   dominant factor.  With deg equal to seven, the period is actually much
   longer than the 7*(2**7 - 1) predicted by this formula.  */



/* For each of the currently supported random number generators, we have a
   break value on the amount of state information (you need at least this many
   bytes of state info to support this random number generator), a degree for
   the polynomial (actually a trinomial) that the R.N.G. is based on, and
   separation between the two lower order coefficients of the trinomial.  */

/* Linear congruential.  */
#define	TYPE_0		0
#define	BREAK_0		8
#define	DEG_0		0
#define	SEP_0		0

/* x**7 + x**3 + 1.  */
#define	TYPE_1		1
#define	BREAK_1		32
#define	DEG_1		7
#define	SEP_1		3

/* x**15 + x + 1.  */
#define	TYPE_2		2
#define	BREAK_2		64
#define	DEG_2		15
#define	SEP_2		1

/* x**31 + x**3 + 1.  */
#define	TYPE_3		3
#define	BREAK_3		128
#define	DEG_3		31
#define	SEP_3		3

/* x**63 + x + 1.  */
#define	TYPE_4		4
#define	BREAK_4		256
#define	DEG_4		63
#define	SEP_4		1


/* Array versions of the above information to make code run faster.
   Relies on fact that TYPE_i == i.  */

#define	MAX_TYPES	5	/* Max number of types above.  */

struct random_poly_info
{
    int seps[MAX_TYPES];
    int degrees[MAX_TYPES];
};

static const struct random_poly_info random_poly_info =
{
    { SEP_0, SEP_1, SEP_2, SEP_3, SEP_4 },
    { DEG_0, DEG_1, DEG_2, DEG_3, DEG_4 }
};




/* If we are using the trivial TYPE_0 R.N.G., just do the old linear
   congruential bit.  Otherwise, we do our fancy trinomial stuff, which is the
   same in all the other cases due to all the global variables that have been
   set up.  The basic operation is to add the number at the rear pointer into
   the one at the front pointer.  Then both pointers are advanced to the next
   location cyclically in the table.  The value returned is the sum generated,
   reduced to 31 bits by throwing away the "least random" low bit.
   Note: The code takes advantage of the fact that both the front and
   rear pointers can't wrap on the same call by not testing the rear
   pointer if the front one has wrapped.  Returns a 31-bit random number.  */
int random_r(struct random_data *buf, int32_t *result)
{
    int32_t *state;

    if (buf == NULL || result == NULL)
	goto fail;

    state = buf->state;

    if (buf->rand_type == TYPE_0)
    {
	int32_t val = state[0];
	val = ((state[0] * 1103515245) + 12345) & 0x7fffffff;
	state[0] = val;
	*result = val;
    }
    else
    {
	int32_t *fptr = buf->fptr;
	int32_t *rptr = buf->rptr;
	int32_t *end_ptr = buf->end_ptr;
	int32_t val;

	val = *fptr += *rptr;
	/* Chucking least random bit.  */
	*result = (val >> 1) & 0x7fffffff;
	++fptr;
	if (fptr >= end_ptr)
	{
	    fptr = state;
	    ++rptr;
	}
	else
	{
	    ++rptr;
	    if (rptr >= end_ptr)
		rptr = state;
	}
	buf->fptr = fptr;
	buf->rptr = rptr;
    }
    return 0;

fail:
    return -1;
}

/* Initialize the random number generator based on the given seed.  If the
   type is the trivial no-state-information type, just remember the seed.
   Otherwise, initializes state[] based on the given "seed" via a linear
   congruential generator.  Then, the pointers are set to known locations
   that are exactly rand_sep places apart.  Lastly, it cycles the state
   information a given number of times to get rid of any initial dependencies
   introduced by the L.C.R.N.G.  Note that the initialization of randtbl[]
   for default usage relies on values produced by this routine.  */
int srandom_r (unsigned int seed, struct random_data *buf) {
    int type;
    int32_t *state;
    long int i;
    long int word;
    int32_t *dst;
    int kc;

    if (buf == NULL)
	goto fail;
    type = buf->rand_type;
    if ((unsigned int) type >= MAX_TYPES)
	goto fail;

    state = buf->state;
    /* We must make sure the seed is not 0.  Take arbitrarily 1 in this case.  */
    if (seed == 0)
	seed = 1;
    state[0] = seed;
    if (type == TYPE_0)
	goto done;

    dst = state;
    word = seed;
    kc = buf->rand_deg;
    for (i = 1; i < kc; ++i)
    {
	/* This does:
	   state[i] = (16807 * state[i - 1]) % 2147483647;
	   but avoids overflowing 31 bits.  */
	long int hi = word / 127773;
	long int lo = word % 127773;
	word = 16807 * lo - 2836 * hi;
	if (word < 0)
	    word += 2147483647;
	*++dst = word;
    }

    buf->fptr = &state[buf->rand_sep];
    buf->rptr = &state[0];
    kc *= 10;
    while (--kc >= 0)
    {
	int32_t discard;
	(void) random_r (buf, &discard);
    }

done:
    return 0;

fail:
    return -1;
}

/* Initialize the state information in the given array of N bytes for
   future random number generation.  Based on the number of bytes we
   are given, and the break values for the different R.N.G.'s, we choose
   the best (largest) one we can and set things up for it.  srandom is
   then called to initialize the state information.  Note that on return
   from srandom, we set state[-1] to be the type multiplexed with the current
   value of the rear pointer; this is so successive calls to initstate won't
   lose this information and will be able to restart with setstate.
   Note: The first thing we do is save the current state, if any, just like
   setstate so that it doesn't matter when initstate is called.
   Returns a pointer to the old state.  */
int initstate_r (unsigned int seed, char *arg_state, size_t n, struct random_data *buf)
{
    int type;
    int degree;
    int separation;
    int32_t *state;

    if (buf == NULL)
	goto fail;

    if (n >= BREAK_3)
	type = n < BREAK_4 ? TYPE_3 : TYPE_4;
    else if (n < BREAK_1)
    {
	if (n < BREAK_0)
	{
	    goto fail;
	}
	type = TYPE_0;
    }
    else
	type = n < BREAK_2 ? TYPE_1 : TYPE_2;

    degree = random_poly_info.degrees[type];
    separation = random_poly_info.seps[type];

    buf->rand_type = type;
    buf->rand_sep = separation;
    buf->rand_deg = degree;
    state = &((int32_t *) arg_state)[1];	/* First location.  */
    /* Must set END_PTR before srandom.  */
    buf->end_ptr = &state[degree];

    buf->state = state;

    srandom_r (seed, buf);

    state[-1] = TYPE_0;
    if (type != TYPE_0)
	state[-1] = (buf->rptr - state) * MAX_TYPES + type;

    return 0;

fail:
    return -1;
}

// Manual implementation needed for HWs without division
// By Stuart Sul
int __aeabi_idivmod(int numerator, int denominator) {
    if (denominator == 0)
        return 0; // Undefined (TODO: panic)

    int neg = 0;
    if (numerator < 0) {
        numerator = -numerator;
        neg = !neg;
    }
    if (denominator < 0) {
        denominator = -denominator;
        neg = !neg;
    }

    // Unsigned division (long division method)
    unsigned int uquotient = 0;
    unsigned int unumerator = (unsigned int)numerator;
    unsigned int udenominator = (unsigned int)denominator;

    for (int i = 31; i >= 0; i--) {
        if ((unumerator >> i) >= udenominator) {
            unumerator -= (udenominator << i);
            uquotient += (1U << i);
        }
    }

    int quotient = (neg ? -(int)uquotient : (int)uquotient);
    int remainder = (int)unumerator;

    // ARM EABI expects remainder in r1
    asm volatile ("mov r1, %0" : : "r"(remainder) : "r1");

    return quotient;  // r0 = quotient
}

/* Restore the state from the given state array.
   Note: It is important that we also remember the locations of the pointers
   in the current state information, and restore the locations of the pointers
   from the old state information.  This is done by multiplexing the pointer
   location into the zeroth word of the state information. Note that due
   to the order in which things are done, it is OK to call setstate with the
   same state as the current state
   Returns a pointer to the old state information.  */
int setstate_r (char *arg_state, struct random_data *buf)
{
    int32_t *new_state = 1 + (int32_t *) arg_state;
    int type;
    int old_type;
    int32_t *old_state;
    int degree;
    int separation;

    if (arg_state == NULL || buf == NULL)
	goto fail;

    old_type = buf->rand_type;
    old_state = buf->state;
    if (old_type == TYPE_0)
	old_state[-1] = TYPE_0;
    else
	old_state[-1] = (MAX_TYPES * (buf->rptr - old_state)) + old_type;

    type = new_state[-1] % MAX_TYPES;
    if (type < TYPE_0 || type > TYPE_4)
	goto fail;

    buf->rand_deg = degree = random_poly_info.degrees[type];
    buf->rand_sep = separation = random_poly_info.seps[type];
    buf->rand_type = type;

    if (type != TYPE_0)
    {
	int rear = new_state[-1] / MAX_TYPES;
	buf->rptr = &new_state[rear];
	buf->fptr = &new_state[(rear + separation) % degree];
    }
    buf->state = new_state;
    /* Set end_ptr too.  */
    buf->end_ptr = &new_state[degree];

    return 0;

fail:
    return -1;
}


#ifdef TEST

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#define STATESIZE 128

// we are not guaranteed that the libc implement of random is the same.
// we provide our own.

static void print_random(struct random_data *r, unsigned n) {
    for(int i = 0; i < n; i++) {
        int x; 
        if(random_r(r, &x))
            assert(0);
        printf("x=%x, %d\n", x,x%1024);
    }
}

int main() {
    int seed = 0;
    char statebuf[STATESIZE];
    struct random_data r;

    memset(&r, 0, sizeof r);
    if(initstate_r(seed, statebuf, STATESIZE, &r))
        assert(0);
    if(srandom_r(seed, &r))
        assert(0);

    // should be same.
    print_random(&r, 8);
    srandom_r(seed, &r);
    print_random(&r, 8);

    return 0;
}
#endif
