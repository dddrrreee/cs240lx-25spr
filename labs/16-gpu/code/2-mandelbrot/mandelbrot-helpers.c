#include "mandelbrot.h"

// int_to_ascii and build_PGM are GPTed functions for creating the output file.

// A small helper function to convert a positive integer to decimal ASCII.
// Returns the number of characters written into 'buf'. No sign handling.
static int int_to_ascii(int val, char *buf)
{
    // We'll build digits in reverse order in 'temp', then copy them to 'buf'.
    char temp[16];
    int idx = 0;

    // Handle zero explicitly
    if (val == 0)
    {
        buf[0] = '0';
        return 1;
    }

    // Extract digits in reverse
    while (val > 0)
    {
        int digit = val % 10;
        val /= 10;
        temp[idx++] = (char)('0' + digit);
    }

    // Now reverse them into buf
    int written = 0;
    while (idx > 0)
    {
        buf[written++] = temp[--idx];
    }

    return written;
}

char *buildPGM(int *outSize, int HEIGHT, int WIDTH, const unsigned char input_image[HEIGHT][WIDTH])
{
    int pos = 0;
    char *g_pgmData = kmalloc(64 + HEIGHT * WIDTH);
    // -- 1) Write the PGM "magic number": "P5\n"
    g_pgmData[pos++] = 'P';
    g_pgmData[pos++] = '5';
    g_pgmData[pos++] = '\n';

    // -- 2) Write "WIDTH HEIGHT\n"
    pos += int_to_ascii(WIDTH, &g_pgmData[pos]);
    g_pgmData[pos++] = ' ';
    pos += int_to_ascii(HEIGHT, &g_pgmData[pos]);
    g_pgmData[pos++] = '\n';

    // -- 3) Write "255\n" as the max pixel value
    g_pgmData[pos++] = '2';
    g_pgmData[pos++] = '5';
    g_pgmData[pos++] = '5';
    g_pgmData[pos++] = '\n';

    // -- 4) Fill the raw 16x16 pixel data.
    //    Each pixel is a single byte in [0..255].
    //    We'll do a small gradient for demonstration.
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            // Simple pattern: add x+y, just to have something visually distinct
            g_pgmData[pos++] = input_image[y][x];
        }
    }

    // total bytes in the g_pgmData
    *outSize = pos;

    // Return pointer to the buffer
    return g_pgmData;
}

float float_recip(float value)
{
    union
    {
        float f;
        uint32_t i;
    } pun;
    pun.f = value;
    pun.i = 0x7EF127EA - pun.i;
    float r = pun.f;
    r = r * (2.0f - value * r);
    r = r * (2.0f - value * r);
    r = r * (2.0f - value * r);
    return r;
}

uint32_t hex_recip(float value)
{
    union
    {
        float f;
        uint32_t i;
    } pun;
    pun.f = float_recip(value);
    return pun.i;
}

void gpu_prepare(
    volatile struct GPU **gpu)
{
    uint32_t handle, vc;
    volatile struct GPU *ptr;

    if (qpu_enable(1))
        panic("Failed to enable GPU");

    handle = mem_alloc(sizeof(struct GPU), 4096, GPU_MEM_FLG);
    if (!handle)
    {
        qpu_enable(0);
        panic("Failed to allocate GPU memory");
    }
    vc = mem_lock(handle);

    ptr = (volatile struct GPU *)(vc - 0x40000000);
    if (ptr == NULL)
    {
        mem_free(handle);
        mem_unlock(handle);
        qpu_enable(0);
        panic("Failed to convert handle to GPU bus address");
    }

    ptr->handle = handle;
    ptr->mail[0] = vc + offsetof(struct GPU, code);
    ptr->mail[1] = vc + offsetof(struct GPU, unif);

    *gpu = ptr;
    return;
}

uint32_t gpu_execute(volatile struct GPU *gpu)
{
    return gpu_fft_base_exec_direct(
        (uint32_t)gpu->mail[0],
        (uint32_t *)gpu->unif_ptr,
        NUM_QPUS);
}

void gpu_release(volatile struct GPU *gpu)
{
    uint32_t handle = gpu->handle;
    mem_unlock(handle);
    mem_free(handle);
    qpu_enable(0);
}
