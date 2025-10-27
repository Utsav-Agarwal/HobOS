#include <stdint.h>
#include "hobos/mmu.h"

#include "hobos/mmu/bcm2835.h"
#include "hobos/kstdio.h"
#include "hobos/uart.h"

#define KB(x)	x * (1 << 10)
#define map_sz	512
#define ID_PG_SZ   KB(4)

#define KERNEL_START	0xFFFFFF8000000000

static uint64_t *id_map_end;

// lets map 2 GB
uint64_t set_id_translation_table() 
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
	map[0] = ((uint64_t) (uint8_t *)&__end + ID_PG_SZ) | mem_attr;

	//L2 (8B * 512 = 4096B)
	map[512] = ((uint64_t) (uint8_t *)&__end + 2*ID_PG_SZ) | mem_attr;

	//L3 identity
	for (i=0; i<512; i++) {
		map[2*512 + i] = (uint64_t) i*ID_PG_SZ | mem_attr;
		id_map_end = &map[2*512 + i];
	}

	return (uint64_t) &map[0];
}

uint64_t set_kernel_translation_table()
{
	//we can just reuse the id_map as we only
	//really care about it being mapped to high memory

	volatile uint64_t *map = (uint64_t *) &__end;

	return (uint64_t) &map[0];
}

void set_ttbr1_el1(uint64_t x) {
	asm("msr ttbr1_el1, %0"::"r"(x));
}

void set_ttbr0_el1(uint64_t x) {
	asm("msr ttbr0_el1, %0"::"r"(x));
}

uint64_t switch_vmem(void)
{
	uint64_t tcr, reg;
	
	set_ttbr1_el1((uint64_t) set_kernel_translation_table());
	
	asm("mrs %0, tcr_el1":"=r"(tcr));

	tcr |= TG1_GRANULE_SZ_4KB << TG1_POS;
	tcr |= 25 << T1SZ_POS;
	tcr |= EPD_WALK << EPD1_POS;

	asm("msr tcr_el1, %0"::"r"(tcr));
	

	//at this point the table should be active, so in theory
	//we should be able to just set the next instruction
	
	asm("mov %0, sp":"=r"(reg));
	reg += KERNEL_START;
	asm("mov sp, %0"::"r"(reg));


	asm("mov %0, lr":"=r"(reg));
	reg += KERNEL_START;
	asm("mov lr, %0"::"r"(reg));

	//remove access
	//set_ttbr0_el1(0);
	//asm("tlbi vmalle1; dsb ish; isb");
}

void create_phy_table(uint64_t *map, uint64_t baddr, uint32_t nr_entries, 
		uint8_t pg_sz_kb, uint64_t mem_attr) 
{

	uint32_t i; 
	for(i=0; i<nr_entries; i++)
		map[i] = baddr + i*KB(pg_sz_kb) | mem_attr;

}

uint64_t create_pg_tables(uint64_t baddr, uint8_t size, uint8_t pg_sz_kb)
{
	uint64_t nr_phy_pages, nr_tables;
	uint64_t *map = id_map_end + ID_PG_SZ;	//leave some space
	int i=0;

	uint64_t mem_attr =  PT_PAGE 
			| PT_AP_RW 
			| PT_AF_ACCESSED 
			| PT_UXN_NX
			| PT_SH_O
			| PT_INDEX_MEM;
	
	//calculate number of pages required
	nr_phy_pages = size/KB(pg_sz_kb);
	nr_tables = nr_phy_pages/512;	//each table can have 512 entries
	
	//TODO: Add support for dynamic multi level pg table support
	//for now, lets assume its only ever going to be requiring one
	//table for the level before
	

	//1st level
	map[0] = ((uint64_t) ((uint8_t *)&map[0] + ID_PG_SZ)) | mem_attr;
	//2nd level
	map[512] = ((uint64_t) ((uint8_t *)&map[0] + 2*ID_PG_SZ)) | mem_attr;
	//3rd level
	create_phy_table(&map[512*2], baddr, nr_phy_pages, 
				pg_sz_kb, mem_attr);
	

	return (uint64_t) map;

}

inline void set_tcr_ttbr0_config(struct t_cfg *t0)
{
	uint64_t tcr;

	asm("mrs %0, tcr_el1":"=r"(tcr));

	tcr |= t0->tsz 	<< T0SZ_POS;
	tcr |= t0->tgsz << TG0_POS;
	tcr |= t0->epd 	<< EPD0_POS;
	tcr |= t0->irgn << IRGN0_POS;
	tcr |= t0->orgn << ORGN0_POS;
	tcr |= t0->sh 	<< SH0_POS;

	asm("msr tcr_el1, %0"::"r"(tcr));
}

//we will use ttbr0_el1 for all ioremaps/kernel thread contexts/etc.
//For now, lets keep all context mappings as simply identity maps
void set_new_context(uint64_t baddr, uint32_t size, uint8_t pg_sz_kb)
{
	uint64_t pg_baddr, tcr = 0;
	struct t_cfg t0;

	//we dont want a fixed granule size since different
	//threads may require different handling
	
	switch (size) {
		case 4:
			t0.tgsz = TG0_GRANULE_SZ_4KB;
			break;
		case 16:
			t0.tgsz = TG0_GRANULE_SZ_16KB;
			break;
		case 64:
			t0.tgsz = TG0_GRANULE_SZ_64KB;
			break;

	}

	t0.irgn = 0;
	t0.orgn = 0;
	t0.sh	= 0;
	t0.epd	= EPD_WALK;
	t0.tsz 	= 25;	//TODO: make this dynamic

	set_tcr_ttbr0_config(&t0);
	set_ttbr0_el1(create_pg_tables(baddr, size, pg_sz_kb));

}

void init_mmu(void) 
{
	uint64_t tcr, sctlr, spsr;

	//page table set
	set_ttbr0_el1(set_id_translation_table() + CNP_COMMON);
	asm("msr mair_el1, %0"::"r"(at));

	//translation control TCR_EL1

	tcr = 25 << T0SZ_POS;
	tcr |= TG0_GRANULE_SZ_4KB << TG0_POS;
	tcr |= EPD_WALK << EPD0_POS;
	tcr |= 0b00 << IRGN0_POS;
	tcr |= 0b00 << ORGN0_POS;
	tcr |= 0b00 << SH0_POS;

	asm("msr tcr_el1, %0"::"r"(tcr));

	//system control SCTLR_EL1

	asm("mrs %0, sctlr_el1":"=r"(sctlr));
	
#ifdef SCTLR_QUIRKS
	handle_sctlr_quirks(&sctlr);
#endif
	sctlr &= ~(1 << 1); //remove alignment check
	sctlr &= ~(1 << 25); //make sure little endian
	

	//enable mmu
	sctlr |= 1;
	
	asm("msr sctlr_el1, %0"::"r"(sctlr));
	asm("isb; dsb sy");

	return;
}
