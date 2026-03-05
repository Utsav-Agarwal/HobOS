/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#define CORE_STACK_SIZE		0x1000

.macro m__curr_core_el
	mrs x0, CurrentEL
	lsr x0, x0, #2
	and x0, x0, #0x0F
.endm

.macro m__curr_core_id
	mrs x0, mpidr_el1
	and x0, x0, #0x3
.endm

#endif
