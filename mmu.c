// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/barrier.h>
#include <hobos/compiler_types.h>
#include <hobos/mmu/bcm2835.h>
#include <hobos/mmu.h>
#include <hobos/smp.h>

#define map_sz		512
#define ID_PG_SZ	PAGE_SIZE

/*
 * This is the first id translation table created, it will always be the 1st page table
 * created.
 */
static u64 set_id_translation_table(void)
{
	create_id_mapping(0, 0x1000 * 512, 0, PTE_FLAGS_KERNEL_GENERIC);
	if (!global_page_tables[0])
		return 0;

	return (u64)(global_page_tables[0]->pt);
}

/*
 * We can just reuse the id_map for now as we only
 * really care about it being mapped to high memory
 */
static inline u64 get_kernel_translation_table(void)
{
	if (!global_page_tables[0])
		return 0;

	return (u64)(global_page_tables[0]->pt);
}

static inline void set_ttbr1_el1(u64 x)
{
	asm volatile ("msr ttbr1_el1, %0"::"r"(x));
}

static void disable_ttbr0_el1(void)
{
	u64 tcr;

	asm volatile ("mrs %0, tcr_el1" : "=r"(tcr));
	tcr |= EPD_FAULT << EPD0_POS;
	asm volatile ("msr tcr_el1, %0"::"r"(tcr));
}

static void enable_ttbr0_el1(void)
{
	u64 tcr;

	asm volatile ("mrs %0, tcr_el1" : "=r"(tcr));
	tcr |= EPD_WALK << EPD0_POS;
	asm volatile ("msr tcr_el1, %0"::"r"(tcr));
}

void set_ttbr0_el1(u64 x)
{
	disable_ttbr0_el1();
	mb();
	asm volatile ("msr ttbr0_el1, %0"::"r"(x));
	asm volatile ("dmb ish; tlbi vmalle1; isb; dmb ish;");
	enable_ttbr0_el1();
}

void switch_vmem(void)
{
	u64 tcr, reg;

	set_ttbr1_el1((u64)get_kernel_translation_table());

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

	asm volatile ("mrs %0, vbar_el1" : "=r"(reg));
	reg += KERNEL_START;
	asm volatile ("msr vbar_el1, %0"::"r"(reg));

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
		set_ttbr0_el1(get_kernel_translation_table() + CNP_COMMON);

	if (!get_kernel_translation_table())
		return;

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
