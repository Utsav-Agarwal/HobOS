#ifndef __IRQ_CONTROLLER
#define __IRQ_CONTROLLER

#include <stdint.h>
#include "mmio.h"

#define IRQ_BCM_SOC	0xA
#define IRQ_ARM_GENERIC	0xB

struct irq_controller {
	char name[128];
	uint8_t type;	
	uint64_t *base;	
	
	void (*enable_interrupt) (uint64_t interrupt_nr);
};

void init_irq_controller(struct irq_controller *irq, uint8_t type);

#endif
