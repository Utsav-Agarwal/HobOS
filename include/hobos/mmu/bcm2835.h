/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __MMU_CONF
#define __MMU_CONF

#include "../mmu.h"

//TODO: use linker script definitions
#define TABLE_BADDR	(unsigned long)(&__end)
#define TTBR1_OFFSET_B	0
#define TTBR1_OFFSET	0
#define MMU_TSZ		25

//default magic value required for rpi3
#define SCTLR_QUIRKS

inline void handle_sctlr_quirks(unsigned long *sctlr)
{
	*sctlr = 0xC00800;
}

static const struct mair_attr at[] = {
	{
		.outer_cache = MAIR_MEM_O_WB_NT(ALLOC, ALLOC),
		.inner_cache = MAIR_MEM_I_WB_NT(ALLOC, ALLOC),
	}, //Device memory
	{
		.outer_cache = MAIR_DEV(0, 0, 1) >> 4,
		.inner_cache = MAIR_DEV(0, 0, 1),
	}, //Uncacheable memory
	{
		.outer_cache = MAIR_DEV(0, 0, 0) >> 4,
		.inner_cache = MAIR_DEV(0, 0, 0),
	},
};

struct mair mair_el1 = {
	.attr0 = at[0],
	.attr1 = at[1],		//non cacheable device memory (mmio)
	.attr2 = at[2],		//non-cacheable memory
};

#endif

