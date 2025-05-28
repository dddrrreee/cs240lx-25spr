.include "../share/vc4inc/vc4.qinc"

# Read uniforms into registers
mov   ra0, unif #Input   
mov   ra1, unif #Output
mov   ra2, unif #Target


#DMA Read one row
mov vr_setup, vdr_setup_0(1, 16, 1, vdr_h32(1, 0, 0))
mov vr_addr, ra0
mov -, vr_wait

#VPM Read the row into rb1
mov vr_setup, vpm_setup(1, 1, h32(0))
mov rb1, vpm
mov -, vw_wait

#Iterate over the vector, adding 1 to any values less than target
#Exit once all are equal to target
:add_loop

#rb1 is curr, ra2 is target
#We see if target - curr is nonzero (assume target > curr)
mov r1, rb1
mov r2, ra2
sub.setf  r2, r2, r1

#If target > curr, add 1 to curr
#What happens if we get rid of condition code?
add.ifnz rb1, r1, 1

#If any values still aren't equal to target,
# we branch back to the loop
#What happens if we make this allnz?
brr.anynz -, :add_loop
nop
nop
nop


mov vw_setup, vpm_setup(1, 1, h32(0))
mov vpm, rb1
mov -, vw_wait

mov vw_setup, vdw_setup_0(1, 16, dma_h32(0,0))
mov vw_addr, ra1
mov -, vw_wait

# End of kernel
:end
thrend
mov interrupt, 1
nop


