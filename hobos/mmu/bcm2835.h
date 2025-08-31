#ifndef __MMU_CONF
#define __MMU_CONF

#include "../mmu.h"

#define TTBR1_OFFSET	0x1000

extern uint64_t __end;

struct ttbr_cfg ttbr0_el1 = {
	.table_base_addr = (uint64_t) &__end,
	.asid = 0,
	.skl = 0,
	.cnp = 0,
}; 

struct ttbr_cfg ttbr1_el1 = {
	.table_base_addr = ((uint64_t) &__end) + TTBR1_OFFSET,
	.asid = 0,
	.skl = 0,
	.cnp = 0,
}; 

struct tcr_el1_cfg tcr_el1 = {
	.t0_sz = 0,
	.t1_sz = 0,
	.tg0 = TG0_GRANULE_SZ_4KB,
	.tg1 = TG1_GRANULE_SZ_4KB,
	.ips_sz = 0,
};

#endif

