// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/irq_controller.h>
#include <hobos/irq/irq_bcm.h>
#include <hobos/kstdio.h>
#include <hobos/mmio.h>
#include <hobos/timer.h>

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

void __handle_irq_main(void)
{
	kprintf("handling irq\n");
	global_timer.reset_timer(&global_timer);
	global_timer.set_timer(&global_timer, 0x200000);
	return;
}
