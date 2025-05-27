#include "rpi.h"
#include <stddef.h>
#include <string.h>

#include "mailbox.h"
#include "deadbeef.h"

#define GPU_MEM_FLG 0xC // cached=0xC; direct=0x4
#define GPU_BASE 0x40000000

/*
	This will be allocated on GPU memory
	Anything you want to share between Host/Device goes here
	The general format should be:
	PROGRAM BUFFERS (esp. input/output arrays)
	CODE BUFFER (your assembled qasm kernel)
	UNIF[NUM_QPUS][NUM_UNIFS] (a 2D array where each inner array is the
		array of uniforms for the corresponding QPU)
	UNIF_PTR[NUM_QPUS] (The addresses to the base of the 1D UNIF array for each QPU)
	MAIL[2] (GPU CODE ADDR, GPU UNIF BASE ADDR)
	HANDLE (Provided by the mailbox for GPU-allocated code)
*/
struct GPU
{
	uint32_t output[64];
	unsigned code[sizeof(deadbeef) / sizeof(uint32_t)];
	unsigned unif[1][1];
	unsigned unif_ptr[1];
	unsigned mail[2];
	unsigned handle;
};

/*
	Sets up the GPU memory
*/
void gpu_prepare(
	volatile struct GPU **gpu)
{
	unsigned handle, vc;
	volatile struct GPU *ptr;

	// Makes a mailbox call to turn on GPU, checks for failure
	if (qpu_enable(1))
		panic("Failed to enable GPU");

	// Makes a mailbox call to allocate a page-aligned struct GPU in GPU bus memory
	handle = mem_alloc(sizeof(struct GPU), 4096, GPU_MEM_FLG);
	if (!handle)
	{
		qpu_enable(0);
		panic("Failed to allocate GPU memory");
	}

	// Makes a mailbox call to convert the handle to a GPU bus address we have exclusive access to
	vc = mem_lock(handle);

	// Convert to a CPU address so we can initialize values
	ptr = (volatile struct GPU *)(vc - GPU_BASE);
	if (ptr == NULL)
	{
		mem_free(handle);
		mem_unlock(handle);
		qpu_enable(0);
		panic("Failed to convert handle to GPU bus address");
	}

	// Now initialize values (will be seen by CPU and GPU)
	ptr->handle = handle;

	// 2-word mailbox array should be GPU addresses of code, unif
	ptr->mail[0] = GPU_BASE + (uint32_t)&ptr->code;
	ptr->mail[1] = GPU_BASE + (uint32_t)&ptr->unif;

	*gpu = ptr;
	return;
}

unsigned gpu_execute(volatile struct GPU *gpu)
{
	// See mailbox.c, mailbox.h
	// ARG1: GPU Pointer to start of code
	// ARG2: GPU array of uniform pointers with length NUM_QPUs
	// ARG3: NUM_QPUs (this example only uses 1)
	return gpu_fft_base_exec_direct(
		(uint32_t)gpu->mail[0],
		(uint32_t *)gpu->unif_ptr,
		1);
}

void gpu_release(volatile struct GPU *gpu)
{
	unsigned handle = gpu->handle;
	mem_unlock(handle);
	mem_free(handle);
	qpu_enable(0);
}

void notmain(void)
{
	printk("Testing GPU DMA writes...\n");
	int i, j;

	// After prep, gpu points to a struct GPU allocated on the GPU (address offset by 0x40000000)
	volatile struct GPU *gpu;
	gpu_prepare(&gpu);

	memcpy((void *)gpu->code, deadbeef, sizeof gpu->code);

	// Note that these are addresses the GPU will see, so we need to add offset
	// Only uniform is base of output array
	gpu->unif[0][0] = GPU_BASE + (uint32_t)&gpu->output;

	// Only running on 1 QPU
	gpu->unif_ptr[0] = GPU_BASE + (uint32_t)&gpu->unif[0];

	// Initialize output
	memset((void *)gpu->output, 0xff, sizeof gpu->output);

	// All 64 values should be 0xffffffff
	printk("Memory before running code: %x %x %x %x\n", gpu->output[0], gpu->output[16], gpu->output[32], gpu->output[48]);

	// Running our kernel
	trace("Executing process... result: %d\n", gpu_execute(gpu));

	// Should be 4 16-wide vectors, holding 0xdeadbeef, 0xbeefdead, 0xfaded070, 0x0
	printk("Memory after running code:  %x %x %x %x\n", gpu->output[0], gpu->output[16], gpu->output[32], gpu->output[48]);

	// Mailbox cleanup functions
	gpu_release(gpu);
}
