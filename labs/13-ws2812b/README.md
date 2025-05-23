## Controlling a  WS2812B light string.

Today you'll write the code to control a WS2812B light string.
The WS2812B is fairly common, fairly cheap.  Each pixel has 8-bits of
color (red-green-blue) and you can turn on individual lights in the array.

Each light strip has an input direction and an output direction (look
at the arrows).  You can attach multiple in series by connecting the
output of the first one to the input of the second.  They can be cut
into pieces, connected, etc.

The WS2812B's are a good example of how to communicate information
using time.  One way to communicate things fancier than on/off is to use
a fancier protocol (e.g., I2C, SPI).  Another is to just use delays ---
if you have good resolution (low variance, precise control over time and
ability to measure it), each nanosecond of delay can signify information.
The WS2812B's use a crude variant of this.

The WS2812B protocol to send a bit of information:
  1. To send a `1` bit: write a `1` for `T1H` nanoseconds (defined in datasheet), 
     then a `0` for T0L nanoseconds.
  2. To send a `0` bit: write a `1` for `T0H` nanoseconds, then a `0` for `T0L` 
     nanoseconds.
  3. To send a byte: send the 7th bit (as above), the 6th, the 5th...
  4. The timings are fairly tight.  The datasheet states +/- 150
     nanoseconds (one cycle on the armv6 is about 1.42 nanoseconds)
     Thus you will have to write your code very carefully.  You will also
     have to be smart about measuring it --- your measurements take time
     and will pollute the times you measure.

The protocol to set N pixels:
  1. There will be some number of pixels on each light string.  (You can 
     count them.)
  2. Each pixel needs 3 bytes of information (G, R, B --- in that order) for the 
     color.
  3. Send each byte using the protocol above.
  4. The first 3 bytes you send will get claimed by the first pixel, the second 3
     bytes by the second, etc.

### Notes.

Common mistakes:
  1. For the inlined GPIO routines make sure you use `volatile` pointers.
  2. When writing to the neopixel array, make sure you discard any out of
     bound pixel values.
  3. If you're not getting fairly tight timings, make sure all you are doing
     is writing to the GPIO address --- no if-statement sanity checking,
     division, calling into `GET32` or `PUT32`.
  4. When you reset the neopixel buffer make sure any pixel that was not
     written to is cleared, otherwise each pixel will remain set to the last
     value written to it.

### Check-off

Regular checkoff:

  1. Implement `code/WS2812.h`.  These should pass the timing
     tests run by `code/0-timing-check.c`.   Since part of this class
     is learning to use datasheets, *please* only use the datasheet we
     have (in `doc/`) and the comments in the given code, don't use blog
     posts, etc.
    
     This header gets wrapped into a simple interface (`code/neopixel.h`)
     Optimize the timings as far as you can and have the code still work.
     The program `2-neopix.c` should work as expected.

  2. Do something novel/cute with the light using your interface.
      1. use your IR remote to control the strip.
      2. use your accel/gyro to control the strip.
      3. control more than one strip at a time.  ideally where you
         merge the writes for each into a single set updates (recall
         we can set and clear many pins at once with a single store).
      4. do the 2d light arrays we have and display words, funny faces,
         etc.  Compose with (1)-(4).
      5. if you have your nrf code working from 140e: use one pi to control
         another over RF.  using the accel/gyro to control a remote strip
         will blow your friends' minds.
      6. for a more technical approach: Max suggested using PWM  to control
         the strip.  Or, if you're ambitious, DMA.

Daniel mode:
  - If you want to Daniel style and write all the code from scratch, the
    interface is pretty small.  You'll just need to put your code in the
    `WS2812b.h` header.   Note: you'll have to do your own timings and
    show that your primitives have accurate timings.


-------------------------------------------------------------------------
### Some timing puzzles `puzzles`

We care about nanosecond timings.  At this level, small changes in code
can lead to noticeable impacts.  One confusing timing bug we had for
awhile was related to instruction alignment --- a timed block of code
that was 32 byte aligned took less time than a non-32-byte aligned block.
There's a couple puzzles in the `puzzle` directory to look at.

-------------------------------------------------------------------------
###  Part 0: hook up your hardware.


To hook up the light strings wire up the:
  - power
  - ground
  - signal to GPIO 21

Some light strips allow you to attach to both sides.  If you look closely
there are little arrows on the light strips.  Make sure you attach the
light strips to the side the arrows are pointing away from.  FOr the
connectors we have: push in the sides and the jumper wire should slide in
and stay pretty firmly.

Bootload the `staff-binaries/2-neopix.bin` and make sure it sends some
pixels around the light strip.

