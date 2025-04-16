add r0, r0, r1
add r0, r0, #1
mov r0, #1

nop

push    {r4, lr}
ldr r0, [pc, #4]    ; 8020 <hello+0x10>
bl  0x8070
pop {r4, pc}

@ andeq   r9, r0, r8, ror #4


push {r0,r4}
