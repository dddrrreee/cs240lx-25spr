## I2C

### Errata

Two big changes for software I2c.

First,  delete the checking if the pin is input or output and always
set it to the right thing.   Just delete:

        unsigned SCL_is_input_p:1;
        unsigned SDA_is_input_p:1;

Second, My I2C looks slightly different from the wikipedia.  
I send a 1 as the last bit on the last byte:

```
// Read a byte from I2C bus
uint8_t i2c_read_byte(i2c_t *h, bool done_p) {
    uint8_t byte = 0;

    // msb reads
    for (unsigned bit = 0; bit < 8; ++bit)
        byte = (byte << 1) | i2c_read_bit(h);

    // if it's the last byte, <done_p>=1
    i2c_write_bit(h, done_p);
    return byte;
}


int sw_i2c_read(i2c_t *h, uint8_t data[], unsigned nbytes) {
    i2c_start_cond(h);  // xfer start

    // send address for read: low bit is 1
    if(!i2c_write_byte(h, h->addr<<1|1))
        panic("nake: failed to write byte\n");

    // write a bit after each bit: are we done?
    for(unsigned i = 0; i < nbytes; i++)
        data[i] = i2c_read_byte(h, i==(nbytes-1));

    i2c_stop_cond(h);  // xfer end
    return 1;
}
```

### overview

Since this is midterm week, we'll do a low-key device lab by building
the main black box of the IMU lab: the i2c device driver.

The i2c protocol.  You used our staff i2c code on Tuesday to
communicate with your accel/gyro, so it makes sense to write
it yourself.  To remove all mystery, we will do it using both
the hardware i2c in the bcm2835 and a bit-banged version.

  - hardware i2c: described in the Broadcom document pages 28---36 of
    the BCM document.  Excerpted: [./docs/BCM2835-i2c.pdf][hw-i2c].

    You'll notice that the i2c datasheet looks similar to UART
    (fixed-size FIFO queue for transmit and receive, the need to check
    if data or space is available, control over speed, errata, etc).
    The more devices you do the more you'll notice they share common
    patterns.  The nice thing: there exists an N s.t. after doing N
    devices, doing N+1 is pretty quick.

  - software i2c: [The Wikipedia for the i2c protocol][bit-bang-i2c]
    gives a pretty easy pseudo-code you can use to do a bit-banged
    version.
 

Checkoff:
  1. i2c hardware driver.  Should drop into last lab and give sensible
     values.  
  2. ***this is now an extension***: i2c software driver.  Should drop
     into last lab and give sensible values.

  3. Both of these should pass the gyro and accel self-test.  Self-
     test was optional from last lab but it found so many issues
     in code that we're adding it as a requirement for today's lab.

------------------------------------------------------------------------------
### 1. hardware I2C driver: `code-i2c/i2c.c`

