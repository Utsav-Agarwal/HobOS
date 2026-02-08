/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef ENTRY_H
#define ENTRY_H

#include <hobos/page_alloc.h>
#include <hobos/compiler_types.h>

// based on the identity page table mapping
#define USR_INIT	((u64)(0x1000 * 512))
#define USR_SZ		((u64)(0x1000 * 512 * 128))
#define USR_SZ_KB	(USR_SZ / KB(1))
#define USR_END		(USR_INIT + USR_SZ)

/*
 * map physical memory,
 * assign page table and jump to user code
 */
void usr_init(void);

#endif
