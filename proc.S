.section ".text.proc"

.global setup_stack
.global save_curr_context
.global resume_from_context
.global curr_core_id
.global curr_core_el
.global core_stack_base
.global jump_to_EL1
.global enable_global_interrupts
.global handle_irq

#define CORE_STACK_SIZE		0x1000

enable_global_interrupts:
    msr daifclr, #2
    dmb osh
    ret

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

//push all the registers just after the stack pointer
.macro save_curr_context
    //reserving 16*16 bytes of stack
    sub sp, sp, #16*16	//30 regs, 8 bytes each
    stp x0, x1, [sp, #16*0]
    stp x2, x3, [sp, #16*1]
    stp x4, x5, [sp, #16*2]
    stp x6, x7, [sp, #16*3]
    stp x8, x9, [sp, #16*4]
    stp x10, x11, [sp, #16*5]
    stp x12, x13, [sp, #16*6]
    stp x14, x15, [sp, #16*7]
    stp x16, x17, [sp, #16*8]
    stp x18, x19, [sp, #16*9]
    stp x20, x21, [sp, #16*10]
    stp x22, x23, [sp, #16*11]
    stp x24, x25, [sp, #16*12]
    stp x26, x27, [sp, #16*13]
    stp x28, x29, [sp, #16*14]
    str x30, [sp, #16*15]
.endm

//undo previous operation and start restoring
.macro resume_from_context
    ldp x0, x1, [sp, #16*0]
    ldp x2, x3, [sp, #16*1]
    ldp x4, x5, [sp, #16*2]
    ldp x6, x7, [sp, #16*3]
    ldp x8, x9, [sp, #16*4]
    ldp x10, x11, [sp, #16*5]
    ldp x12, x13, [sp, #16*6]
    ldp x14, x15, [sp, #16*7]
    ldp x16, x17, [sp, #16*8]
    ldp x18, x19, [sp, #16*9]
    ldp x20, x21, [sp, #16*10]
    ldp x22, x23, [sp, #16*11]
    ldp x24, x25, [sp, #16*12]
    ldp x26, x27, [sp, #16*13]
    ldp x28, x29, [sp, #16*14]
    ldr x30, [sp, #16*15]
    add sp, sp, #16*16	//reclaim stack
.endm

handle_irq:
    save_curr_context
    bl kernel_test
    resume_from_context
    eret

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
