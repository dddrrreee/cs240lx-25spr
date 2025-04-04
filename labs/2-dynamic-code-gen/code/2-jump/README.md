Make a new linker script that inserts:
  1. Sticks a header at the start of each binary.
  2. Where the first word of the header is a branch instruction that
     will jump over the header.  This should work with arbitrary 
     sizes.
  3. includes the string "hello world\n" right after the jump 
     instruction.
