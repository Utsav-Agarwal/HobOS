#include <stdint.h>
#include "hobos/mmu.h"

#include "hobos/mmu/bcm2835.h"
#include "hobos/kstdio.h"
#include "hobos/uart.h"

#define KB(x)	x * (1 << 10)

#define map_sz	512

// lets map 2 GB
uint64_t set_translation_table() 
{
	int i;
	// this is still baremetal, so we dont care
	uint64_t map[64] __attribute__ ((__aligned__(512)));

	uint64_t paddr = 0x0;
	uint64_t blk_size = 0x40000000000ull;
	uint64_t attr =  PT_BLOCK 
			| PT_AP_RW 
			| PT_AF_ACCESSED 
			| PT_UXN_NX
			| PT_SH_O
			| PT_INDEX_MEM;

	for(i=0; i<sizeof(map)/sizeof(map[0]); i++) {
		map[i] = paddr | attr;
		paddr += blk_size;
	}

	return (uint64_t) &map[0];
}

void set_ttbr1_el1(uint64_t x) {
	asm("msr ttbr1_el1, %0"::"r"(x));
}

void set_ttbr0_el1(uint64_t x) {
	asm("msr ttbr0_el1, %0"::"r"(x));
}

void init_mmu(void) 
{
	//page table set
	set_ttbr0_el1(set_translation_table());

	//mair
	struct mair attr = {
		{.type = 0xf, .cache = 0xf},
		{.type = 0x4, .cache = 0x4},
		{.type = 0x0, .cache = 0x0},
		{.type = 0x0, .cache = 0x4},
	};
	asm("msr mair_el1, %0"::"r"(attr));

	//translation control TCR_EL1
	uint64_t tcr  = 0;

	asm("mrs %0, tcr_el1":"=r"(tcr));

	tcr |= 16;
	tcr |= TG0_GRANULE_SZ_64KB << TG0_POS;
	tcr |= EPD_WALK << EPD0_POS;
	tcr |= 0b00 << IRGN0_POS;
	tcr |= 0b00 << ORGN0_POS;
	tcr |= 0b00 << SH0_POS;

	asm("msr tcr_el1, %0"::"r"(tcr));

	//system control SCTLR_EL1
	uint64_t sctlr = 0;

	asm("mrs %0, sctlr_el1":"=r"(sctlr));
	
	sctlr |= 0xC00800;
	sctlr &= ~(1 << 1); //remove alignment check
	sctlr &= ~(1 << 25); //make sure little endian

	//enable mmu
	sctlr |= 1;
	
	asm("msr sctlr_el1, %0"::"r"(sctlr));
	asm("isb; dsb sy");

	return;
}
