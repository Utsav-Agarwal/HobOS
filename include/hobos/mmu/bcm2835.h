#ifndef __MMU_CONF
#define __MMU_CONF

#include "../mmu.h"

//TODO: use linker script definitions
#define PAGE_SIZE	KB(4)
#define TABLE_BADDR	(uint64_t)(&__end)
#define TTBR1_OFFSET_B	0
#define TTBR1_OFFSET	0
#define MMU_TSZ		25

//default magic value required for rpi3
#define SCTLR_QUIRKS	

__inline__ void handle_sctlr_quirks(uint64_t *sctlr)
{
	*sctlr = 0xC00800;
}

struct ttbr_cfg ttbr0_el1 = {
	.table_base_addr = TABLE_BADDR,
	.asid = 0,
	.skl = 0,
	.cnp = 1,
}; 

struct ttbr_cfg ttbr1_el1 = {
	.table_base_addr =  TABLE_BADDR + TTBR1_OFFSET_B,
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
		.outer_cache = MAIR_MEM_O_WB_NT(ALLOC,ALLOC), 
		.inner_cache = MAIR_MEM_I_WB_NT(ALLOC,ALLOC),
	}, //Device memory
	{
		.outer_cache = MAIR_DEV(0,0,1) >> 4, 
		.inner_cache = MAIR_DEV(0,0,1),
	}, //Uncacheable memory
	{
		.outer_cache = MAIR_DEV(0,0,0) >> 4,
		.inner_cache = MAIR_DEV(0,0,0),
	},
};

struct mair mair_el1 = {
	.attr0 = at[0],
	.attr1 = at[1],		//non cacheable device memory (mmio)
	.attr2 = at[2],		//non-cacheable memory
};

#endif

