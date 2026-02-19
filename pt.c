// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/kstdio.h>
#include <hobos/lib/pt_lib.h>
#include <hobos/asm/barrier.h>
#include <hobos/mmu.h>
#include <hobos/compiler_types.h>

//global list of page tables
//TODO: make size dynamic
struct page_table_desc *global_page_tables;
u8 pt_ctr;

// for simplicity, we will assume that this OS only cares about
// 4KB pages.
//
// create an entry based on the address given
// TODO: Add support for more granule sizes
u64 pt_entry(u64 paddr, u64 flags)
{
	return (paddr | flags);
}

//TODO: Add error checking to make sure we dont exceed 512 entries
//TODO: support to be added for specific indexes
//TODO: for malloc entries, assign a different block and mark it
//TODO: device memories should be in a separate block/page table
//		-- for now we dont really care about granularity about
//		permissions for io, so we can just allocate a big block for
//		io based memory ops
void place_pt_entry(struct page_table_desc *pt_desc, u64 pte, int index)
{
	u64 *pt = pt_desc->pt;

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
u64 *create_pt_entries(struct page_table_desc *pt_desc,
		       u64 start_paddr, u64 end_paddr,
		       u64 flags)
{
	u64 *pt = pt_desc->pt;
	u64 *start_pte;
	u64 pte;
	u64 offset = KB(4);	//should be separated by 1 page atleast

	pte = pt_entry(start_paddr, flags);
	start_pte = &pt[pt_desc->pt_len];
	place_pt_entry(pt_desc, pte, -1);

	//more than one addresses?
	//are they separated enough for another entry?
	if (((end_paddr - start_paddr) < offset) ||
	    start_paddr == end_paddr)
		return start_pte;

	//else
	for (u64 i = start_paddr + offset; i <= end_paddr; i += offset) {
		pte = pt_entry(i, flags);
		place_pt_entry(pt_desc, pte, -1);
	}

	return start_pte;
}

//if 0 is provided, new address should be given automatically
//NOTE: first time usage MUST provide a base address
struct page_table_desc *create_pt(u64 pt_baddr, char level)
{
	struct page_table_desc *pt_desc;
	u64 *pt;

	if (pt_baddr != 0) {
		dmb(ish);
		pt = (u64 *)pt_baddr;
	} else {
		//if no arg, just move to the next available space
		dmb(ish);
		pt = kmalloc(0x1000);
		if (!pt)
			return 0;
	}

	if (!global_page_tables) {
		global_page_tables = kmalloc(0x1000);
		//memset(global_page_tables, 0, 0x1000);
	}

	pt_desc = &global_page_tables[pt_ctr++];
	pt_desc->pt = pt;
	pt_desc->level = level;
	pt_desc->pt_len = 0;
	pt_desc->next = 0;
	pt_desc->child_pt_desc = 0;

	return pt_desc;
}

static inline void extract_va_metadata(u64 va, struct va_metadata *meta)
{
	u64 mask = 0xFFF;

	meta->offset = va & mask;	//offset is calculated with last
					//12 bits

	mask = BITM(9);		//9 bits bitmask

	meta->index[0] = (va >> 30) & mask;
	meta->index[1] = (va >> 21) & mask;
	meta->index[2] = (va >> 12) & mask;
}

static inline u64 pte_is_empty(u64 pte)
{
	return !(pte);
}

static u64 extract_addr_from_pte(u64 pte)
{
	u64 addr = 0;

	addr = pte & ~(0xF0000000000FFF);
	if (addr < 0xFFFFFFFF)
		return addr;

	return KERNEL_START + (addr & 0xFFFFFFFF);
}

static void pt_desc_add_child(struct page_table_desc *parent,
			      struct page_table_desc *child)
{
	struct page_table_desc *pt_desc = parent->child_pt_desc;

	if (pt_desc) {
		while (pt_desc->next)
			pt_desc = pt_desc->next;

		pt_desc->next = child;
		return;
	}

	parent->child_pt_desc = child;
}

static struct page_table_desc *pt_desc_get_from_parent(u64 addr,
						       struct page_table_desc *parent)
{
	struct page_table_desc *pt_desc = parent->child_pt_desc;

	// no child!
	if (!pt_desc)
		return 0;

	while (pt_desc) {
		if (addr == (u64)pt_desc->pt)
			return pt_desc;

		pt_desc = pt_desc->next;
	}

	return 0;
}

void *map_pa_to_va_pg(u64 pa, u64 va, struct page_table_desc *pt_top,
		      u64 flags, bool walk_only)
{
	struct page_table_desc *pt_desc = pt_top;
	struct va_metadata meta;
	u64 pte_flags = flags;
	u16 pt_index;
	u8 level, i;
	u64 pte;
	u64 *pt;

	extract_va_metadata(va, &meta);
	meta.offset += pa;	//we want physical address at the end

	if (!flags)
		pte_flags = PTE_FLAGS_KERNEL_GENERIC;

	for (i = pt_desc->level; i <= PT_LVL_MAX; i++) {
		level = pt_desc->level;
		pt = pt_desc->pt;
		pt_index = meta.index[level - 1];

		//L3
		if (i == PT_LVL_MAX) {
			// ensure all is up to date
			mb();
			if (walk_only)
				return &pt[pt_index];

			pt[pt_index] = pt_entry(meta.offset, pte_flags);
			break;
		}

		pte = pt[pt_index];
		if (pte_is_empty(pte)) {
			struct page_table_desc *new_pt_desc;

			//we create next level
			new_pt_desc = create_pt(0, level + 1);
			if (!new_pt_desc)
				return 0;

			pte = pt_entry((u64)new_pt_desc->pt, pte_flags);
			place_pt_entry(pt_desc, pte, pt_index);

			pt_desc_add_child(pt_desc, new_pt_desc);
			pt_desc = new_pt_desc;
		} else {
		       pt_desc = pt_desc_get_from_parent(
						extract_addr_from_pte(pte),
						pt_desc);

			if (!pt_desc)
				return 0;
		}
	}

	return (void *)va;
}

/*
 * Assume this is the first function to be called for page tables
 * when starting MMU for now
 */
void create_id_mapping(u64 start_paddr, u64 end_paddr, u64 flags)
{
	/*
	 * for now lets assume T0/1_SZ is constant at 25, so we
	 * only care about 3 levels
	 */
	struct page_table_desc *pt_desc;
	int i;

	pt_desc = create_pt(0, 1);
	if (!pt_desc)
		return;

	for (i = start_paddr; i < end_paddr; i += PAGE_SIZE)
		map_pa_to_va_pg(i, i, pt_desc, flags, 0);
}

static inline void invalidate_tlb_va(u64 va)
{
	asm volatile ("tlbi vaae1, %0" :: "r"(va));
}

/*
 * returns old flags to save and adds attribute provided
 */
void va_set_attr(u64 va, struct page_table_desc *pt_desc, u64 pte_attr)
{
	u64 *pte = map_pa_to_va_pg(0, va, pt_desc, 0, 1);

	*pte = (*pte | pte_attr);
}

/*
 * returns old flags to save and adds attribute provided
 */
void va_clear_attr(u64 va, struct page_table_desc *pt_desc, u64 pte_attr)
{
	u64 *pte = map_pa_to_va_pg(0, va, pt_desc, 0, 1);

	*pte = (*pte & ~pte_attr);
	invalidate_tlb_va(va);

	// ensure all changes have been committed
	mb();
	isb();
}
