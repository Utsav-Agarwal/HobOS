// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/lib/stdlib.h>
#include <hobos/mmu.h>
#include <hobos/entry.h>
#include <hobos/kstdio.h>

static inline int user_addr_is_valid(void *addr)
{
	u64 addr_u64 = (u64)addr;

	if (addr_u64 < (u64)va_to_pa((char *)USR_INIT))
		return -1;

	if (addr_u64 > (u64)va_to_pa((u64 *)USR_END))
		return -1;

	return 0;
}

// TODO: add point to modify page table attributes for this page
unsigned int copy_from_user(void *u_buf, void *k_buf, unsigned int len)
{
	u64 user_buf_addr = (u64)u_buf;
	unsigned int effective_len = USR_END - user_buf_addr;

	if (user_addr_is_valid(u_buf)) {
		kprintf("Invalid address!: %x [%x:%x]\n",
			u_buf, USR_INIT, USR_END);

		return -1;
	}

	if (effective_len > len)
		effective_len = len;

	/*
	 * We can already read userspace from kernel
	 */
	strcpy(k_buf, u_buf);
	return effective_len;
}
