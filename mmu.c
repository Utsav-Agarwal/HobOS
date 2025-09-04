#include <stdint.h>
#include "hobos/mmu.h"

#include "hobos/mmu/bcm2835.h"
#include "hobos/kstdio.h"

extern uint64_t mmio_base;

static void __set_ttbr(struct ttbr_cfg *cfg, uint8_t index, uint8_t el)
{
	uint64_t ttbr_config;


	ttbr_config = 
		((uint32_t)cfg->table_base_addr & 0x7FFFFFFFFFF);

	ttbr_config |= cfg->cnp < CnP_POS;
	ttbr_config |= cfg->skl < SKL_POS;
	ttbr_config |= cfg->asid < ASID_POS;


	//dont care about addr over 43 bits for now
//	ttbr_config |= 
//		(cfg->table_base_addr & ~0x7FFFFFFFFFF) < BADDR_1_POS;


	kprintf("Config: %x\n", ttbr_config);

	switch (el) {
		case 2:
			__asm__ volatile ("msr ttbr0_el2, %0"
				:
				:"r"(ttbr_config)
				: "memory");
			break;
		case 3:
			__asm__ volatile ("msr ttbr0_el3, %0"
				:
				:"r"(ttbr_config)
				: "memory");
			break;
		default:
			//EL1/0
			if (!index)
				__asm__ volatile ("msr ttbr0_el1, %0"
					:
					:"r"(ttbr_config)
					: "memory");
			else
				__asm__ volatile ("msr ttbr1_el1, %0"
					:
					:"r"(ttbr_config)
					: "memory");
	}
}

static void __set_tcr_el1 (struct tcr_el1_cfg *cfg) {
	uint64_t tcr_config;

	tcr_config = cfg->t0_sz < T0_SZ_POS;
	tcr_config |= cfg->t1_sz < T1_SZ_POS;
	
	tcr_config |= cfg->tg0 < TG0_POS;
	tcr_config |= cfg->tg1 < TG1_POS;

	tcr_config |= cfg->ips_sz < IPS_POS;

	__asm__ volatile ("msr tcr_el1, %x0"
			:
			:"r"(tcr_config));	
}

static void inline __enable_mmu(void) {
	__asm__ volatile (" \
			mrs x0, sctlr_el1; \
			orr x0, x0, #1; \
			msr sctlr_el1, x0; \
			");
}


static inline uint64_t __get_next_table(uint64_t curr_table, const uint32_t pg_sz) 
{
	uint64_t next = curr_table + pg_sz;
	
	return next;
}

static inline uint64_t *__get_table(uint64_t curr_table) 
{
	return (uint64_t *) curr_table;
}

static inline uint64_t __get_table_level(uint64_t lvl_0_baddr, uint8_t lvl, const uint32_t pg_sz) 
{
	int i; 
	uint64_t addr;

	for(i = 0; i<lvl; i++)
		addr = __get_next_table(lvl_0_baddr, pg_sz);

	return addr; 
}


