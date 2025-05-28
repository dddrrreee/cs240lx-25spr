.include "../share/vc4inc/vc4.qinc"

# LOAD UNIFORMS INTO REGISTERS  
mov   ra0, unif #A
mov   ra1, unif #B
mov   ra2, unif #C
# TODO: LOAD IN ANY OTHER UNIFORMS YOU DEFINE. MAKE SURE THE ORDER MATCHES


# YOU WILL PROBABLY NEED A LOOP OF SOME SORT
:loop

    # TODO: DMA READ A AND B FROM PHYSICAL MEMORY TO VPM

    # TODO: DMA READ A
    mov vr_setup, vdr_setup_0(SOMETHING) # USE vdr_setup_0 MACRO
                                         # How will you do the read?
    mov vr_addr, ra1            # USE YOUR UNIFORM ADDRESS FOR A 
                                # (will probably need to change with the loop)
    mov -, vr_wait              # KICK OFF THE READ

    # TODO: REPEAT FOR B

    # TODO: SETUP VPM READS/WRITES (the vpm_setup macro to configure read/write) 
    mov vr_setup, vpm_setup(WHAT GOES HERE?)
    mov vw_setup, vpm_setup(WHAT GOES HERE?)

    # READ FROM VPM INTO REGISTERS
    mov r1, vpm
    mov -, vw_wait        # WE HAVE TO DO THIS FOR VPM READS/WRITES

    mov r2, vpm
    mov -, vw_wait        # WE HAVE TO DO THIS FOR VPM READS/WRITES

    # TODO: DO THE ADD

    # TODO: WRITE IT BACK OUT TO VPM

    # TODO: DMA WRITE FROM VPM TO PHYSICAL MEMORY
    mov vw_setup, vdw_setup_0(SOMETHING HERE) # USE vdw_setup_0 MACRO TO 
                                              # DEFINE HOW YOU'LL DO THE READ
    mov vw_addr, ra3            # USE YOUR UNIFORM ADDRESS FOR C
    mov -, vw_wait              # KICK OFF THE WRITE


    
    # FIGURE OUT WHETHER TO CONTINUE THE LOOP
    # THE FOLLOWING IS EQUIVALENT TO: for (int i=<initial ra3 value>, i > 0; i--)

    sub.setf ra3, ra3, 1
    brr.anynz -, :loop

    # DO NOT REMOVE DO NOT REMOVE DO NOT REMOVE
    # After a branch, the 3 subsequent instructions are executed
    # If you want to do something useful with these 3 instructions in your kernel, feel free
    nop
    nop
    nop
    # END DO NOT REMOVE 


# End of kernel
:end
thrend
mov interrupt, 1
nop
nop
