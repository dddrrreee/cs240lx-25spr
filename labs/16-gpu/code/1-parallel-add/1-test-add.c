#include "parallel-add.h"

/* SIMPLE TEST TO COMPARE YOUR KERNEL WITH CPU OUTPUT */
void test_add(void)
{
    int i, j;
    volatile struct addGPU *add_gpu;
    vec_add_init(&add_gpu, N);

    for (i = 0; i < N; i++)
    {
        add_gpu->A[i] = 32 + i;
        add_gpu->B[i] = 64 + i;
        add_gpu->C[i] = 0;
    }

    // Test addition
    printk("\nTesting addition on GPU...\n");
    printk("Memory before addition: %x %x %x %x\n", add_gpu->C[0], add_gpu->C[1], add_gpu->C[2], add_gpu->C[3]);

    int start_time = timer_get_usec();
    int iret_add = vec_add_exec(add_gpu);
    int end_time = timer_get_usec();
    int gpu_add_time = end_time - start_time;

    printk("Memory after addition:  %d %d %d %d\n", add_gpu->C[0], add_gpu->C[1], add_gpu->C[2], add_gpu->C[3]);

    for (i = 0; i < N; i++)
    {
        if (add_gpu->C[i] != (32 + i) + (64 + i))
        {
            panic("Add Iteration %d: %d + %d = %d. INCORRECT", i, add_gpu->A[i], add_gpu->B[i], add_gpu->C[i]);
        }
        else if (i*16 % N == 0)
        {
            printk("Add Iteration %d: %d + %d = %d. CORRECT\n", i, add_gpu->A[i], add_gpu->B[i], add_gpu->C[i]);
        }
    }

    // CPU benchmarks
    int cpu_add_time = 0;

    start_time = timer_get_usec();
    for (i = 0; i < N; i++)
        add_gpu->C[i] = add_gpu->A[i] + add_gpu->B[i];
    end_time = timer_get_usec();
    cpu_add_time = end_time - start_time;

    printk("CPU Addition Time: %d us\n", cpu_add_time);
    printk("GPU Addition Time: %d us\n", gpu_add_time);

    printk("Speedup: %dx\n", cpu_add_time / gpu_add_time);

    vec_add_release(add_gpu);
}

void notmain(void)
{
    printk("Testing addition on GPU...\n");
    
    test_add();

    // delay_ms(6000);
}