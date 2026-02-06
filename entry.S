// SPDX-License-Identifier: GPL-2.0-only

#define USR_INIT	(0x1000 * 512)

.global jump_to_usr
.type jump_to_usr, %function
jump_to_usr:
	dmb osh
	isb
	mov x0, #USR_INIT
	msr sp_el0, x0
	add x0, x0, 0x1000
	and x0, x0, #0xFFFFFFFF;
	msr elr_el1, x0
	eret
