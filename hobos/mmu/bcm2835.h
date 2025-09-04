#ifndef __MMU_CONF
#define __MMU_CONF

#include "../mmu.h"

//TODO: use linker script definitions
#define TABLE_BADDR	0x90000
#define TTBR1_OFFSET	0x1000
#define PAGE_SIZE	4096

struct ttbr_cfg ttbr0_el1 = {
	.table_base_addr = TABLE_BADDR,
	.asid = 0,
	.skl = 0,
	.cnp = 1,
}; 

struct ttbr_cfg ttbr1_el1 = {
	.table_base_addr = TABLE_BADDR + TTBR1_OFFSET,
	.asid = 0,
	.skl = 0,
	.cnp = 1,
}; 

struct tcr_el1_cfg tcr_el1 = {
	.t0_sz = 16,
	.t1_sz = 16,
	.tg0 = TG0_GRANULE_SZ_4KB,
	.tg1 = TG1_GRANULE_SZ_4KB,
	.ips_sz = 5,	//TODO: Autodetect
};

#endif