***NOTE: three easy mistakes***:
  - Most registers have fields you can just write to clear, but 
    if the register has a device enable flag in it, you better keep 
    this bit set!
  - Printing to debug can mess up i2c and cause a correct driver
    to act incorrectly.  So be careful (I wasted 20 minutes).
  - If the device gets in a bad state you'll have to do a hard 
    power cycle of everything or it will just ignore you.  (You'll
    be able to tell b/c the staff i2c driver won't work either.)

By now you should be able to bang out this driver pretty quickly.
Hints:
  1. We'll use BSC1.  The document states we can't use BSC2.  I haven't
     tried BSC0.
  2. As a first step, try to read known values and make sure the device
     is talking to you.   If you look in the document you'll see that
     on boot-up `cdiv` (p34) should return 0x5dc and `clkt` (p35) should
     return 0x40.  Worth checking them to make sure things are working.
  3. In general: Make sure you have device barriers when setting GPIO and I2C.  
     Along with when we enter and leave the routines.

You can see that the GPIO SCL and SDA pins are pins 2 and 3.

Initialization: `i2c_init`

  1. You'll need to setup the GPIO SCL and SDA pins. Note that these
     are different than the i2c device so you need a dev barrier.

     You can see those in [../../docs/gpio.png][gpio-pins].  You can
     then look up how to set them in the BCM doc p 102:

<p align="center">
  <img src="images/gpio-i2c.png" width="500" />
</p>

  2. Then enable the BSC we want (C register, p 29) to use along with
     any clock divider (p 34, default is 0).
  3. Clear the BSC status register (S register, p 31):
     clear any errors and clear the done field.
  4. After done: Make sure there is no active transfer (S register, p31)
     and along with a few other fields that make sense (up to you).


Again, make sure you use device barriers.

Reading data: `i2c_read`:
  1. Do the start of a transfer.  

     Before starting: wait until transfer is not active.

     Then: check in status that: fifo is empty, there was no clock
     stretch timeout and there were no errors.  We shouldn't see any of
     these today.

     Clear the DONE field in status since it appears it can still be
     set from a previous invocation.

     Set the device address and length.

     Set the control reg to read and start transfer.  Common mistake:
     don't disable the i2c hardware device, and make sure you clear
     the read bit so it doesn't preserve the old value.  

     Wait until the transfer has started.

  2. Read the bytes: you'll have to check that there is a byte available
     each time.

  3. Do the end of a transfer: use status to wait for `DONE` (p32).  Then
     check that TA is 0, and there were no errors.

Write has the same transfer start (step 1) and end (step 3). As with
uart you'll have to wait until there is space and then you write 8 bits
using a `PUT32` (not `PUT8`).

#### What to do first

  1. The Makefile links in the accel driver and the staff code from
     last lab.
  2. If you do `make` everything should run and produce readings.
  3. Now change:

            # code-i2c/Makefile
            # COMMON_SRC = i2c.c

     To:

            COMMON_SRC = i2c.c

     And the makefile will compile in the `i2c.c` and start using that.

  4. If you get stuck you can always comment this out back out and
     it will use the staff i2c.  If the code still seems broken,
     power-cycle your pi by unplugging the usb cord and plug it back in.
     The IMU's state can stay across reboots since it doesn't know you
     rebooted, it only knows it still has power.

------------------------------------------------------------------------------
### 2. software I2C driver: `code-sw-i2c/i2c.c`

Like many digital protocols, building a bit-banged version of the 
hardware protocol (1) can be easier than writing the hardware version
and (2) removes alot of the mystery of what is going on.
Besides just a life of knowledge, having a software version makes 
the world a better place in several ways:
  1. You can work around hardware bugs.  For example, the BCM has
     a known i2c "clock stretching" bug that you can eliminate with the
     software version.
  2. You can work around hardware limitations: typically hardware
     has a small set of speeds you can select among using a "clock
     divider": a bit-banged version can be much more fine-tuned for
     different device speeds.
  3. You can use different pins.  I2c hardware has pre-assigned
     addresses (often at most two).  With bit-banging you can solve
     address conflicts by putting a device on different pins.  (You can
     also do the same to make wiring easier.)
  4. It's very easy to port.  Device hardware descriptions are not famous
     for simplicity, nor for error-free prose. Further, in many new chips,
     there simply is no hardware description (or if there is, it's not
     in English).  If we have a bit banged protocol, porting to a new
     machine involves setting GPIO pins and having some kind of cycle
     (or microsecond) counter.  Porting the bit-banged driver today,
     could easily just take a couple minutes on a new chip.


The following is a raw cut-and-paste from the [Wikipedia bit-bang i2c
implementation][bit-bang-i2c].  If you define the different helper
routines it should just work --- we'll discuss these helpers
after the code.

```
// Hardware-specific support functions that MUST be customized:
#define I2CSPEED 100
void I2C_delay(void);
bool read_SCL(void); // Return current level of SCL line, 0 or 1
bool read_SDA(void); // Return current level of SDA line, 0 or 1
void set_SCL(void); // Do not drive SCL (set pin high-impedance)
void clear_SCL(void); // Actively drive SCL signal low
void set_SDA(void); // Do not drive SDA (set pin high-impedance)
void clear_SDA(void); // Actively drive SDA signal low
void arbitration_lost(void);

bool started = false; // global data

void i2c_start_cond(void)
{
  if (started) { 
    // if started, do a restart condition
    // set SDA to 1
    set_SDA();
    I2C_delay();
    set_SCL();
    while (read_SCL() == 0) { // Clock stretching
      // You should add timeout to this loop
    }

    // Repeated start setup time, minimum 4.7us
    I2C_delay();
  }

  if (read_SDA() == 0) {
    arbitration_lost();
  }

  // SCL is high, set SDA from 1 to 0.
  clear_SDA();
  I2C_delay();
  clear_SCL();
  started = true;
}

void i2c_stop_cond(void)
{
  // set SDA to 0
  clear_SDA();
  I2C_delay();

  set_SCL();
  // Clock stretching
  while (read_SCL() == 0) {
    // add timeout to this loop.
  }

  // Stop bit setup time, minimum 4us
  I2C_delay();

  // SCL is high, set SDA from 0 to 1
  set_SDA();
  I2C_delay();

  if (read_SDA() == 0) {
    arbitration_lost();
  }

  started = false;
}

// Write a bit to I2C bus
void i2c_write_bit(bool bit)
{
  if (bit) {
    set_SDA();
  } else {
    clear_SDA();
  }

  // SDA change propagation delay
  I2C_delay();

  // Set SCL high to indicate a new valid SDA value is available
  set_SCL();

  // Wait for SDA value to be read by target, minimum of 4us for standard mode
  I2C_delay();

  while (read_SCL() == 0) { // Clock stretching
    // You should add timeout to this loop
  }

  // SCL is high, now data is valid
  // If SDA is high, check that nobody else is driving SDA
  if (bit && (read_SDA() == 0)) {
    arbitration_lost();
  }

  // Clear the SCL to low in preparation for next change
  clear_SCL();
}

// Read a bit from I2C bus
bool i2c_read_bit(void)
{
  bool bit;

  // Let the target drive data
  set_SDA();

  // Wait for SDA value to be written by target, minimum of 4us for standard mode
  I2C_delay();

  // Set SCL high to indicate a new valid SDA value is available
  set_SCL();

  while (read_SCL() == 0) { // Clock stretching
    // You should add timeout to this loop
  }

  // Wait for SDA value to be written by target, minimum of 4us for standard mode
  I2C_delay();

  // SCL is high, read out bit
  bit = read_SDA();

  // Set SCL low in preparation for next operation
  clear_SCL();

  return bit;
}

// Write a byte to I2C bus. Return 0 if ack by the target.
bool i2c_write_byte(bool send_start,
                    bool send_stop,
                    unsigned char byte)
{
  unsigned bit;
  bool     nack;

  if (send_start) {
    i2c_start_cond();
  }

  for (bit = 0; bit < 8; ++bit) {
    i2c_write_bit((byte & 0x80) != 0);
    byte <<= 1;
  }

  nack = i2c_read_bit();

  if (send_stop) {
    i2c_stop_cond();
  }

  return nack;
}

// Read a byte from I2C bus
unsigned char i2c_read_byte(bool nack, bool send_stop)
{
  unsigned char byte = 0;
  unsigned char bit;

  for (bit = 0; bit < 8; ++bit) {
    byte = (byte << 1) | i2c_read_bit();
  }

  i2c_write_bit(nack);

  if (send_stop) {
    i2c_stop_cond();
  }

  return byte;
}

void I2C_delay(void)
{ 
  volatile int v;
  int i;

  for (i = 0; i < I2CSPEED / 2; ++i) {
    v;
  }
}
```


#### Implementing helpers.

Given the above code, the game is to build the different helpers.

```
void I2C_delay(void);
bool read_SCL(void); // Return current level of SCL line, 0 or 1
bool read_SDA(void); // Return current level of SDA line, 0 or 1
void set_SCL(void); // Do not drive SCL (set pin high-impedance)
void clear_SCL(void); // Actively drive SCL signal low
void set_SDA(void); // Do not drive SDA (set pin high-impedance)
void clear_SDA(void); // Actively drive SDA signal low
void arbitration_lost(void);
```

Before starting, you'll want to keep in mind a few weird differences in
the i2c compared to some of the other devices we've done:

  1. Rather than being initialized at the beginning to either
     output or input, the GPIO pins used get switched from input to
     output depending on where you are in the i2c protocol.  No
     other device we used has done this!
  2. Unlike other devices,  we *do not* write a 1 by:
       1. Initialization: set GPIO to output.
       2. Use: Writing a 1 to it.

     Since this would cause problems when running multiple devices.  
     Instead, we set a pin to "high impedance" by:
       1. Initialization: set each GPIO pin to be a pull-up.
          This adds a resistance of about 50k ohm to the GPIO pin.
          Once you set this, you don't change it.
       2. Use: To "write" a 1: simply set the GPIO pin to input.
          This will cause any device reading to see a 1 but won't cause
          electrical conflicts if multiple devices do this.

The easy routines:
  - `I2C_delay`: you can just use a microsecond delay.  (Fancier is to 
     have a custom speed per i2c device.)
  - `read_SCL`: set the SCL pin to an input and return a read of it.
  - `read_SDA`: set the SDA pin as an input and return a read of it.
  - `arbitration_lost`: panic that arbitration is lost.

The weird routines:
  - `set_SCL`: set pin to high impedance (just switch to input).
  - `set_SDA`: set pin to high impedance (just switch to input).
  - `clear_SCL`: set pin to output and write a 0.
  - `clear_SDA`: set pin to output and write a 0.

The wrong way to track what state a pin is in, is to stare really
hard at the code.  This will reliably produce a bunch of bugs.
Instead you can just track the current state of the SDA and SCL pins,
and do the needful.


To make things a bit easier, we defined the following structure
in `sw-i2c.h`:

```
typedef struct i2c {
    // is this a transmitter or receiver?
    unsigned is_transmit_p;
    bool started_p;

    uint8_t addr;
    uint8_t SCL;
    uint8_t SDA;

    // XXX: NOTE: we eliminated the use of is_input_p
    // just cut this.
    unsigned SCL_is_input_p:1;
    unsigned SDA_is_input_p:1;
} i2c_t;
```

During initialization you should set the SCL and SDA pins to
   1. Input.
   2. Pullups (`gpio_set_pullup`).

Then:
   1. If you want to "write" a 1: Set the pin to `gpio_input`.
   2. If you want to write a 0: set the pin to `gpio_output` and write a 0.


#### What to do first

  1. The Makefile links in the accel driver, the staff code from
     last lab, and a staff version of the bit-banged i2c.
  2. If you do `make` everything should run and produce readings.
  3. Now change:

            # code-sw-i2c/Makefile
            # COMMON_SRC = sw-i2c.c 
            STAFF_OBJS += staff-sw-i2c.o

     To:

            COMMON_SRC = sw-i2c.c 
            # STAFF_OBJS += staff-sw-i2c.o

     And the makefile will compile in the `sw-i2c.c` and start using that.

  4. If you get stuck you can always comment this out back out and
     it will use the staff i2c.  If the code still seems broken,
     power-cycle your pi by unplugging the usb cord and plug it back in.
     The IMU's state can stay across reboots since it doesn't know you
     rebooted, it only knows it still has power.

------------------------------------------------------------------------------
### Extensions.

Some interesting extensions:
  1. How fast can you run a given IMU in terms of data / sec?
  2. How fast can you run two IMU's in terms of data/sec?
  3. I havent' done this: but can you use two simultaneous IMUs
     to get better data by interpolating?  

     Ideally if an IMU produces readings every T microseconds, you'd
     start the first IMU at 0, and the second at T/2 microseconds so
     you'd have 2x the readings.  In theory this should let you get more
     accurate better pitch, yaw, etc.

  4. Use DMA to pull the readings off the IMU.  We'll do DMA in a few
     labs, but if you can figure it out on your own you'll obviously
     learn more.

  5. Do loop back where you SW i2c to yourself and see how fast you 
     can make it: use two threads, one for sender, one for receiver,
     and do a cswitch when they are waiting using  delays (the code
     in libpi will call `rpi_wait` that you can just write to call
     `rpi_yield`).

[bit-bang-i2c]: https://en.wikipedia.org/wiki/I%C2%B2C
[hw-i2c]: ./docs/BCM2835-i2c.pdf
[gpio-pins]: ../../docs/gpio.png

