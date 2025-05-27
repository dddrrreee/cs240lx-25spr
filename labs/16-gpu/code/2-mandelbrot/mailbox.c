#include <stdint.h>
#include "rpi.h"

#define MAILBOX_BASE 0x2000B880
#define MAILBOX_READ (*(volatile uint32_t *)(MAILBOX_BASE + 0x0))
#define MAILBOX_STATUS (*(volatile uint32_t *)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE (*(volatile uint32_t *)(MAILBOX_BASE + 0x20))

#define MAILBOX_FULL 0x80000000
#define MAILBOX_EMPTY 0x40000000

#define V3D_BASE 0x20C00000
#define V3D_SRQSC (V3D_BASE + 0x418)
#define V3D_L2CACTL (V3D_BASE + 0x020)
#define V3D_SLCACTL (V3D_BASE + 0x024)
#define V3D_SRQPC (V3D_BASE + 0x0430)
#define V3D_SRQUA (V3D_BASE + 0x0434)
#define V3D_SRQCS (V3D_BASE + 0x043c)
#define V3D_DBCFG (V3D_BASE + 0x0e00)
#define V3D_DBQITE (V3D_BASE + 0x0e2c)
#define V3D_DBQITC (V3D_BASE + 0x0e30)
//
// Basic mailbox I/O routines
//

// Write 'data' to the mailbox on the specified channel.
void mailbox_write(uint8_t channel, uint32_t data)
{
	// Wait until mailbox is not full.
	while (MAILBOX_STATUS & MAILBOX_FULL)
	{
	}
	// Lower 4 bits are used for channel.
	MAILBOX_WRITE = (data & ~0xF) | (channel & 0xF);
}

// Read from the mailbox on the specified channel.
uint32_t mailbox_read(uint8_t channel)
{
	uint32_t data;
	while (1)
	{
		data = MAILBOX_READ;
		if ((data & 0xF) == channel)
			return data & ~0xF;
	}
}

// Perform a mailbox property call.
// The message buffer 'msg' must be 16-byte aligned.
// Returns nonzero if the call succeeded.
int mbox_property(uint32_t *msg)
{
	// Check alignment.
	if ((uint32_t)msg & 0xF) return 0;
	mailbox_write(8, (uint32_t)msg);
	while (mailbox_read(8) != (uint32_t)msg);

	return (msg[1] == 0x80000000);
}

//
// Mailbox property calls for GPU memory and QPU control.
// All property messages are sent on mailbox channel 8.
//

uint32_t mem_alloc(uint32_t size, uint32_t align, uint32_t flags)
{

	uint32_t p[9] __attribute__((aligned(16))) =
		{
			9 * sizeof(uint32_t), // size
			0x00000000,			  // process request
			0x3000c,			  // (the tag id)
			3 * sizeof(uint32_t), // (size of the buffer)
			3 * sizeof(uint32_t), // (size of the data)
			size,				  // (num bytes)
			align,				  // (alignment)
			flags,				  // (MEM_FLAG_L1_NONALLOCATING)
			0					  // end tag
		};
	assert(mbox_property(p));
	return p[5];
}

uint32_t mem_free(uint32_t handle)
{

	uint32_t p[7] __attribute__((aligned(16))) =
		{
			7 * sizeof(uint32_t), // size
			0x00000000,			  // process request
			0x3000f,			  // (the tag id)
			1 * sizeof(uint32_t), // (size of the buffer)
			1 * sizeof(uint32_t), // (size of the data)
			handle,				  // (handle)
			0					  // end tag
		};
	assert(mbox_property(p));
	return p[5];
}

uint32_t mem_lock(uint32_t handle)
{

	uint32_t p[7] __attribute__((aligned(16))) =
		{
			7 * sizeof(uint32_t), // size
			0x00000000,			  // process request
			0x3000d,			  // (the tag id)
			1 * sizeof(uint32_t), // (size of the buffer)
			1 * sizeof(uint32_t), // (size of the data)
			handle,				  // (handle)
			0					  // end tag
		};
	assert(mbox_property(p));
	return p[5];
}

uint32_t mem_unlock(uint32_t handle)
{
	uint32_t p[7] __attribute__((aligned(16))) =
		{
			7 * sizeof(uint32_t), // size
			0x00000000,			  // process request
			0x3000e,			  // (the tag id)
			1 * sizeof(uint32_t), // (size of the buffer)
			1 * sizeof(uint32_t), // (size of the data)
			handle,				  // (handle)
			0					  // end tag
		};
	assert(mbox_property(p));
	return p[5];
}

uint32_t qpu_enable(uint32_t enable)
{

	uint32_t p[7] __attribute__((aligned(16))) =
		{
			7 * sizeof(uint32_t), // size
			0x00000000,			  // process request
			0x30012,			  // (the tag id)
			1 * sizeof(uint32_t), // (size of the buffer)
			1 * sizeof(uint32_t), // (size of the data)
			enable,				  // (enable QPU)
			0					  // end tag
		};
	assert(mbox_property(p));
	return p[5];
}

unsigned gpu_fft_base_exec_direct(
	uint32_t code,
	uint32_t unifs[],
	int num_qpus)
{
	//printk("RUNNING WITH %d\n QPUS", num_qpus);
	PUT32(V3D_DBCFG, 0); // Disallow IRQ

	PUT32(V3D_DBQITE, 0);  // Disable IRQ
	PUT32(V3D_DBQITC, -1); // Resets IRQ flags

	PUT32(V3D_L2CACTL, 1 << 2); // Clear L2 cache
	PUT32(V3D_SLCACTL, -1);		// Clear other caches

	PUT32(V3D_SRQCS, (1 << 7) | (1 << 8) | (1 << 16)); // Reset error bit and counts

	for (unsigned q = 0; q < num_qpus; q++)
	{ // Launch shader(s)

		PUT32(V3D_SRQUA, (uint32_t)unifs[q]); // Set the uniforms address
		PUT32(V3D_SRQPC, (uint32_t)code); // Set the program counter
	}


	// Busy wait polling
	while (((GET32(V3D_SRQCS) >> 16) & 0xff) != num_qpus);

	return 0;
}
