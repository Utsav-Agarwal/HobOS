#include "hobos/irq_controller.h"
#include "hobos/lib/stdlib.h"

static void bcm_irq_enable_interrupt (uint64_t interrupt_nr)
{
}

void bcm_irq_controller_init(struct irq_controller *irq)
{
	uint64_t base;
	char name[128] = "bcm-irq";
	
	switch (rpi_version) {
		case 3:
			base = 0xB000;
			break;
		default:
			base = 0;
			return;
	}


	
	memcpy(irq->name, name, 128);
	irq->base = (uint64_t *) ioremap((uint64_t)mmio_base + base);
	irq->enable_interrupt = bcm_irq_enable_interrupt;
}
