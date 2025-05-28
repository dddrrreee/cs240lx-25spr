# unif[0] = input vector of 64 32-bit elements initialized to some value.
# unif[1] = output vector of 64 32-bit elements
# copies unif[0] to unif[1] using a very slow 16 entry at a time
# load and then store.

.include "../share/vc4inc/vc4.qinc"

ldi r2, 64

mov     r0,     unif           # r0 ← src address
mov     r1,     unif           # r1 ← dst address

mov     vr_setup, vdr_setup_0(0, 16, 1, vdr_h32(1, 0,0))
mov     vr_addr,  r0           # launch DMA LOAD
mov     -,        vr_wait      # stall until load completes


# increment r0 by 64 and read again.
add r0, r0, r2
mov     vr_setup, vdr_setup_0(0, 16, 1, vdr_h32(1, 1,0))
mov     vr_addr,  r0           # launch DMA LOAD
mov     -,        vr_wait      # stall until load completes

add r0, r0, r2
mov     vr_setup, vdr_setup_0(0, 16, 1, vdr_h32(1, 2,0))
mov     vr_addr,  r0           # launch DMA LOAD
mov     -,        vr_wait      # stall until load completes
 
add r0, r0, r2
mov     vr_setup, vdr_setup_0(0, 16, 1, vdr_h32(1, 3,0))
mov     vr_addr,  r0           # launch DMA LOAD
mov     -,        vr_wait      # stall until load completes


# configure VDW for 4 rows × 16 words, stride 64 bytes
mov     vw_setup, vdw_setup_0(4, 16, dma_h32(0,0))
mov     vw_setup, vdw_setup_1(0)   # 64-byte row pitch
mov     vw_addr,  r1            # r_dst = destination bus addr
mov     -,        vw_wait          # wait for DMA to finish

nop
thrend
nop
nop
