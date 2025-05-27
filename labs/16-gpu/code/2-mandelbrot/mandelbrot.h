#include "rpi.h"
#include <stddef.h>
#include <string.h>
#include "fat32/code/pi-sd.h"
#include "fat32/code/fat32.h"
#include "mailbox.h"
#include "mandelbrotshader.h"

#define GPU_MEM_FLG 0xC // cached=0xC; direct=0x4
#define GPU_BASE 0x40000000

#define RESOLUTION 64
#define MAX_ITERS 100
#define NUM_QPUS 8 

//TODO: CHANGE THE NUMBER OF UNIFS TO MATCH YOUR KERNEL. OURS HAS 6, yours doesn't have to
#define NUM_UNIFS 6

struct GPU
{
	uint32_t output[2*RESOLUTION][2*RESOLUTION];
	uint32_t code[sizeof(mandelbrotshader) / sizeof(uint32_t)];
	uint32_t unif[NUM_QPUS][NUM_UNIFS];
	uint32_t unif_ptr[NUM_QPUS];
	uint32_t mail[2];
	uint32_t handle;
};


//HELPER FUNCTIONS (See mandelbrot-helpers.c if your curious)
static int int_to_ascii(int val, char *buf);
char *buildPGM(int *outSize, int HEIGHT, int WIDTH, const unsigned char input_image[HEIGHT][WIDTH]);
float float_recip(float value);
uint32_t hex_recip(float value);
void gpu_prepare(volatile struct GPU **gpu);
uint32_t gpu_execute(volatile struct GPU *gpu);
void gpu_release(volatile struct GPU *gpu);