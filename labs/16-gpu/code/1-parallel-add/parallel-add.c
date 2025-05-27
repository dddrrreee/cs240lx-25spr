#include "rpi.h"
#include <stddef.h>
#include <string.h>
#include "parallel-add.h"
#include "mailbox.h"

// TODO: SWAP THESE

// #include "addshader.h"
#include "staffaddshader.h"

void add_gpu_prepare(
	volatile struct addGPU **gpu)
{
	uint32_t handle, vc;
	volatile struct addGPU *ptr;

	/* TURN ON THE QPU */
	if (qpu_enable(1))
		panic("Failed to enable GPU");

	/* ALLOCATE MEMORY FOR THE STRUCT QPU */
	handle = mem_alloc(sizeof(struct addGPU), 4096, GPU_MEM_FLG);
	if (!handle)
	{
		qpu_enable(0);
		panic("Failed to allocate GPU memory");
	}

	/* CLAIM THE BUS ADDRESS OF THE MEMORY */
	vc = mem_lock(handle);

	/* USE THE Pi 0 GPU OFFSET TO GET AN ADDRESS WE CAN READ/WRITE ON THE CPU */
	ptr = (volatile struct addGPU *)(vc - 0x40000000);
	if (ptr == NULL)
	{
		mem_free(handle);
		mem_unlock(handle);
		qpu_enable(0);
		panic("Failed to convert handle to GPU bus address");
	}

	/* INITIALIZE STRUCT QPU FIELDS*/
	ptr->handle = handle;
	ptr->mail[0] = GPU_BASE + (uint32_t)&ptr->code;
	ptr->mail[1] = GPU_BASE + (uint32_t)&ptr->unif;

	*gpu = ptr;
	return;
}

/* SEND THE CODE AND UNIFS TO THE GPU (see docs p. 89-91)*/
uint32_t add_gpu_execute(volatile struct addGPU *gpu)
{
	return gpu_fft_base_exec_direct(
		(uint32_t)gpu->mail[0],
		(uint32_t *)gpu->unif_ptr,
		1);
}

/* RELEASE MEMORY AND TURN OFF QPU */
void vec_add_release(volatile struct addGPU *gpu)
{
	uint32_t handle = gpu->handle;
	mem_unlock(handle);
	mem_free(handle);
	qpu_enable(0);
}

// TODO: SET UP THE UNIFORMS FOR YOUR KERNEL
void vec_add_init(volatile struct addGPU **gpu, int n)
{
	add_gpu_prepare(gpu);

	volatile struct addGPU *ptr = *gpu;
	memcpy((void *)ptr->code, addshader, sizeof ptr->code);

	ptr->unif[0][0] = GPU_BASE + (uint32_t)&ptr->A;
	ptr->unif[0][1] = GPU_BASE + (uint32_t)&ptr->B;
	ptr->unif[0][2] = GPU_BASE + (uint32_t)&ptr->C;

	todo("ADD ANY OTHER UNIFORMS YOU NEED FOR THE PARALLEL-ADD KERNEL (AND CHANGE NUM_UNIFS CONSTANT)");

	ptr->unif_ptr[0] = GPU_BASE + (uint32_t)&ptr->unif;
}

int vec_add_exec(volatile struct addGPU *gpu)
{
	int start_time = timer_get_usec();
	int iret = add_gpu_execute(gpu);
	int end_time = timer_get_usec();

	return end_time - start_time;
}
