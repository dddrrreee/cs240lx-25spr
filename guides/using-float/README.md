### pi with floating point.

We added floating point to the libpi.  It should work out of the box if you
do a pull and recompile.

There are two examples: `fp-example` uses floats, `complex-example` uses
complex numbers.

Note the Makefile slightly changes in that you have to specify where to
find the floating point library [cs240lx-25spr/lib/libm](../../lib/libm).
The current library only compiles a small subset of the math library.
Should be easy enough to add the rest.

If you compile and run `fp-example`:

        % cat fp-example.c  
        // trivial example of using floating point + our simple math
        // library.
        #include "rpi.h"
        #include "rpi-math.h"
        void notmain(void) {
            double x = 3.1415;
            printk("hello from pi=%f float!!\n", x);
        
            double v[] = { M_PI, 0, M_PI/2.0, M_PI/2.0*3.0 };
            for(int i = 0; i < 4; i++)  {
                printk("COS(%f) = %f\n", v[i], cos(v[i]));
                printk("sin(%f) = %f\n", v[i], sin(v[i]));
            }
        }

You should get:

        hello from pi=3.141500 float!!
        COS(3.141592) = -1.0
        sin(3.141592) = 0.0
        COS(0.0) = 1.0
        sin(0.0) = 0.0
        COS(1.570796) = 0.0
        sin(1.570796) = 1.0
        COS(4.712388) = -0.0
        sin(4.712388) = -1.0
        DONE!!!
