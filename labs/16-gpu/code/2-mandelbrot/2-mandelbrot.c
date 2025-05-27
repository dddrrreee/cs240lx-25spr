#include "mandelbrot.h"

void notmain(void)
{
	volatile struct GPU *gpu;
	gpu_prepare(&gpu);

	// Copy the shader code onto our gpu.
	memcpy((void *)gpu->code, mandelbrotshader, sizeof gpu->code);

	// Initialize the uniforms - now we have multiple qpus
	for (int i = 0; i < NUM_QPUS; i++)
	{
		gpu->unif[i][0] = RESOLUTION;
		gpu->unif[i][1] = hex_recip((float)RESOLUTION);
		gpu->unif[i][2] = MAX_ITERS;
		gpu->unif[i][3] = NUM_QPUS;
		gpu->unif[i][4] = i;
		gpu->unif[i][5] = GPU_BASE + (uint32_t)&gpu->output;
		gpu->unif_ptr[i] = GPU_BASE + (uint32_t)&gpu->unif[i];
	}

	// ZERO OUT THE GPU Output array
	for (int i = 0; i < 2 * RESOLUTION; i++)
	{
		for (int j = 0; j < 2 * RESOLUTION; j++)
		{
			gpu->output[i][j] = 0;
		}
	}

//USE THIS FOR DEBUGGING PURPOSES - CHANGE TO 0 TO RUN PROPERLY
#if 1
	printk("Running code on GPU...\n");

	int debug_time = timer_get_usec();
	int debug_iret = gpu_execute(gpu);
	int debug_end_time = timer_get_usec();
	printk("DONE!\n");
	int debug_gpu_time = debug_end_time - debug_time;

	printk("Time taken on GPU: %d us\n", debug_gpu_time);	

	//We are computing i*WIDTH + j at each i, j

	for (int i=0; i<2*RESOLUTION; i++) {
		for (int j=0; j<2*RESOLUTION; j++) {
			if (gpu->output[i][j] != i*2*RESOLUTION + j) {
				printk("ERROR: gpu->output[%d][%d] = %d\n", i, j, gpu->output[i][j]);
			} else if (i * 4 % (2*RESOLUTION) == 0 && j * 4 % (2*RESOLUTION) == 0) {
				printk("CORRECT: gpu->output[%d][%d] = %d (%d * %d + %d)\n", i, j, gpu->output[i][j], i, 2*RESOLUTION, j);
			}
		}
	}

	gpu_release(gpu);
	clean_reboot();
#endif

	// GPU execution
	printk("Running code on GPU...\n");
	int start_time = timer_get_usec();
	int iret = gpu_execute(gpu);
	int end_time = timer_get_usec();
	printk("DONE!\n");
	int gpu_time = end_time - start_time;

	// CPU execution
	printk("Running code on CPU...\n");
	uint32_t output_cmp[2 * RESOLUTION][2 * RESOLUTION];
	int cpu_time = 0;
	float recip = float_recip((float)RESOLUTION);
	start_time = timer_get_usec();

	// Use this as a guide for your GPU Kernel
	for (int i = 0; i < 2 * RESOLUTION; i++)
	{
		float y = -1.0f + (recip * (float)i);
		for (int j = 0; j < 2 * RESOLUTION; j++)
		{
			float x = -1.0f + (recip * (float)j);
			float u = 0.0;
			float v = 0.0;
			float u2 = u * u;
			float v2 = v * v;
			int k;
			for (k = 1; k < MAX_ITERS && (u2 + v2 < 4.0); k++)
			{
				v = 2 * u * v + y;
				u = u2 - v2 + x;
				u2 = u * u;
				v2 = v * v;
			}
			if (k >= MAX_ITERS)
			{
				output_cmp[i][j] = 1;
			}
			else
			{
				output_cmp[i][j] = 0;
			}
		}
	}
	end_time = timer_get_usec();
	cpu_time = end_time - start_time;

	printk("Time taken on CPU: %d us\n", cpu_time);
	printk("Time taken on GPU: %d us\n", gpu_time);

	printk("Speedup: %dx\n", cpu_time / gpu_time);

	printk("%d\n", output_cmp[0][0]);
	unsigned char GPU_OUT[2 * RESOLUTION][2 * RESOLUTION];
	for (int i = 0; i < 2 * RESOLUTION; i++)
	{
		for (int j = 0; j < 2 * RESOLUTION; j++)
		{
			if (gpu->output[i][j] > 0)
			{
				GPU_OUT[i][j] = 255;
			}
			else
			{
				GPU_OUT[i][j] = 0;
			}
		}
	}

	// Use fat32 to write the completed image out to disk.
	kmalloc_init(8 * FAT32_HEAP_MB);
	pi_sd_init();

	mbr_t *mbr = mbr_read();

	mbr_partition_ent_t partition;
	memcpy(&partition, mbr->part_tab1, sizeof(mbr_partition_ent_t));
	assert(mbr_part_is_fat32(partition.part_type));

	fat32_fs_t fs = fat32_mk(&partition);

	pi_dirent_t root = fat32_get_root(&fs);

	char *hello_name = "OUTPUT.PGM";
	fat32_delete(&fs, &root, hello_name);
	fat32_create(&fs, &root, hello_name, 0);
	trace("CREATED\n");
	int size;
	char *data = buildPGM(&size, 2 * RESOLUTION, 2 * RESOLUTION, GPU_OUT);
	pi_file_t hello = (pi_file_t){
		.data = data,
		.n_data = size,
		.n_alloc = size,
	};

	fat32_write(&fs, &root, hello_name, &hello);

	gpu_release(gpu);
}
