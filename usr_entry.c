// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/barrier.h>
#include <hobos/entry.h>
#include <hobos/lib/pt_lib.h>
#include <hobos/lib/stdlib.h>
#include <hobos/mmu.h>
#include <hobos/kstdio.h>
#include <hobos/compiler_types.h>

void (*usr_entry)(void) = (void (*)(void)) (USR_INIT + PAGE_SIZE); 

void usr_init(void)
{
	u64 pt_addr = (u64)kmalloc(0x1000 * 4);

	create_id_mapping(USR_INIT, USR_INIT + (0x1000 * 512), pt_addr, 
			  PTE_FLAGS_USER_GENERIC);

	set_ttbr0_el1(pt_addr);
	dma_mb();
}

 __section(".user_test")void usr_entry_test (void)
{
	volatile int *x = (volatile int *)0x202000;

	*x = 0xdeadbeef;

	while(1);
}
