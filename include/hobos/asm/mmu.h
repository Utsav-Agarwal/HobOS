/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef ASM_MMU_H
#define ASM_MMU_H

#define INIT_PT_FLAGS		0x0040000000000703
#define INIT_PAGE_FLAGS		0x0040000000000703

#define INIT_TCR_EL1_TTBR0	0x19
#define INIT_TCR_EL1_TTBR1	0x80190000

#define PAGE_SIZE		0x1000
#define VMEM_OFFSET		0xffffff8000000000

.macro m__setup_mair
	mov x0, 0x0040dd	//same as old mmu init
	msr mair_el1, x0
.endm

.macro m__tcr
	mov x0, #INIT_TCR_EL1_TTBR0
	msr tcr_el1, x0
.endm

.macro m__sctlr
	ldr x0, = 0xC00800
	bic x0, x0, #(1 << 1)	//disable alignment check
	bic x0, x0, #(1 << 25)	//LEndian
	orr x0, x0, #0x1	//enable mmu
	msr sctlr_el1, x0
.endm

.macro m__setup_mmu
	m__setup_mair
	m__tcr
	m__sctlr
	isb
	dmb ish
.endm

#endif
