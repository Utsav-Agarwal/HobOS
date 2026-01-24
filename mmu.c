// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/barrier.h>
#include <hobos/mmu/bcm2835.h>
#include <hobos/mmu.h>
#include <hobos/smp.h>

#define map_sz		512
#define ID_PG_SZ	PAGE_SIZE

//default MMU characteristics
//#define KERNEL_START	(~(BITM(64-MMU_TSZ)))

void create_id_mapping(u64 start_paddr, unsigned long end_paddr,
			unsigned long pt)
{
	//for now lets assume T0/1_SZ is constant at 25, so we
	//only care about 3 levels
	u16 flags = PTE_FLAGS_GENERIC;
	u64 end_addr;
	struct page_table_desc *pt_desc;

	pt_desc = create_pt(pt, 1);
	end_addr = (unsigned long)(pt_desc->pt) + NEXT_PT_OFFSET;
	create_pt_entries(pt_desc, end_addr, end_addr, flags);

	pt_desc = create_pt(end_addr, 2);
	end_addr += NEXT_PT_OFFSET;
	create_pt_entries(pt_desc, end_addr, end_addr, flags);

	pt_desc = create_pt(end_addr, 3);
	create_pt_entries(pt_desc, start_paddr, end_paddr, flags); //2GB
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

static inline unsigned long pte_is_empty(unsigned long pte)
{
	return !(pte);
}

void map_pa_to_va_pg(u64 pa, unsigned long va, struct page_table_desc *pt_top,
		     unsigned long flags)
{
	struct page_table_desc *pt_desc = pt_top;
	u64 volatile *pt, pte;
	struct va_metadata meta;
	u16 pt_index;
	u8 level, i;

	extract_va_metadata(va, &meta);
	meta.offset += pa;	//we want physical address at the end

	for (i = pt_desc->level; i <= PT_LVL_MAX; i++) {
		level = pt_desc->level;
		pt = pt_desc->pt;
		pt_index = meta.index[level - 1];

		//L3
		if (i == PT_LVL_MAX) {
			if (!flags)
				pt[pt_index] = pt_entry(meta.offset,
							PTE_FLAGS_GENERIC);
			else
				pt[pt_index] = pt_entry(meta.offset, flags);

			break;
		}

		pte = pt[pt_index];
		if (pte_is_empty(pte)) {
			struct page_table_desc *new_pt_desc;

			//we create next level
			new_pt_desc = create_pt(0, level + 1);
			pte = pt_entry((unsigned long)new_pt_desc->pt,
				       PTE_FLAGS_GENERIC);

			place_pt_entry(pt_desc, pte, pt_index);
			pt_desc = new_pt_desc;
		} else {
			//extract next table
			pt_desc = (struct page_table_desc *)
				((pte & ~(0xF0000000000FFF)) + 0x1000);
		}
	}
}

static unsigned long set_id_translation_table(void)
{
	create_id_mapping(0, 0x1000 * 512, (unsigned long)&__end);
	return (unsigned long)(global_page_tables[0]->pt);
}

static unsigned long set_kernel_translation_table(void)
{
	//we can just reuse the id_map for now as we only
	//really care about it being mapped to high memory
	return (unsigned long)(global_page_tables[0]->pt);
}

static inline void set_ttbr1_el1(unsigned long x)
{
	asm("msr ttbr1_el1, %0"::"r"(x));
}

static inline void set_ttbr0_el1(unsigned long x)
{
	asm("msr ttbr0_el1, %0"::"r"(x));
}

unsigned long switch_vmem(void)
{
	u64 tcr, reg;

	set_ttbr1_el1((unsigned long)set_kernel_translation_table());

	asm("mrs %0, tcr_el1" : "=r"(tcr));

	tcr |= TG1_GRANULE_SZ_4KB << TG1_POS;
	tcr |= MMU_TSZ << T1SZ_POS;
	tcr |= EPD_WALK << EPD1_POS;

	asm("msr tcr_el1, %0"::"r"(tcr));

	//at this point the table should be active, so in theory
	//we should be able to just set the next instruction

	reg = ((unsigned long)&__core0_stack) + KERNEL_START;
	asm("mov sp, %0"::"r"(reg));

	asm("mov %0, lr" : "=r"(reg));
	reg += KERNEL_START;
	asm("mov lr, %0"::"r"(reg));

	asm("tlbi vmalle1; dsb sy; isb");
}

void init_mmu(void)
{
	u64 tcr, sctlr, spsr;

	//page table set
	//we want to share the page tables from core 0 to others, to start
	//with
	if (!curr_core_id())
		set_ttbr0_el1(set_id_translation_table() + CNP_COMMON);
	else
		set_ttbr0_el1((unsigned long)global_page_tables[0]->pt + CNP_COMMON);

	asm("msr mair_el1, %0"::"r"(mair_el1));

	//translation control TCR_EL1

	tcr = MMU_TSZ << T0SZ_POS;
	tcr |= TG0_GRANULE_SZ_4KB << TG0_POS;
	tcr |= EPD_WALK << EPD0_POS;
	tcr |= 0b00 << IRGN0_POS;
	tcr |= 0b00 << ORGN0_POS;
	tcr |= 0b00 << SH0_POS;

	asm("msr tcr_el1, %0"::"r"(tcr));

	//system control SCTLR_EL1

	asm("mrs %0, sctlr_el1" : "=r"(sctlr));

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
}
