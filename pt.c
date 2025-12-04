#include "hobos/lib/pt_lib.h"
#include "hobos/kstdio.h"

//global list of page tables
//TODO: make size dynamic
struct page_table_desc *global_page_tables[10];
uint8_t pt_ctr = 0;

// for simplicity, we will assume that this OS only cares about
// 4KB pages.
//
// create an entry based on the address given
// TODO: Add support for more granule sizes
static uint64_t pt_entry(uint64_t paddr, uint64_t flags) {
	return (paddr | flags);	
}

//TODO: Add error checking to make sure we dont exceed 512 entries
// take a pt entry value and place it inside a pt
static void place_pt_entry(struct page_table_desc *pt_desc,
		uint64_t pte)
{
	volatile uint64_t *pt = pt_desc->pt;

	pt[pt_desc->pt_len++] = pte;
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
	place_pt_entry(pt_desc, pte);

	//more than one addresses?
	//are they separated enough for another entry?
	if (((end_paddr - start_paddr) < offset) || 
			(start_paddr == end_paddr))
		return start_pte;

	//else
	for (uint64_t i = start_paddr + offset; i <= end_paddr; i += offset) {
		pte = pt_entry(i, flags);
		place_pt_entry(pt_desc, pte);
	}

	return start_pte;	
}

struct page_table_desc *create_pt(uint64_t pt_baddr, uint8_t level)
{

	volatile uint64_t *pt = (uint64_t *) pt_baddr;
	struct page_table_desc *pt_desc; 

	pt_desc = (struct page_table_desc *) &pt[512];
	pt_desc->level = level;
	pt_desc->pt_len = 0;

	pt_desc->pt = pt; 
	pt_desc->pt[0] = 0;
	
	global_page_tables[pt_ctr++] = pt_desc;

	return pt_desc;	
}

//traverse the page table and validate this vaddr range
void validate_pt(uint64_t baddr, uint64_t start, uint64_t end)
{

}

