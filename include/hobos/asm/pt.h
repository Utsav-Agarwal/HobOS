/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef ASM_PT_H
#define ASM_PT_H

#include <hobos/asm/mmu.h>

#define PT_SIZE			0x1000
#define PTE_SIZE		8

//assume x0 contains addr
.macro m__populate_pt_pages
	mov x2, #0		//first address to be mapped
	mov x5, #0x1f0000	//final expected address

next_pte :
	ldr x1, = INIT_PAGE_FLAGS	//pte flags
	orr x1, x1, x2			//page pa
	str x1, [x0]			//write entry
	add x0, x0, #PTE_SIZE		//next entry

	add x2, x2, #PAGE_SIZE
	cmp x2, x5

	ble next_pte		//if out of range, stop

	dmb ish
.endm

.macro m__va_to_pa label
	adr x0, \label
	bic x0, x0, VMEM_OFFSET
.endm

.macro m__jmp_pa_from_va label
	m__va_to_pa \label
	bic x0, x0, VMEM_OFFSET

	blr x0
.endm

#endif
