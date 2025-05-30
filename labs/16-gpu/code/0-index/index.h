#include "rpi.h"
#include <stddef.h>
#include <string.h>
#include "mailbox.h"
#include "indexshader.h"

#define GPU_MEM_FLG 0xC // cached=0xC; direct=0x4
#define GPU_BASE 0x40000000

#define WIDTH 64
#define HEIGHT 32
#define NUM_QPUS 16

//TODO: CHANGE THE NUMBER OF UNIFS TO MATCH YOUR KERNEL. OURS HAS 6, yours doesn't have to
#define NUM_UNIFS 5

struct GPU
{
	uint32_t output[HEIGHT][WIDTH];
	uint32_t code[sizeof(indexshader) / sizeof(uint32_t)];
	uint32_t unif[NUM_QPUS][NUM_UNIFS];
	uint32_t unif_ptr[NUM_QPUS];
	uint32_t mail[2];
	uint32_t handle;
};

int gpu_prepare(volatile struct GPU **gpu);
uint32_t gpu_execute(volatile struct GPU *gpu);
void gpu_release(volatile struct GPU *gpu);
