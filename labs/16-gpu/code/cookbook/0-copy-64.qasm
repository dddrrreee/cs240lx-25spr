# unif[0] = input vector of 64 32-bit elements initialized to some value.
# unif[1] = output vector of 64 32-bit elements
# copies unif[0] to unif[1]

.include "../share/vc4inc/vc4.qinc"

mov     r0,     unif           # r0 ← src address
mov     r1,     unif           # r1 ← dst address

# note: if we delete <vdr_setup_1> doesn't work --- only copies
# 16 entries, despite a bunch of different attempts.
mov     vr_setup, vdr_setup_1(64)
mov     vr_setup, vdr_setup_0(0, 16, 4, vdr_h32(1, 0,0))
mov     vr_addr,  r0           # launch DMA LOAD
mov     -,        vr_wait      # stall until load completes

# configure VDW for 4 rows × 16 words
mov     vw_setup, vdw_setup_0(4, 16, dma_h32(0,0))
mov     vw_addr,  r1            # r_dst = destination bus addr
mov     -,        vw_wait          # wait for DMA to finish

nop
thrend
nop
nop

