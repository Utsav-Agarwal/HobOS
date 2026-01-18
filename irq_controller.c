#include "hobos/irq_controller.h"
#include "hobos/irq/irq_bcm.h"
#include "hobos/kstdio.h"
#include "hobos/mmio.h"

//use soc based ic by default for now
void init_irq_controller(struct irq_controller *irq, uint8_t type)
{
	switch (type) {
		case IRQ_BCM_SOC:
			bcm_irq_controller_init(irq);
			break;
		//TODO:
		case IRQ_ARM_GENERIC:	
			return;
		default:
			kprintf("Unrecognized IRQ controller type\n");
			return;
	}
}
