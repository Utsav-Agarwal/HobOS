/* SPDX-License-Identifier: GPL-2.0-only */

.section ".text.boot"

#include <hobos/asm/assembler.h>
#include <hobos/asm/mmu.h>
#include <hobos/asm/pt.h>

.macro m__prep_ttbr1_el1
	mrs x0, ttbr0_el1
	msr ttbr1_el1, x0

	mrs x0, tcr_el1
	ldr x1, =INIT_TCR_EL1_TTBR1
	orr x0, x0, x1
	msr tcr_el1, x0

	tlbi vmalle1
	isb
	dmb sy

	ldr x1, =VMEM_OFFSET
	mov x0, sp
	add x0, x0, x1
	mov sp, x0

	isb
	dmb sy
.endm

.macro m__jump_to_EL1
	mov x16, lr

	//We need to go to EL1
	m__curr_core_el
	cmp	x0, #1
	beq	el1
el3:
	cmp	x0, #3
	bne	el2

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
	adr x1, el2
	msr elr_el3, x1

	//move to el2
	eret

el2:
	m__curr_core_el
	cmp	x0, #2
	bne el1

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
	adr x1, el1
	msr elr_el2, x1

	adrp x0, __core0_stack
	msr sp_el1, x0
	//move to EL1
	eret
el1:
.endm

.macro m__lowlevel_init
	//assign stack
	adrp x0, __core0_stack
	mov sp, x0

	m__jump_to_EL1

	//set up initial page tables with id mapping
	//we are going to replace this, so we just need it until
	//we can reach a point where we can create a new one in C
	adrp x0, __kernel_pt	//x0 at L1[0]
	//add x0, x0, #1		//share with other cores
	msr ttbr0_el1, x0

	add x2, x0, #PT_SIZE
	ldr x1, =INIT_PT_FLAGS
	orr x1, x1, x2
	str x1, [x0]			//L1

	add x0, x0, #PT_SIZE		//x0 now at L2[0]

	add x2, x0, #PT_SIZE		//L2
	ldr x1, =INIT_PT_FLAGS
	orr x1, x1, x2
	str x1, [x0]

	add x0, x0, #PT_SIZE		//x0 now at L3[0] - page level

	m__populate_pt_pages

	//setup mmu
	m__setup_mmu

	//vmem switch
	m__prep_ttbr1_el1
	bl switch_to_highmem
.endm

.global _start
_start:
	m__lowlevel_init

	// clear bss
	ldr     x1, =__bss_start
	ldr     w2, =BSS_SIZE
3:	cbz     w2, 4f
	str     xzr, [x1], #8
	sub     w2, w2, #1
	cbnz    w2, 3b
4:
	adr	x0, vector_table
	msr vbar_el1, x0
	isb

	// jump to C code, should not return
5: 	bl      main
	// for failsafe, halt this core too
	b       5b

switch_to_highmem:
	ldr x0, =VMEM_OFFSET
	orr lr, lr, x0
	ret

.align 11
sync_exception_handler:
	bl kernel_panic
	eret

.macro m__ventry label
	.align 7
	b \label
.endm

dummy_vector:
	wfe
	b dummy_vector

/* We need to preserve caller program order */
.macro m__save_fplr
	sub sp, sp, #8*2
	stp x29, x30, [sp]
.endm

.macro m__restore_fplr
	ldp x29, x30, [sp]
	add sp, sp, #8*2
.endm

handle_el0_svc:
	m__save_fplr
	bl disable_global_interrupts
	bl syscall_el0_common
	bl enable_global_interrupts
	m__restore_fplr
	eret

.align 11
vector_table:
	m__ventry sync_exception_handler
	m__ventry dummy_vector
	m__ventry dummy_vector
	m__ventry dummy_vector

	//EL1h
	m__ventry dummy_vector
	m__ventry handle_irq
	m__ventry dummy_vector
	m__ventry dummy_vector

	m__ventry handle_el0_svc
