.include "../share/vc4inc/vc4.qinc"

# Read uniforms into registers
mov   ra0, unif #HEIGHT    
mov   ra1, unif #WIDTH
mov   ra2, unif #NUM_QPU
mov   ra3, unif #QPU_NUM
mov   ra4, unif #ADDRESS


mov ra10, ra3 #i = QPU_NUM

:row_loop


mov ra11, 0 # j_base = 0

shl r1, ra1, 2 #bytes_per_row = 4*WIDTH
mul24 ra12, r1, ra10 #row_base_addr = i * bytes_per_row


:column_loop

    mov r0, ra11 # j_base 
    add r0, r0, elem_num # j = j_base + elem_num
    
    
    mov r1, ra10  # i
    mov r2, ra1   # WIDTH

    mul24 r1, r1, r2       #RESULT = i * WIDTH  
    add r1, r1, r0         #RESULT += j 

    #Write 1 row, increment by 1, start at 0,0
    mov r2, vpm_setup(1, 1, h32(0))     
    
    #For VPM Write, use VPM Row at index QPU_NUM (For h32 VPM setup, VPM Y is bits 0:5 p. 57)
    add vw_setup, ra3, r2               

    #VPM Write
    mov vpm, r1
    mov -, vw_wait

    #For DMA write, choose VPM Row at index QPU_NUM (for DMA, VPM Y is bits 7:13 p. 58)
    shl r1, ra3, 7                  
    
    #Write 1 row, length 16, start at 0,0
    mov r2, vdw_setup_0(1, 16, dma_h32(0,0)) 
    
    #Add our calculated index to the macro
    add vw_setup, r1, r2                
    

    mov r1, ra11            #j_base
    shl r1, r1, 2           #row_pointer = j_base * (4 bytes)
    add r1, ra12, r1        #array_pointer = row_base_addr + row_pointer
    add vw_addr, ra4, r1    #Address = Uniform_address + array_pointer
    mov -, vw_wait          # Kick off dma write
    
    add ra11, ra11, 16      #j_base += 16
    mov r1, ra1             #WIDTH

    #if (any j < WIDTH) { restart column_loop }
    sub.setf r1, ra11, r1    
    brr.anyc -, :column_loop    

    nop
    nop
    nop

    mov r1, ra2                 #NUM_QPU
    mov r2, ra10                #i
    add ra10, r1, r2            # i += NUM_QPU
    mov r1, ra0                 # HEIGHT
    

    #Note that every i will be the same here
    #Because our SIMD vectors are horizontal

    #if i < HEIGHT {RESTART ROW LOOP} 
    sub.setf r1, ra10, r1       
    brr.anyc -, :row_loop
    nop
    nop
    nop

# End of kernel
:end
thrend
mov interrupt, 1
nop


