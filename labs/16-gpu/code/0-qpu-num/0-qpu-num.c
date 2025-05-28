#include "qpu-num.h"

void notmain(void)
{
	volatile struct GPU *gpu;
	int ret = gpu_prepare(&gpu);
	if (ret < 0)
		return;

	//Copy the shader code onto our gpu.
	memcpy((void *)gpu->code, qpu_numshader, sizeof gpu->code);

	//Initialize the uniforms - now we have multiple qpus
	for (int i=0; i<NUM_QPUS; i++) {
	    gpu->unif[i][0] = i;
	    gpu->unif[i][1] = GPU_BASE + (uint32_t) &gpu->output;
	    gpu->unif_ptr[i] = GPU_BASE + (uint32_t)&gpu->unif[i];
	}
	for (int i=0; i<NUM_QPUS; i++) {
		for (int j=0; j<16; j++) {
			gpu->output[i][0] = 0;
		}
	}
	printk("Running code on GPU...\n");

	int start_time = timer_get_usec();
	int iret = gpu_execute(gpu);
	int end_time = timer_get_usec();
	printk("DONE!\n");
	int gpu_time = end_time - start_time;

	printk("Time taken on GPU: %d us\n", gpu_time);	

	//We are computing i*WIDTH + j at each i, j

	for (int i=0; i<NUM_QPUS; i++) {
		for (int j=0; j<1; j++) {
			printk("QPU-Num %d ran on QPU %d\n", i, gpu->output[i][j]);
		}
	}

	gpu_release(gpu);
}
