# unif[0] = input vector of 64 32-bit elements initialized to some value.
# unif[1] = output vector of 64 32-bit elements 
#
# use a loop to write [0..15], [0..15]+16, [0..15]+32, [0..15]+48 
# to output (so 4 16-element 32-bit vectors)
#
# ignores input.
.include "../share/vc4inc/vc4.qinc"

.macro write_16, rv
    mov     vpm, rv
    mov     -, vw_wait
    add     rv, rv, 16
    nop
.endm

.macro output_64, r_out
    mov     vw_setup, vdw_setup_0(4, 16, dma_h32(0,0))
    mov     vw_addr,  r_out            # r_dst = destination bus addr
    mov     -,        vw_wait          # wait for DMA to finish
.endm

.macro done
    nop
    thrend
    nop
    nop
.endm

mov     r0,     unif           # r0 ← src address
mov     r1,     unif           # r1 ← dst address

mov     r2, elem_num

# not sure if we can do a different way?
mov vw_setup, vpm_setup(4, 1, h32(0))   # Write 4 rows,
                                        # increment by 1 after each write,
                                        # start at VPM coord 0,0

mov     r2, elem_num
ldi     r3, 4  

# loop 4 times.
:loop
    mov         vpm, r2
    mov         -, vw_wait
    nop
    add         r2, r2, 16

    # is r3 < 4?
    sub.setf r3, r3, 1     # r0 -= 1   (NZ flags updated)
    brr.anynn -, :loop

    # i think it has 3 delay slots?
    nop
    nop
    nop

output_64(r1)
done
