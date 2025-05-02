#ifndef __SS_PIXIE_H__
#define __SS_PIXIE_H__

// our two system calls.  should be in a different header
// so can include in .S files, but to keep simple...
#define PIXIE_SYS_DIE   0
#define PIXIE_SYS_STOP  1


// start tracing
void pixie_start(void);
// stop tracing: return number of instructions.
unsigned pixie_stop(void);

// print out the top N counts.
//
// note: this is heavy-handed: you probably want some
// additional routines.
void pixie_dump(unsigned N);

// 0 = quiet, !0 = chatty
void pixie_verbose(int verbose_p);

// client can define this: will get called when 
// we see a PIXIE_SYS_DIE system call.
void pixie_die_handler(uint32_t regs[16]);

#endif
