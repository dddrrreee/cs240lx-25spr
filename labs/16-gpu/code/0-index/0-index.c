#include "index.h"

void notmain(void)
{
	volatile struct GPU *gpu;
	int ret = gpu_prepare(&gpu);
	if (ret < 0)
		return;

	//Copy the shader code onto our gpu.
	memcpy((void *)gpu->code, indexshader, sizeof gpu->code);

	//Initialize the uniforms - now we have multiple qpus
	for (int i=0; i<NUM_QPUS; i++) {
	    gpu->unif[i][0] = HEIGHT;
		gpu->unif[i][1] = WIDTH;
	    gpu->unif[i][2] = NUM_QPUS;
	    gpu->unif[i][3] = i;
	    gpu->unif[i][4] = GPU_BASE + (uint32_t) &gpu->output;
	    gpu->unif_ptr[i] = GPU_BASE + (uint32_t)&gpu->unif[i];
	}
	for (int i=0; i<HEIGHT; i++) {
		for (int j=0; j<WIDTH; j++) {
			gpu->output[i][j] = 0;
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

	for (int i=0; i<HEIGHT; i++) {
		for (int j=0; j<WIDTH; j++) {
			if (gpu->output[i][j] != i*WIDTH + j) {
				printk("ERROR: gpu->output[%d][%d] = %d\n", i, j, gpu->output[i][j]);
			} else if (i * 4 % HEIGHT == 0 && j * 4 % WIDTH == 0) {
				printk("CORRECT: gpu->output[%d][%d] = %d (%d * %d + %d)\n", i, j, gpu->output[i][j], i, WIDTH, j);
			}
		}
	}

	gpu_release(gpu);
}
