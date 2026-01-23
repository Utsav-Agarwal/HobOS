#ifndef __MMU_CONF
#define __MMU_CONF

#include "../mmu.h"

//NOTE: for any other configurations, define __MMU_CONF. This will result 
//in a compilation error if more than one configurations are supplied.

struct ttbr_cfg ttbr0_el1 = {
	.table_base_addr = 0,
	.asid = 0,
	.skl = 0,
	.cnp = 0,
}; 

struct ttbr_cfg ttbr1_el1 = {
	.table_base_addr = 0,
	.asid = 0,
	.skl = 0,
	.cnp = 0,
}; 

struct tcr_el1_cfg tcr_el1 = {
	.t0_sz = 0,
	.t1_sz = 0,
	.tg0 = 0,
	.tg1 = 0,
	.ips_sz = 0,
};

#endif

