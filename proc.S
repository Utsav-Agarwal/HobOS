.section ".text.proc"

.global setup_stack
.global save_curr_context
.global resume_from_context
.global curr_core_id
.global curr_core_el
.global core_stack_base
.global jump_to_EL1

#define CORE_STACK_SIZE	0x1000

jump_to_EL1:
    mov x16, lr

    //We need to go to EL1 
    bl	curr_core_el
    cmp	x0, #1
    beq	1f
3:
    cmp	x0, #3
    bne	2f

    //config SCR_EL3
    mov x1, #0x1	//EL0/1 are non-secure
    orr x1, x1, #0x80	//Disable secure monitor
    orr x1, x1, #0x100	//Enable Hypervisor instructions
    orr x1, x1, #0x400	//Enable AArch64 for lower exception levels
    msr	scr_el3, x1

    //config SPSR_EL3
    mov x1, #0x9	//Drop to EL2 with SP_EL2
    orr x1, x1, #0xC0	//Mask FIQ, IRQ
    orr x1, x1, #0x300	//Mask Debug, SError Exceptions
    msr spsr_el3, x1

    //set return address after returning
    adr x1, 2f
    msr elr_el3, x1

    //move to el2
    eret

2:
    cmp	x0, #2
    bne 1f
   
    //config CNTHCTL_EL2
    mrs x0, cnthctl_el2
    orr x0, x0, #0x1		//allow access to counter registers
    orr x0, x0, #0x2		//allow access to physical timer
    msr cnthctl_el2, x0
    msr cntvoff_el2, xzr	//zero counter offset

    //config HCR_EL2
    mrs x0, hcr_el2
    orr x0, x0, #0x2		//enable SWIO, hardwired for RPI3
    orr x0, x0, #(1<<31) 	//enable AArch64 for lower exception levels
    msr hcr_el2, x0

    //config SCTLR_EL1
    mov	x0, #0x800		//enable instruction cache
    //movk x0, #0x30d0, lsl #16
    mov x0, #(1<<16)		//execute WFI as normal
    orr x0, x0, #(1<<18)	//execute WFE as normal
    orr x0, x0, #(1<<19)	//write sections of memory are not executable
    msr sctlr_el1, x0

    //config SPSR_EL2
    mov x0, #0x5		//set next EL to EL1 and SP to SP_EL1
    orr x0, x0, #0xc0		//set IRQ and FIRQ mask
    orr x0, x0, #0x300		//set Debug exception and SError mask
    msr spsr_el2, x0
    
    //set EL1 stack pointer
    adr x1, 1f
    msr elr_el2, x16

    bl curr_core_id
    bl core_stack_base
    msr sp_el1, x0

    //move to EL1
    eret

save_curr_context:
    stp x19, x20, [x0, #0];
    stp x21, x22, [x0, #16];
    stp x23, x24, [x0, #32];
    stp x25, x26, [x0, #48];
    stp x27, x28, [x0, #64];
    stp x29, x30, [x0, #80];
    mov	x1, sp;
    str x1, [x0, #96];
    ret

resume_from_context:
    ldp x19, x20, [x0, #0];
    ldp x21, x22, [x0, #16];
    ldp x23, x24, [x0, #32];
    ldp x25, x26, [x0, #48];
    ldp x27, x28, [x0, #64];
    ldp x29, x30, [x0, #80];
    ldr x1, [x0, #96];
    mov	sp, x1;
    ret

setup_stack:
    //get core id
    mrs x0, mpidr_el1
    and x0, x0, #0x3
 
    //core 0 does not need offset
    cbz	x0, 1f

    //get offset from base
    ldr x2, =CORE_STACK_SIZE
    mul x2, x0, x2

1:
    //get stack associated to core id
    adrp x1, __core0_stack
    //stack grows from high to low
    sub x1, x1, x2
    mov sp, x1

    //continue execution to main
    //for core 0
    cbnz x0, exec
    ret

exec:
    b __park_and_wait

//x0 = core number
//returns x0 = stack base
core_stack_base:
    //get offset from base
    ldr x2, =CORE_STACK_SIZE
    mul x0, x0, x2

1:
    //get stack associated to core id
    adrp x1, __core0_stack
    sub x0, x1, x0
    ret

curr_core_id:
    mrs x0, mpidr_el1
    and x0, x0, #0x3
    ret

curr_core_el:
    mrs x0, CurrentEL
    lsr x0, x0, #2
    and x0, x0, #0x0F
    ret
