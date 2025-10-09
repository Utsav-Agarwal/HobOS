#include <stdint.h>
#include "hobos/mmu.h"

#include "hobos/mmu/bcm2835.h"
#include "hobos/kstdio.h"
#include "hobos/uart.h"

#define KB(x)	x * (1 << 10)
#define PG_START	0x1000

uint64_t set_translation_table() 
{
	int i;
	// this is still baremetal, so we dont care
	uint64_t *map = (uint64_t *)&__end;
	uint64_t map_sz = 64;

	uint64_t paddr = 0x0;
	uint64_t blk_size = KB(4);
	uint64_t attr = PT_KERNEL 
			| PT_BLOCK 
			| PT_AP_RW 
			| PT_AF_ACCESSED 
			| PT_UXN_NX
			| PT_SH_O
			| PT_INDEX_MEM;

	for(i=0; i<map_sz; i++) {
		map[i] = paddr | attr;
		paddr += blk_size;
	}
	
	return (uint64_t) &__end;
}

void set_ttbr1_el1(uint64_t x) {
	asm("msr ttbr1_el1, %0"::"r"(x));
}

void init_mmu(void) 
{
	uint64_t baddr = set_translation_table();
	set_ttbr1_el1(baddr);

	return;
}
