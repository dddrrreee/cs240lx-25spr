## A 4x4 keyboard matrix (very simple)

In the interests of giving people final project building blocks, this
is a quick lab for a standard 4x4 (16 button) matrix keyboard.  It has
4 rows, and 4 columns --- labeled R1-R4 and C1-C4 on the pcb.

A positive of this device is that it is cheap and plentiful.  A downside
is that you'll need 8 jumpers (4 for each row, 4 for each column),
which is can be annoying since it's 16 coin tosses (one for each end)
that a connection is loose or mistaken.  With that said 8 is (in practice)
more than 2x easier than 16 jumpers (one for each button).

You'll also note that because of some device cleverness, we don't need
need 10 jumpers b/c the board doesn't require power or ground, which is
cool and a bit weird.

Initialization:
  1. Set the four row pins as outputs.
  2. Set the four column pins as inputs and pull-downs.

The basic idea: 
  1. Turn row 0 on.
  2. Read each of the pins associated with the four columns.
  3. If column pin i is on, that means button (0, i) is depressed.
  4. Turn row 0 off.
  5. Repeat steps 1-4 for rows the other rows (1, 2, 3).

Pretty simple in terms of code.  Most of the problems should be hardware
wiring.  Common mistakes: use a GPIO pin used for something else that
is active!   For example, pins 14 and 15 are used for the UART, so if
you use these you'll get random button pushes and/or corrupted printing.
