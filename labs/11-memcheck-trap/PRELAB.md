### Prelab for trap based memchecking.

The next couple of labs replicate research papers.  So you should
read them.  The one for today's lab is the original Purify paper:

  - [./docs/cs343-annot-purify.pdf](./docs/cs343-annot-purify.pdf)

Some rough lecture notes to help out (or confuse, depending :):

  - [./docs/purify-notes.txt](./docs/purify-notes.txt)

You should read it at least a couple of times thoroughly.  Building
it will be a lot easier if you understand it :)

To hopefully make things more concrete there are two example programs:

  1. `example-single-step`: shows how to do single stepping with the full
     register save/restore code from 140e.  It's similar to lab 9's
     pixie example starter code, so hopefully shouldn't be too hard
     to understand.

  2. `example-trap`: shows how to use domain traps to trace memory
     operations.  If you took 140e, this looks like our pinned-vm code,
     where we take domain faults.  You may  want to read the README
     for review.

You should look through the code (it has a lot of expository comments),
run it, and ideally change it to see how things work.  It will make make
lab much faster.
