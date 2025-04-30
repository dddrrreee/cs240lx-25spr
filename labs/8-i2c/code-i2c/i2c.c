/*
 * simplified i2c implementation --- no dma, no interrupts. should 
 * probobaly add both.  
 *
 * the pi's we use can only access i2c1 (gpio pins 2,3) so we hardwire 
 * everything in.
 *
 * datasheet starts at p28 in the broadcom pdf.
 *
 * make sure you use device barriers at the start and end!  we don't
 * know what else the client code was doing.
 */
#include "rpi.h"
#include "libc/helper-macros.h"
#include "i2c.h"

// example of using a structure to control a device.  
// note:
//   1. the use of static asserts to check offsets.
//   2. we don't use bitfields here, and only read/write
//      using 32 values. p28: "All accesses are assumed to 
//      be 32-bit"
//   3. probably should have just stuck with enums, but
//      the starter code was already out, so :)
typedef struct {
	uint32_t control; // "C" register, p29
	uint32_t status;  // "S" register, p31

#	define check_dlen(x) assert(((x) >> 16) == 0)
	uint32_t dlen; 	// p32. number of bytes to xmit, recv
					// reading from dlen when TA=1
					// or DONE=1 returns bytes still
					// to recv/xmit.  
					// reading when TA=0 and DONE=0
					// returns the last DLEN written.
					// can be left over multiple pkts.

    // Today address's should be 7 bits.
#	define check_dev_addr(x) assert(((x) >> 7) == 0)
	uint32_t 	dev_addr;   // "A" register, p 33, device addr 

	uint32_t fifo;  // p33: only use the lower 8 bits.
#	define check_clock_div(x) assert(((x) >> 16) == 0)
	uint32_t clock_div;     // p34
	// we aren't going to use this: fun to mess w/ tho.
	uint32_t clock_delay;   // p34
	uint32_t clock_stretch_timeout;     // broken on pi.
} RPI_i2c_t;

// offsets from table "i2c address map" p 28
_Static_assert(offsetof(RPI_i2c_t, control) == 0, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, status) == 0x4, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, dlen) == 0x8, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, dev_addr) == 0xc, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, fifo) == 0x10, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, clock_div) == 0x14, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, clock_delay) == 0x18, "wrong offset");

/*
 * There are three BSC masters inside BCM. The register addresses starts from
 *	 BSC0: 0x7E20_5000 (0x20205000)
 *	 BSC1: 0x7E80_4000
 *	 BSC2 : 0x7E80_5000 (0x20805000)
 * the PI can only use BSC1.
 */
static volatile RPI_i2c_t *i2c = (void*)0x20804000; 	// BSC1

// write <nbytes> of data from input array <data> to device <addr>.
//
// should extend so this can fail.
int i2c_write(unsigned addr, uint8_t data[], unsigned nbytes) {
    todo("implement");
	return 1;
}

// read <nbytes> of data from device <addr> and write it into 
// output array <data>
//
// should extend so it returns failure.
int i2c_read(unsigned addr, uint8_t data[], unsigned nbytes) {
    todo("implement");
	return 1;
}

// initialize the i2c hardware to default speed.
//
// notes:
//  - make sure you setup the GPIO pins to enable i2c.
//  - uses a clock divider of 0.
void i2c_init(void) {
    todo("setup GPIO, setup i2c, sanity check results");
}

// shortest will be 130 for i2c accel.
void i2c_init_clk_div(unsigned clk_div) {
    todo("same as init but set the clock divider");
}
