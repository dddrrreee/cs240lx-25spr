#ifndef __MATRIX_LIB_H__
#define __MATRIX_LIB_H__

/*
 * Sparse matrix multiplication; engler 1997.
 */
#include "rpi.h"
#include "pi-random.h"
#include "cycle-count.h"
#include "arena.h"

extern arena_t *jit_arena;

// trivial veneer so unix code works with libpi
static uint32_t random(void) { 
    return pi_random();
}

static void *
calloc(unsigned n, unsigned e) {
#if 1
    void *p = kmalloc(n*e);
#else
    void *p = arena_alloc(jit_arena, n*e);
#endif
    assert(p);
    return p;
}
  
// multiply nxn square matrices: c = a*b
static void 
matrix_mult(uint32_t **c, 
    uint32_t **a, 
    uint32_t **b, 
    unsigned n) 
{
    for(unsigned i = 0; i < n; i++) {
        for(unsigned j = 0; j < n; j++) {
            uint32_t s = 0;
            for (unsigned k = 0; k < n; k++) {
		        s += a[i][k] * b[k][j];
            }
			c[i][j] = s;
        }
    }
}

static int 
matrix_cmp(const char *msg,
    uint32_t **a, 
    uint32_t **b, 
    unsigned n,
    int verbose_p) 
{
    unsigned n_error = 0;

    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            // no error.
            if(a[i][j] == b[i][j])
                continue;

            n_error++;
            if(verbose_p)
                output("ERROR: <%s>: a[%d][%d]=%d, b[%d][%d]=%d\n",
                        msg, a[i][j], b[i][j]);
        }
    }
    if(n_error)
        panic("%s: %d total errors\n", msg, n_error);
    return n_error;
}

// allocate a square matrix
static uint32_t **
matrix_new(unsigned n) {
  	uint32_t **matrix, *tmp;

  	matrix = calloc(n,sizeof(uint32_t *));
  	tmp = calloc(n*n, sizeof(uint32_t));
    for(int i = 0; i < n; i++) {
        matrix[i] = &tmp[i*n];
    }
	return matrix;
}

static void vec_init(uint32_t *a, 
    unsigned n, 
    unsigned percent_0) 
{
    assert(percent_0 <= 100);

    for(int j=0;j<n;j++) {
        if(random()%100 < percent_0)
            a[j] = 0;
        else
            a[j]= random() & 0xff;
    }
}

static void matrix_init(uint32_t **a, 
    unsigned n, 
    unsigned percent_0) 
{
    for(int i=0;i<n;i++)
        vec_init(a[i], n, percent_0);
}

  
// initialize and allocate NxN square matrix
//      percent spareness = percent 0s.
static inline uint32_t **
matrix_mk(unsigned n, unsigned percent_0) {
    uint32_t **a = matrix_new(n);
    matrix_init(a,n,percent_0);
    return a;
}

static void 
matrix_print(char *s, uint32_t **a, unsigned n) {
    output("\n%s:", s);
    for(int i = 0; i < n; i++) {
        output("\n");
        for(int j = 0; j < n; j++)
            output(" %d ", a[i][j]);
    }
  	output("\n\n");
} 
static void 
vec_print(char *s, uint32_t *a, unsigned n) {
    output("\n%s: {", s);
    for(int i = 0; i < n; i++)
        output(" %d, ", a[i]);
  	output("}\n");
}

static inline uint32_t *
vec_new(unsigned n) {
    return calloc(n,sizeof(uint32_t));
}

static inline uint32_t *
vec_mk(unsigned n, unsigned percent_0) {
    uint32_t *a = vec_new(n);
    vec_init(a,n,percent_0);
    return a;
}

#endif
