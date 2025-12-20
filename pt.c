#include "hobos/kstdio.h"
#include "hobos/lib/pt_lib.h"
#include "hobos/asm/barrier.h"

//global list of page tables
//TODO: make size dynamic
struct page_table_desc *global_page_tables[10];
uint8_t pt_ctr = 0;

// for simplicity, we will assume that this OS only cares about
// 4KB pages.
//
// create an entry based on the address given
// TODO: Add support for more granule sizes
uint64_t pt_entry(uint64_t paddr, uint64_t flags) {
	return (paddr | flags);	
}

//TODO: Add error checking to make sure we dont exceed 512 entries
//TODO: support to be added for specific indexes
//TODO: for malloc entries, assign a different block and mark it
//TODO: device memories should be in a separate block/page table
//		-- for now we dont really care about granularity about 
//		permissions for io, so we can just allocate a big block for
//		io based memory ops
void place_pt_entry(struct page_table_desc *pt_desc, uint64_t pte, int index)
{
	volatile uint64_t *pt = pt_desc->pt;

	if (index == -1) {
		dmb(ish);
		pt[pt_desc->pt_len++] = pte;
		return;
	} 

	pt[index] = pte;
	pt_desc->pt_len = index;
}

//for a given page table, create required entries
//just use generic flags for now
//
//NOTE: this works for both tables and page entries
//
//NOTE: start and end paddr may also refer to other page tables,
//based on what.
//
//TODO: Add failure conditions
//
//
//returns: the first new page table entry created
uint64_t *create_pt_entries(
		struct page_table_desc *pt_desc,
		uint64_t start_paddr, uint64_t end_paddr, uint64_t flags)
{
	uint64_t *pt = pt_desc->pt;
	uint64_t *start_pte, pte;
	uint64_t offset = KB(4);	//should be separated by 1 page atleast

	pte = pt_entry(start_paddr, flags);
	start_pte = &pt[pt_desc->pt_len];
	place_pt_entry(pt_desc, pte, -1);

	//more than one addresses?
	//are they separated enough for another entry?
	if (((end_paddr - start_paddr) < offset) || 
			(start_paddr == end_paddr))
		return start_pte;

	//else
	for (uint64_t i = start_paddr + offset; i <= end_paddr; i += offset) {
		pte = pt_entry(i, flags);
		place_pt_entry(pt_desc, pte, -1);
	}

	return start_pte;	
}

//if 0 is provided, new address should be given automatically
//NOTE: first time usage MUST provide a base address
struct page_table_desc *create_pt(uint64_t pt_baddr, uint8_t level)
{

	struct page_table_desc *pt_desc; 
	volatile uint64_t *pt;

	if (pt_baddr != 0) {
		dmb(ish);
		pt = (uint64_t *) pt_baddr; 
	} else {
		//if no arg, just move to the next available space
		dmb(ish);
		pt = (uint64_t *) 
			((uint64_t) global_page_tables[pt_ctr-1]->pt + 
			 NEXT_PT_OFFSET);
	}

	memset(pt, 0, 0x1000);	//512 entries * 8 B
	pt_desc = (struct page_table_desc *) &pt[512];

	wmb();
	pt_desc->pt = pt;
	pt_desc->level = level;
	pt_desc->pt_len = 0;
	pt_desc->pt[0] = 0;
	global_page_tables[pt_ctr++] = pt_desc;

	return pt_desc;	
}

void reserve_block(uint64_t baddr)
{
	void *addr;
	uint16_t i = 0;

	for (i=0; i<512; i++)
		addr = ioremap(baddr + 0x1000*i);
}

//traverse the page table and validate this vaddr range
void validate_pt(uint64_t baddr, uint64_t start, uint64_t end)
{

}

