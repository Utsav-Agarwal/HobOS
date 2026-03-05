// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/barrier.h>
#include <hobos/compiler_types.h>
#include <hobos/mmu/bcm2835.h>
#include <hobos/mmu.h>
#include <hobos/smp.h>
#include <hobos/kstdio.h>

#define map_sz		512
#define ID_PG_SZ	PAGE_SIZE

/*
 * We can just reuse the id_map for now as we only
 * really care about it being mapped to high memory
 */
static inline u64 get_kernel_translation_table(void)
{
	struct page_table_desc *pt_desc = &global_page_tables[0];

	pt_desc = (struct page_table_desc *)(va_to_pa(pt_desc));
	if (!pt_desc)
		return 0;

	return (u64)(pt_desc->pt);
}

static inline u64 get_ttbr0_el1(void)
{
	u64 reg;

	asm volatile ("mrs %0, ttbr0_el1" : "=r"(reg));
	return reg;
}

static inline u64 get_ttbr1_el1(void)
{
	u64 reg;

	asm volatile ("mrs %0, ttbr1_el1" : "=r"(reg));
	return reg;
}

static inline void set_ttbr1_el1(u64 x)
{
	u64 _x = x;

	if (_x > KERNEL_START)
		_x -= KERNEL_START;

	asm volatile ("msr ttbr1_el1, %0"::"r"(_x));
	asm volatile ("dmb ish; tlbi vmalle1; ic iallu; isb; dmb ish;");
}

void set_ttbr0_el1(u64 x)
{
	//disable_ttbr0_el1();
	asm volatile ("msr ttbr0_el1, %0"::"r"(x));
	asm volatile ("dmb ish; tlbi vmalle1; isb; dmb ish;");
	//enable_ttbr0_el1();
}

void switch_vmem(void)
{
	u64 tcr, reg;

	set_ttbr1_el1(get_ttbr0_el1());

	asm volatile ("mrs %0, tcr_el1" : "=r"(tcr));

	tcr |= TG1_GRANULE_SZ_4KB << TG1_POS;
	tcr |= MMU_TSZ << T1SZ_POS;
	tcr |= EPD_WALK << EPD1_POS;

	asm volatile ("msr tcr_el1, %0"::"r"(tcr));

	//at this point the table should be active, so in theory
	//we should be able to just set the next instruction

	asm volatile ("mov %0, sp" : "=r"(reg));
	reg += KERNEL_START;
	asm volatile ("mov sp, %0"::"r"(reg));

	asm volatile ("mov %0, lr" : "=r"(reg));
	reg += KERNEL_START;
	asm volatile ("mov lr, %0"::"r"(reg));

	asm volatile ("mrs %0, vbar_el1" : "=r"(reg));
	reg += KERNEL_START;
	asm volatile ("msr vbar_el1, %0"::"r"(reg));

	asm volatile ("tlbi vmalle1; dsb sy; isb");
}

static u64 *extract_pt_addr(int level)
{
	u64 *pt_addr = 0;

	if (level > 1)
		return (u64 *)(*global_page_tables[level - 2].pt & 0xFFFFF000);
	else
		return (u64 *)get_ttbr1_el1();

	return pt_addr;
}

/*
 * Given ttbr1, extract all info for existing page tables into pt_desc
 */
static void extract_global_pt_info(void)
{
	struct page_table_desc *pt_desc = 0;
	struct page_table_desc *parent = 0;
	int i;

	/*
	 * For now, lets just assume that we are only going to be dealing
	 * with 3 levels, always starting from 1
	 */

	for (i = 1; i < 4; i++) {
		/* ensure you link the pt descriptors */
		if (pt_desc)
			parent = pt_desc;

		pt_desc = create_pt_desc(i);
		pt_desc->pt = extract_pt_addr(i);

		if (i == 3)
			pt_desc->pt_len = 512;
		else
			pt_desc->pt_len = 1;

		if (pt_desc)
			parent->child_pt_desc = pt_desc;
	}
}

void init_mmu(void)
{
	//page table set
	//we want to share the page tables from core 0 to others, to start
	//with
	//
	//TODO: extract all data from existing page table into table descriptors
	if (!curr_core_id())
		extract_global_pt_info();
	else
		set_ttbr0_el1(get_kernel_translation_table());

	if (!get_kernel_translation_table())
		return;

	asm volatile ("msr mair_el1, %0"::"r"(mair_el1));
	asm volatile ("dsb sy; isb");

	//invalidate tlb
	asm volatile ("tlbi vmalle1; dsb sy; isb");
}