static void __set_translation_tables(uint64_t paddr_table_base) {
	/*
	 * For now, lets work with 4kB only, identity mapped.
	 * Take the input as the base of L1
	 * */

	//4kb granule
	//48-Bit addr
	//9 bits each level addr (to another table)
	//final 12 bits = offset - always consistent (selects memory byte within the 4KB block)

	//we need 3 levels of translation
	//Since we are identity mapping, we dont care about more than
	//one virtual space, so we can just skip level 0 for now
	
	uint64_t *pt_desc = (uint64_t *) paddr_table_base;
	uint64_t vaddr, next_desc = __get_next_table(paddr_table_base, PAGE_SIZE);

	int i=0;

	//we want 3 tables
	
	//Here we set the first entries for each table that point
	//to the start of the next table. This is only needed for 
	//L0/L2 since L3 will point to the physical block
	
	//NOTE: We can ofcourse change it so that it points to L4
	//which can then point to a page directly.

	//skip L0, jump the next desc to L1
	next_desc = __get_next_table(next_desc, PAGE_SIZE);
	for (i=0; i<1; i++) {
		*pt_desc = next_desc
			| PT_AF_ACCESSED	//first table is always marked accessed, else pagefault
			| PT_SH_I		//inner shareable, i.e, let local processor cache this
			| PT_USER		//unpriviledged access allowed
			| PT_INDEX_MEM		//classify normal memory
			| PT_PAGE;		//indexed by pages

		//go to the next prospective table header
		pt_desc = __get_table(next_desc);
		next_desc = __get_next_table(next_desc, PAGE_SIZE);
	}

	//now we want to populate the L2/L3 table with identity mapping
	//
	//L2
	//we are starting with 1st entry, not the 0th
	pt_desc = __get_table(__get_table_level(paddr_table_base, 2, PAGE_SIZE)) + 1;
	for (i=1; i<512; i++) {
		vaddr = (uint64_t) (i << 21);

		*pt_desc = vaddr
			| PT_AF_ACCESSED	//first table is always marked accessed, else pagefault
			| PT_USER		//unpriviledged access allowed
			| PT_BLOCK;		//indexed by block, since this will point to L3
		
		//we only care about bits [29:21] when translating L2
		//to L3. So we need to compare those bits when trying to
		//identify mmio addresses
		if (vaddr >= (mmio_base << 21))
			*pt_desc |= PT_INDEX_DEV | PT_SH_O;	//this memory is shared with a memory mapped dev
		else
			*pt_desc |= PT_INDEX_MEM | PT_SH_I;	//normal memory, cacheable to the processor

	}

	//L3
	//TODO: import data
	uint64_t data = 0/PAGE_SIZE;
	
	pt_desc = __get_table(__get_table_level(paddr_table_base, 3, PAGE_SIZE)) + 1;
	for (i=1; i<512; i++) {
		vaddr = (uint64_t) (i * PAGE_SIZE);	//physical memory addresses

		*pt_desc = vaddr
			| PT_AF_ACCESSED	//first table is always marked accessed, else pagefault
			| PT_SH_I		//inner shareable, i.e, let local processor cache this
			| PT_USER		//unpriviledged access allowed
			| PT_INDEX_MEM		//classify normal memory
			| PT_PAGE;		//indexed by pages

		if (data <= vaddr || data > 0x80)	
			*pt_desc |= PT_AP_RW | PT_UXN_NX;	//we want to separate data/code
		else
			*pt_desc |= PT_AP_RO;	

	}


	//We have just set the user space, now we need to set the kernel
	//For this, we will use the TTBR1 register and map the kernel to 
	//higher addresses
	
	pt_desc = __get_table(__get_table_level(paddr_table_base, 1, PAGE_SIZE)); //we want to add an entry for L0
	next_desc = __get_table_level(paddr_table_base, 2*2, PAGE_SIZE);	//we want to jump to L2 for ttbr1
										
	for (i=0; i<2; i++) {
		if (i == 2)
			next_desc = (mmio_base + 0x00201000);		//TODO: remove hardcoded address

		*pt_desc = next_desc
			| PT_AF_ACCESSED	//first table is always marked accessed, else pagefault
			| PT_SH_I		//inner shareable, i.e, let local processor cache this
			| PT_USER		//unpriviledged access allowed
			| PT_INDEX_MEM		//classify normal memory
			| PT_PAGE;		//indexed by pages

		//go to the next prospective table header
		pt_desc = __get_table(next_desc);
		next_desc = __get_next_table(next_desc, PAGE_SIZE);
	}

	




}

//TODO: Take an argument here instead of a static initialization
void init_mmu(void) {

	__set_translation_tables(TABLE_BADDR);


	//TODO: Need to skip L0
	__set_ttbr(&ttbr0_el1, 0, 1);	
	__set_ttbr(&ttbr1_el1, 1, 1);	
	__set_tcr_el1(&tcr_el1);
	__asm__ volatile ("isb");
	
	__enable_mmu();	
	__asm__ volatile ("isb");

	return;
}
