// SPDX-License-Identifier: GPL-2.0-only

#define USR_INIT	(0x1000 * 512)
#define USR_SZ		(0x1000 * 512)

.global jump_to_usr
.type jump_to_usr, %function
jump_to_usr:
	bl usr_init

	mov x0, #USR_INIT
	and x0, x0, #0xFFFFFFFF;
	msr elr_el1, x0

	// place this in EL0 accessible space
	add x0, x0, #USR_SZ
	msr sp_el0, x0

	dmb ish
	isb

	mov x0, #0
	msr spsr_el1, x0

	isb
	eret
