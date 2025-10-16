#include <stdint.h>
#include "hobos/mmu.h"

#include "hobos/mmu/bcm2835.h"
#include "hobos/kstdio.h"
#include "hobos/uart.h"

#define KB(x)	x * (1 << 10)
#define map_sz	512
#define PG_SZ   KB(4)

// lets map 2 GB
uint64_t set_translation_table() 
{
	int i;
	// this is still baremetal, so we dont care
	volatile uint64_t *map = (uint64_t *) &__end;
	uint64_t mem_attr =  PT_PAGE 
			| PT_AP_RW 
			| PT_AF_ACCESSED 
			| PT_UXN_NX
			| PT_SH_O
			| PT_INDEX_MEM;

	//L1
	map[0] = ((uint64_t) (uint8_t *)&__end + PG_SZ) | mem_attr;

	//L2 (8B * 512 = 4096B)
	map[512] = ((uint64_t) (uint8_t *)&__end + 2*PG_SZ) | mem_attr;

	//L3 identity
	for (i=0; i<512; i++)
		map[2*512 + i] = (uint64_t) i*PG_SZ | mem_attr;

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
	int tcr, sctlr, spsr;

	//page table set
	set_ttbr0_el1(set_translation_table() + CNP_COMMON);
	asm("msr mair_el1, %0"::"r"(at));

	//translation control TCR_EL1

	tcr = 25;
	tcr |= TG0_GRANULE_SZ_4KB << TG0_POS;
	tcr |= EPD_WALK << EPD0_POS;
	tcr |= 0b00 << IRGN0_POS;
	tcr |= 0b00 << ORGN0_POS;
	tcr |= 0b00 << SH0_POS;

	asm("msr tcr_el1, %0"::"r"(tcr));

	//system control SCTLR_EL1

	asm("mrs %0, sctlr_el1":"=r"(sctlr));
	
	sctlr = 0xC00800;
	sctlr &= ~(1 << 1); //remove alignment check
	sctlr &= ~(1 << 25); //make sure little endian

	//enable mmu
	sctlr |= 1;
	
	asm("msr sctlr_el1, %0"::"r"(sctlr));
	asm("isb; dsb sy");

	return;
}
