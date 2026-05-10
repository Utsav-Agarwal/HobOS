/* SPDX-License-Identifier: GPL-2.0-only */

#include <hobos/types.h>

#define IRQ_ACCEPT	0x2

u64 irq_save(void);
u64 irq_restore(u64 flags);
