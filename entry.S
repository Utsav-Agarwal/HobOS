// SPDX-License-Identifier: GPL-2.0-only

#define USR_SZ		(0x1000 * 512)

.global jump_to_usr
.type jump_to_usr, %function
jump_to_usr:
	bl usr_init
	bl kernel_splash_msg
	bl get_usr_entry_pt
	and x0, x0, #0xFFFFFFFF;
	msr elr_el1, x0

	// place this in EL0 accessible space
	bl get_usr_end
	msr sp_el0, x0

	dmb ish
	isb

	mov x0, #0
	msr spsr_el1, x0

	isb
	eret
