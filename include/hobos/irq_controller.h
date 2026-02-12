/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __IRQ_CONTROLLER
#define __IRQ_CONTROLLER

#include <hobos/types.h>

#define IRQ_BCM_SOC	0xA
#define IRQ_ARM_GENERIC	0xB

struct irq_controller {
	char name[128];
	void *priv;

	unsigned char type;
	volatile u64 *base;

	void (*enable_interrupt)(void *priv, unsigned long interrupt_nr);
	void (*disable_interrupt)(void *priv, unsigned long interrupt_nr);
};

void init_irq_controller(struct irq_controller *irq, char type);

#endif