Note: A confusing thing is that the power for the WS2812B must be pretty
close to the power used to drive the data pin.  For long light strings
you'd want to power them with a external power supply --- however this
will require you either use a "level shifter" to raise the power used by
the pi to signal (since its pins are 3.3v for output) or a transistor to
connect/disconnect the pi 5v to the signal wire of the LED.  For today
we play fast and lose with electricity: since we can't see it, presumably
nothing can go wrong!

-------------------------------------------------------------------------
### Part 1: implement the timing routines

The header `code/WS2812b.h` defines routines to call to write a 0, write a 1,
write a pixel to the light array.  It also defines timing constants.
You should fill thees in using the datasheet and comments.  When you
are done, they should pass the timing checks called by `code/0-timing-check.c`.
The workflow: 
  1.  `code/0-timing-check.c` has 6 parts.  
  2. Go through each part in order and make sure your code passes the
     timing tests.
  3. Extension: make the timing tests better!

These routines will look similar to the timing routines you built
for the sonar lab, with the change that they use the cycle counter
(`cycle_cnt_read()`) rather than the microsecond timer.  
   1. First implement `delay_ncycles` (`WS2812B.h`) and make sure the initial 
      timing checks in `0-timing-check.c` succeed.

   2. Then implement `t1h`, `t0h`, `t1l` and `t0l` making sure each passes.

For `t1h`, `t0h`, `t1l` and `t0l` you will need to replace `0` with the
needed nanosecond in the timings in `WS2812B.h`:

        // to send a 1: set pin high for T1H ns, then low for T1L ns.
        T1H = ns_to_cycles(0),        // Width of a 1 bit in ns
        T0H = ns_to_cycles(0),        // Width of a 0 bit in ns
        // to send a 0: set pin high for T0H ns, then low for T0L ns.
        T1L = ns_to_cycles(0),        // Width of a 1 bit in ns
        T0L = ns_to_cycles(0),        // Width of a 0 bit in ns

You will also need to implement inlined versions of your `gpio_set_on`
and `gpio_set_off` (also defined in `WS2812B.h`).

Note: 
  1. We want accurate timings.  That is not the same as fast timings.
  2. HOWEVER: the less instructions our underlying primitives take,
     the less that can go wrong.  Each instruction is a coin toss that
     can go awry (cache miss or conflict, prefetch buffer misalignment,
     register pressure that leads to more loads stores, etc).  
  3. One of the easiest way to cut down instructions is to inline the
     `gpio_set_on` and `gpio_set_off` routines, including getting rid
     of their use of `PUT32` and error checking.  For this you'll just
     write to the GPIO addresses directly --- though make sure you
     mark the memory as `volatile`!  

     After you write these, I would try using them to blink an LED since
     if they are wrong, it will be hard to figure stuff out given the
     extra complexity of the ws2812b.

After your code works, the three tests in the code directory should run
and do what they claim.

  1. Make sure `1-blink.c` works with your code and then
     test your understanding by changing it to turn on different colors
     and different pixels.

  2. We give you a simple neopixel interface as an example (see 
     `neopixel.h` and `neopixel.c`).  Make sure that 
     `2-neopix.c` does something.

-------------------------------------------------------------------------
### Part 2: do something cute with the interface.

A lot of people have gotten a lot of free trips and other things because
they were able to do tricks with light strips.   Try to come up with
something cute using you code.


-------------------------------------------------------------------------
### Extensions

***There are a bunch of extensions: if you see this do a pull***

To make it more interesting:
  1. Trim as many nanoseconds off of the timings as possible and see that the 
     code still works.

  2. An interesting speed extension that I wish I had done before class
     is to speed up the GPIO stores.  Currently these use large constants
     that the compiler will store in the code segment and then load ---
     this load is a expensive data cache miss (see `puzzles/1-const-problems.c`).
     Options:
       1. Put the constants in the scratch registers we used in the pixie 
          lab.
       2. Use VM to remap the GPIO memory to addresses that can be loaded 
          in a single instruction.
       3. Use VM and turn on the data cache (the least sexy option, but works :).

-------------------------------------------------------------------------
#### Additional reading.

Some useful documents:
  * Adafruit [best practices](https://learn.adafruit.com/adafruit-neopixel-uberguide/best-practices)
  * [Protocol](https://developer.electricimp.com/resources/neopixels)
  * Nicely worked out [tolerances for the delays](https://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/)
  * Nice photos and setup of a [level shifter](https://learn.adafruit.com/neopixel-levelshifter/shifting-levels)
  * Sparkfun cursory breakdown of the [protocol](https://learn.sparkfun.com/tutorials/ws2812-breakout-hookup-guide)
  * Discussion if you want to [drive many at once](https://learn.adafruit.com/adafruit-neopixel-uberguide/basic-connections)
