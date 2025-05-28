# unif[0] = input vector of 64 32-bit elements initialized to some value.
# unif[1] = output vector of 64 32-bit elements 
#
# writes [0..15] to output (16 32-bit words), ignores input
.include "../share/vc4inc/vc4.qinc"

mov     r0,     unif           # r0 ← src address
mov     r1,     unif           # r1 ← dst address

# not sure if we can do a different way?
mov vw_setup, vpm_setup(1, 1, h32(0))   # Write 1 row,
                                        # increment by 1 after each write,
                                        # start at VPM coord 0,0

# get 0..15 into r2
mov     r2, elem_num
# write it to vpm
mov     vpm, r2
mov     -, vw_wait

# configure VDW for 4 rows × 16 words
mov     vw_setup, vdw_setup_0(4, 16, dma_h32(0,0))
mov     vw_addr,  r1            # r_dst = destination bus addr
mov     -,        vw_wait          # wait for DMA to finish

nop
thrend
nop
nop
