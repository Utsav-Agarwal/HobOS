#ifndef __IRQ_CONTROLLER
#define __IRQ_CONTROLLER

#include <stdint.h>

#define IRQ_BCM_SOC	0xA
#define IRQ_ARM_GENERIC	0xB

struct irq_controller {
	char name[128];
	void *priv;

	unsigned char type;
	unsigned long *base;

	void (*enable_interrupt)(void *priv, unsigned long interrupt_nr);
	void (*disable_interrupt)(void *priv, unsigned long interrupt_nr);
};

void init_irq_controller(struct irq_controller *irq, uint8_t type);

#endif
