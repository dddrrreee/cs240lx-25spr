#include "matrix-lib.h"

typedef uint32_t (*vec_fn_t)(uint32_t *a);

// slow version.
uint32_t vec_dot(uint32_t *a, uint32_t *b, int n);

// this is a simple example of "partial evaluation"
vec_fn_t jit_dot(uint32_t *b, unsigned n);

void jit_free_all(void);
void jit_init(void);
