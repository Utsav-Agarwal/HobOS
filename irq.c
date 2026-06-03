// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/irq.h>

/*
 * Return current irq config and disable irqs
 */
u64 irq_save(void)
{
	u64 flags;

	asm volatile("mrs %0, daif\n"
		     "msr daifset, #2\n"
		     :"=r"(flags));

	return flags;
}


/*
 * Restore irqs from given config
 */
u64 irq_restore(u64 flags)
{
	asm volatile("msr daif, %0\n"
		     ::"r"(flags));

	return flags;
}
