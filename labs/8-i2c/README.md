## I2C

Since this is midterm week, we'll do a low-key device lab by building
the main black box of the IMU lab: the i2c device driver.


The i2c protocol.  You used our staff i2c code on tuesday to
communicate with your accel/gyro, so it makes sense to write
it yourself.  The broadcom document pages 28---36 describes it.

You'll notice that the i2c datasheet looks similar to UART
(fixed-size FIFO queue for transmit and receive, the need to check
if data or space is available, control over speed, errata, etc).
The more devices you do the more you'll notice they share common
patterns.  The nice thing: there exists an N s.t. after doing N
devices, doing N+1 is pretty quick.

The wikipedia for the i2c protocol gives a pretty easy pseudo-code you
can use to do a bit-banged version.

Checkoff:
  1. i2c hardware driver.  Should drop into last lab and give sensible
     values.  
  2. i2c software driver.  Should drop into last lab and give sensible
     values.  
  3. Both of these should pass the gyro and accel self-test.  Self-
     test was optional from last lab but it found so many issues
     in code that we're adding it as a requirement for today's lab.
  
Various extension ideas:
  1. SW i2c.  This is useful for banging a bunch of devices. 
  2. Overclock your pi and measure how much faster you can get it before 
     things break down.  Ideally you'd read the temperature and downthrottle
     when things get "too hot" (not sure what that is :).
  3. Drive a bunch of accels at once.
  4. Do loop back where you SW i2c to yourself and see how fast you 
     can make it: use two threads, one for sender, one for receiver,
     and do a cswitch when they are waiting using  delays (the code
     in libpi will call `rpi_wait` that you can just write to call
     `rpi_yield`).

------------------------------------------------------------------------------
### 1. I2C driver: `code-i2c/i2c.c`

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

Initialization: `i2c_init`

  1. You'll need to setup the GPIO SCL and SDA pins (you can see those in 
     `docs/gpio.png`).  
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

     Set the control reg to read and start transfer.

     Wait until the transfer has started.

  2. Read the bytes: you'll have to check that there is a byte available
     each time.

  3. Do the end of a transfer: use status to wait for `DONE` (p32).  Then
     check that TA is 0, and there were no errors.

Write has the same transfer start (step 1) and end (step 3). As with
uart you'll have to wait until there is space and then you write 8 bits
using a `PUT32` (not `PUT8`).

The code:
  1. There is currently just a simple program that reads the device ID.
  2. To run your accel you'll either copy or symlink the code into a subdirectory
     in `code-i2c` or put `mpu6050.c` and `mpu6050.h` into libpi, the driver
     into `code-i2c`, and link against it.
