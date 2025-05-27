.include "../share/vc4inc/vc4.qinc"


mov vw_setup, vpm_setup(4, 1, h32(0))   # Write 4 rows, 
                                        # increment by 1 after each write, 
                                        # start at VPM coord 0,0

ldi vpm, 0xdeadbeef                     # Row 1
mov -, vw_wait 

ldi vpm, 0xbeefdead                     # Row 2
mov -, vw_wait 

ldi vpm, 0xfaded070                     # Row 3
mov -, vw_wait

ldi vpm, 0xfeedface                     # Row 4
mov -, vw_wait

ldi vw_setup, vdw_setup_0(4, 16, dma_h32(0,0))  # DMA write 4 rows, 
                                                # each length 16, 
                                                # starting at VPM coord 0,0
mov r0, unif
mov vw_addr, r0
mov -, vw_wait
nop;  thrend
nop
nop
