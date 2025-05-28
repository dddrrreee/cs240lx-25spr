#include "cond.h"

void notmain(void)
{
	volatile struct GPU *gpu;
	int ret = gpu_prepare(&gpu);
	if (ret < 0)
		return;

	//Copy the shader code onto our gpu.
	memcpy((void *)gpu->code, condshader, sizeof gpu->code);

	//Initialize the uniforms - now we have multiple qpus
	gpu->unif[0][0] = GPU_BASE + (uint32_t) &gpu->input;  
	gpu->unif[0][1] = GPU_BASE + (uint32_t) &gpu->output;

	//Target value to reach - this must be bigger than 
	//the max value in the input array
	gpu->unif[0][2] = TARGET;  
	gpu->unif_ptr[0] = GPU_BASE + (uint32_t)&gpu->unif[0];

	for (int i=0; i<16; i++) {
		gpu->input[i] = i;
		gpu->output[i] = 0;
	}

	printk("Running code on GPU...\n");

	int start_time = timer_get_usec();
	int iret = gpu_execute(gpu);
	int end_time = timer_get_usec();
	printk("DONE!\n");
	int gpu_time = end_time - start_time;

	printk("Time taken on GPU: %d us\n", gpu_time);	

	//Check to make sure every value in the vector is equal to TARGET
	for (int i=0; i<16; i++) {
		printk("%s: GOT %d, EXPECTED %d\n", gpu->output[i] == TARGET ? "SUCCESS" : "FAILURE", gpu->output[i], TARGET);
	}

	gpu_release(gpu);
}
