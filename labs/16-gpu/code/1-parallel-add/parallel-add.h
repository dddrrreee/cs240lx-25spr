// TODO: SWAP THESE
#include "staffaddshader.h"
// #include "addshader.h"

#include "rpi.h"
#include <stdint.h>

#define GPU_MEM_FLG 0xC // cached=0xC; direct=0x4
#define GPU_BASE 0x40000000

// FEEL FREE TO CHANGE THIS VALUE AND SEE HOW SPEEDUP CHANGES - WILL NEED TO BE
// DIVISIBLE BY 16 (good extension is write a kernel that doesn't need this)
#define N 1048576
// TODO: AFTER YOU DECIDE WHAT YOUR UNIFORMS SHOULD BE, SET THIS CONSTANT
// (Ours is 4, yours doesn't have to be)
// ALSO MAKE SURE YOU SWAP SHADER HEADER FILES (ABOVE)
#define NUM_UNIFS 4

struct addGPU
{
	uint32_t A[N];
	uint32_t B[N];
	uint32_t C[N];
	uint32_t code[sizeof(addshader) / sizeof(uint32_t)];
	uint32_t unif[1][NUM_UNIFS];
	uint32_t unif_ptr[1];
	uint32_t mail[2];
	uint32_t handle;
};

void vec_add_init(volatile struct addGPU **gpu, int n);

int vec_add_exec(volatile struct addGPU *gpu);

void vec_add_release(volatile struct addGPU *gpu);