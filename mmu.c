// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/barrier.h>
#include <hobos/kstdio.h>
#include <hobos/mmu/bcm2835.h>
#include <hobos/mmu.h>
#include <hobos/smp.h>

#define map_sz		512
#define ID_PG_SZ	PAGE_SIZE

void create_id_mapping(u64 start_paddr, u64 end_paddr,
			u64 pt)
{
	//for now lets assume T0/1_SZ is constant at 25, so we
	//only care about 3 levels
	u64 flags = PTE_FLAGS_KERNEL_GENERIC;
	u64 end_addr;
	struct page_table_desc *pt_desc;

	pt_desc = create_pt(pt, 1);
	end_addr = (u64)(pt_desc->pt) + NEXT_PT_OFFSET;
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

static inline u64 pte_is_empty(u64 pte)
{
	return !(pte);
}

void map_pa_to_va_pg(u64 pa, u64 va, struct page_table_desc *pt_top,
		     u64 flags)
{
	struct page_table_desc *pt_desc = pt_top;
	u64 volatile *pt, pte;
	struct va_metadata meta;
	u16 pt_index;
	u64 pte_flags = flags;
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
				pte_flags = PTE_FLAGS_KERNEL_GENERIC;

			pt[pt_index] = pt_entry(meta.offset, pte_flags);
			break;
		}

		pte = pt[pt_index];
		if (pte_is_empty(pte)) {
			struct page_table_desc *new_pt_desc;

			//we create next level
			new_pt_desc = create_pt(0, level + 1);
			pte = pt_entry((u64)new_pt_desc->pt,
				       PTE_FLAGS_KERNEL_GENERIC);

			place_pt_entry(pt_desc, pte, pt_index);
			pt_desc = new_pt_desc;
		} else {
			//extract next table
			pt_desc = (struct page_table_desc *)
				((pte & ~(0xF0000000000FFF)) + 0x1000);
		}
	}
}

volatile char pt __attribute__((section(".misc"))) = 0;
static u64 set_id_translation_table(void)
{
	kprintf("pt: %x\n", &pt);
	create_id_mapping(0, 0x1000 * 512, (u64)&pt);
	return (u64)(global_page_tables[0]->pt);
}

static u64 set_kernel_translation_table(void)
{
	//we can just reuse the id_map for now as we only
	//really care about it being mapped to high memory
	return (u64)(global_page_tables[0]->pt);
}

static inline void set_ttbr1_el1(u64 x)
{
	asm volatile ("msr ttbr1_el1, %0"::"r"(x));
}

static inline void set_ttbr0_el1(u64 x)
{
	asm volatile ("msr ttbr0_el1, %0"::"r"(x));
}

void switch_vmem(void)
{
	u64 tcr, reg;

	set_ttbr1_el1((u64)set_kernel_translation_table());

	asm volatile ("mrs %0, tcr_el1" : "=r"(tcr));

	tcr |= TG1_GRANULE_SZ_4KB << TG1_POS;
	tcr |= MMU_TSZ << T1SZ_POS;
	tcr |= EPD_WALK << EPD1_POS;

	asm volatile ("msr tcr_el1, %0"::"r"(tcr));

	//at this point the table should be active, so in theory
	//we should be able to just set the next instruction

	reg = ((u64)&__core0_stack) + KERNEL_START;
	asm volatile ("mov sp, %0"::"r"(reg));

	asm volatile ("mov %0, lr" : "=r"(reg));
	reg += KERNEL_START;
	asm volatile ("mov lr, %0"::"r"(reg));

	asm volatile ("tlbi vmalle1; dsb sy; isb");
}

void init_mmu(void)
{
	u64 tcr = 0;
	u64 sctlr = 0;

	//page table set
	//we want to share the page tables from core 0 to others, to start
	//with
	if (!curr_core_id())
		set_ttbr0_el1(set_id_translation_table() + CNP_COMMON);
	else
		set_ttbr0_el1((u64)global_page_tables[0]->pt + CNP_COMMON);

	asm volatile ("msr mair_el1, %0"::"r"(mair_el1));

	//translation control TCR_EL1

	tcr = MMU_TSZ << T0SZ_POS;
	tcr |= TG0_GRANULE_SZ_4KB << TG0_POS;
	tcr |= EPD_WALK << EPD0_POS;
	tcr |= 0b00 << IRGN0_POS;
	tcr |= 0b00 << ORGN0_POS;
	tcr |= 0b00 << SH0_POS;

	asm volatile ("msr tcr_el1, %0"::"r"(tcr));

	//system control SCTLR_EL1

	asm volatile ("mrs %0, sctlr_el1" : "=r"(sctlr));

#ifdef SCTLR_QUIRKS
	handle_sctlr_quirks(&sctlr);
#endif
	sctlr &= ~(1 << 1); //remove alignment check
	sctlr &= ~(1 << 25); //make sure little endian

	//enable mmu
	sctlr |= 1;

	asm volatile ("msr sctlr_el1, %0"::"r"(sctlr));
	asm volatile ("dsb sy; isb");

	//invalidate tlb
	asm volatile ("tlbi vmalle1; dsb sy; isb");
}
