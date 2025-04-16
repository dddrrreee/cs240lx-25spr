@ ldr r0, [pc, #4]    ; 8020 <hello+0x10>
@ bl  0x8070
@ pop {r4, pc}

@label:
@b  label

@push { lr }
@pop { lr }

@ andeq   r9, r0, r8, ror #4
@bx lr

push {lr}
@ ldr r0, =[pc, #20]
ldr r0, _label
nop
pop {lr}
bx lr


b _label;
nop
nop
nop
_label:                   .word 5


push {r0-r14}
pop {r0-r14}
