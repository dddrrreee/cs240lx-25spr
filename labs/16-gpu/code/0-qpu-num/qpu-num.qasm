.include "../share/vc4inc/vc4.qinc"

# Read uniforms into registers
mov   ra0, unif #QPU_NUM    
mov   ra1, unif #Address



mov ra10, ra3 #i = QPU_NUM
mov r2, vpm_setup(1, 1, h32(0))     
    
#For VPM Write, use VPM Row at index QPU_NUM (For h32 VPM setup, VPM Y is bits 0:5 p. 57)
add vw_setup, ra0, r2               
#VPM Write
mov vpm, qpu_num

mov -, vw_wait

#For DMA write, choose VPM Row at index QPU_NUM (for DMA, VPM Y is bits 7:13 p. 58)
shl r1, ra0, 7                  

#Write 1 row, length 16, start at 0,0
mov r2, vdw_setup_0(1, 16, dma_h32(0,0)) 

#Add our calculated index to the macro
add vw_setup, r1, r2                
    

mov r1, ra0            #j_base
shl r1, r1, 6           #row_pointer = j_base * (4 bytes)
add vw_addr, ra1, r1    #Address = Uniform_address + array_pointer
mov -, vw_wait          # Kick off dma write

# End of kernel
:end
thrend
mov interrupt, 1
nop


