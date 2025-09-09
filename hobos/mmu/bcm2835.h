#ifndef __MMU_CONF
#define __MMU_CONF

#include "../mmu.h"


extern volatile unsigned char __data_start;
extern volatile unsigned char __end;

//TODO: use linker script definitions
#define PAGE_SIZE	4096
#define TABLE_BADDR	(uint64_t)(&__end)
#define TTBR1_OFFSET	PAGE_SIZE

struct ttbr_cfg ttbr0_el1 = {
	.table_base_addr = TABLE_BADDR,
	.asid = 0,
	.skl = 0,
	.cnp = 1,
}; 

struct ttbr_cfg ttbr1_el1 = {
	.table_base_addr =  TABLE_BADDR + TTBR1_OFFSET,
	.asid = 0,
	.skl = 0,
	.cnp = 1,
}; 

struct tcr_el1_cfg tcr_el1 = {
	.t0_sz = 25,	//addresses 2^39 = 512GB
	.t1_sz = 25,	//addresses 2^39 = 512GB
	.tg0 = TG0_GRANULE_SZ_4KB,
	.tg1 = TG1_GRANULE_SZ_4KB,
	.ips_sz = 2,	//TODO: Autodetect
};

const static struct mair_attr at[] = {
	{
		.type = 0b1111,
		.cache = 0b1111,
	},
	{
		.type = 0b0000,
		.cache = 0b0100,
	},
	{
		.type = 0b0100,	
		.cache = 0b0100,
	},
};

struct mair mair_el1 = {
	.attr0 = at[0],
	.attr1 = at[1],		//non cacheable device memory (mmio)
	.attr2 = at[2],			//non-cacheable memory
};

#endif

