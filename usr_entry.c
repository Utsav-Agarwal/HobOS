// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/barrier.h>
#include <hobos/entry.h>
#include <hobos/lib/pt_lib.h>
#include <hobos/lib/stdlib.h>
#include <hobos/mmu.h>
#include <hobos/kstdio.h>

void usr_init(void)
{
	void *pt = kmalloc(0x1000 * 4);

	create_id_mapping(USR_INIT, USR_INIT + (0x1000 * 512), (u64)pt, 
			  PTE_FLAGS_USER_GENERIC);
}
