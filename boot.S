.section ".text.boot"

.global _start

_start:

    bl setup_stack

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =BSS_SIZE
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b
    
4: 
    
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
    //cmp	x0, #2
    //bne 1f
   
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
    msr elr_el2, x1

    bl curr_core_id
    bl core_stack_base
    msr sp_el1, x0

    //move to EL1
    eret

1:
    mov sp, x0
    adr	x0, vector_table 
    msr vbar_el1, x0
    isb


    // jump to C code, should not return
5:  bl      main
    // for failsafe, halt this core too
    b       1b


.align 11
vector_table:
	b sync_exception_handler
	b 0x200
	b 0x200

.align 11
sync_exception_handler:
	bl kernel_panic
	eret
