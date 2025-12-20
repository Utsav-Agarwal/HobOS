#include "hobos/asm/barrier.h"
#include "hobos/mmu.h"

#include "hobos/mmu/bcm2835.h"
#include "hobos/kstdio.h"
#include "hobos/uart.h"

#define map_sz	512
#define ID_PG_SZ   KB(4)

extern struct page_table_desc *global_page_tables[10];
extern uint8_t pt_ctr;

//create a table just for identity mapping
void create_id_mapping (uint64_t start_paddr, uint64_t end_paddr, 
		uint64_t pt)
{	

	//for each entry, use the mask to check if something is 
	//needed at this level, if not, move to next and keep moving
	//till the final paddr is reached.

	//for now lets assume T0/1_SZ is constant at 25, so we
	//only care about 3 levels
	uint16_t flags = PTE_FLAGS_GENERIC;
	uint64_t end_addr; 
	struct page_table_desc *pt_desc;

	//SKIP L0 - dont need it

	//L1 - point to L2
	pt_desc = create_pt(pt, 1);
	end_addr = (uint64_t)(pt_desc->pt) + NEXT_PT_OFFSET; //next
	create_pt_entries(pt_desc, end_addr, end_addr, flags);

	//L2 - point to L3
	pt_desc = create_pt(end_addr, 2);
	end_addr += NEXT_PT_OFFSET;	//next
	create_pt_entries(pt_desc, end_addr, end_addr, flags);

	//L3 - physical pages
	pt_desc = create_pt(end_addr, 3);
	create_pt_entries(pt_desc, start_paddr, end_paddr, flags); //2GB
		
}

static inline void extract_va_metadata(uint64_t va, struct va_metadata *meta)
{
	uint64_t mask = 0xFFF;

	meta->offset = va & mask;	//offset is calculated with last
					//12 bits
	
	mask = ((1 << 9) - 1);		//9 bits bitmask
	
	meta->index[0] = (va >> 30) & mask;
	meta->index[1] = (va >> 21) & mask;
	meta->index[2] = (va >> 12) & mask;
}

static inline uint64_t pte_is_empty(uint64_t pte)
{
	return !(pte);
}

struct va_metadata meta;
void map_pa_to_va_pg(uint64_t pa, uint64_t va, struct page_table_desc *pt_top, 
		uint64_t flags)
{
	//take va
	//walk the pt(s)
	//reach block/page level
	//create entry
	
	struct page_table_desc *pt_desc = pt_top;
	uint64_t volatile *pt, pte;
	uint16_t pt_index;
	uint8_t level, i;
	
	extract_va_metadata(va, &meta);	
	meta.offset += pa;		//we want physical address at the end

	//lets navigate to l3 and create entries as required
	for (i=pt_desc->level; i<=PT_LVL_MAX; i++) {

		level = pt_desc->level;
		pt = pt_desc->pt;
		pt_index = meta.index[level - 1];	//indexing

		//L3
		if (i == PT_LVL_MAX) {
			//create entry
			if (!flags)
				pt[pt_index] = pt_entry(meta.offset,
						PTE_FLAGS_GENERIC);
			else
				pt[pt_index] = pt_entry(meta.offset, flags);

			break;
		}

		pte = pt[pt_index];
		//TODO: if 0 is provided, new address given automatically
		if (pte_is_empty(pte)) {

			struct page_table_desc *new_pt_desc;
			
			//we create next level
			new_pt_desc = create_pt(0, level + 1);	
			pte = pt_entry((uint64_t )new_pt_desc->pt, 
					PTE_FLAGS_GENERIC);
			place_pt_entry(pt_desc, pte, pt_index);
			pt_desc = new_pt_desc;
		} else {
			//todo: extract pt desc from pt addr dynamically
			//essentially, clean the entry and extract the address,
			//then examine the pt and populate the struct
			//pt_desc = get_pt_desc(pte);
			
			//for now just use the global table
			//and go to next in arr
			
			pt_desc = (struct page_table_desc *) 
				((pte & ~(0xF0000000000FFF)) + 0x1000);

			//pt_desc = global_page_tables[level];	
		}

	}

}

uint64_t set_id_translation_table() 
{

	create_id_mapping(0, 0x1000 * 512, (uint64_t) &__end);
	return (uint64_t) (global_page_tables[0]->pt);
}

uint64_t set_kernel_translation_table()
{
	//we can just reuse the id_map as we only
	//really care about it being mapped to high memory

	int i;
	volatile uint64_t *map = (uint64_t *) &__end;
	uint64_t mem_attr =  PT_BLOCK 
			| PT_AP_RW 
			| PT_AF_ACCESSED 
			| PT_SH_O
			| PT_INDEX_DEV;

	//we do want to map the peripherals to certain memory blocks
	//for now lets just do mini uart
	map[512 + 1] = (uint64_t) ((uint8_t *)AUX_IO_BASE + ID_PG_SZ); 
	
	return (uint64_t) &map[0];
}

void set_ttbr1_el1(uint64_t x) {
	asm("msr ttbr1_el1, %0"::"r"(x));
}

void set_ttbr0_el1(uint64_t x) {
	asm("msr ttbr0_el1, %0"::"r"(x));
}

#define KERNEL_START	0xFFFFFF8000000000

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
	
	reg = ((uint64_t) &__core0_stack) + KERNEL_START;
	asm("mov sp, %0"::"r"(reg));

	asm("mov %0, lr":"=r"(reg));
	reg += KERNEL_START;
	asm("mov lr, %0"::"r"(reg));
	
	asm("tlbi vmalle1; dsb sy; isb");

}

void init_mmu(void) 
{
	uint64_t tcr, sctlr, spsr;

	//page table set
	set_ttbr0_el1(set_id_translation_table() + CNP_COMMON);
	asm("msr mair_el1, %0"::"r"(mair_el1));

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
	asm("dsb sy; isb");

	//invalidate tlb
	asm("tlbi vmalle1; dsb sy; isb");

	return;
}
