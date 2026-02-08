// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/barrier.h>
#include <hobos/entry.h>
#include <hobos/lib/pt_lib.h>
#include <hobos/lib/stdlib.h>
#include <hobos/mmu.h>
#include <hobos/kstdio.h>
#include <hobos/compiler_types.h>

/*
 * We essentially want to create an identity mapped translation table to enter userspace.
 * This will be similar to the kernel switch since we need to keep the page tables within the
 * realm of userspace as well.
 */
static void map_to_usr(u64 start, u64 end)
{
	// add 3 page table addresses to current kernel table
	while (start < end) {
		map_pa_to_va_pg(start, start, global_page_tables[0],
				PTE_FLAGS_USER_GENERIC);
		start += 0x1000;
	}
}

void usr_init(void)
{
	kprintf("User memory range: [0x%x, 0x%x] (%d KiB)\n",
		USR_INIT, USR_END, USR_SZ_KB);

	map_to_usr(USR_INIT, USR_END);
	kprintf("Init success\n");
	// TODO: Maybe assign a new page table?
}

__section(".usr_entry") void usr_entry_pt(void)
{
	while (1)
		;
}
