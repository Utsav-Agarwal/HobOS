/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef ENTRY_H
#define ENTRY_H

#include <hobos/page_alloc.h>

// based on the identity page table mapping
#define USR_INIT	((u64)(0x1000 * 512))

extern void jump_to_usr(void);

/* map physical memory, 
 * assign page table and jump to user code
 */
void usr_init(void);
void switch_to_usr(void);

#endif
