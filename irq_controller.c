// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/irq_controller.h>
#include <hobos/irq/irq_bcm.h>
#include <hobos/kstdio.h>
#include <hobos/mmio.h>

//use soc based ic by default for now
void init_irq_controller(struct irq_controller *irq, char type)
{
	switch (type) {
	case IRQ_BCM_SOC:
		kprintf("BCM IRQ Selected\n");
		bcm_irq_controller_init(irq);
		break;
	//TODO:
	case IRQ_ARM_GENERIC:
		kprintf("ARM GENERIC IRQ Selected\n");
		return;
	default:
		kprintf("Unrecognized IRQ controller type\n");
		return;
	}
}
