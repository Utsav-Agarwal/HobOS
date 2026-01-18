#ifndef __IRQ_BCM
#define __IRQ_BCM

#include "../irq_controller.h"

#define IRQ_BASE_OFFSET_BCM_2835		0xB000

#define IRQ_BASIC_PENDING	0x200
#define IRQ_PENDING_1		0x204
#define IRQ_PENDING_2		0x208
#define IRQ_FIQ_CONTROL		0x20C

#define ENABLE_IRQS_1		0x210
#define ENABLE_IRQS_2		0x214
#define ENABLE_BASIC_IRQS	0x218

#define DISABLE_IRQS_1		0x21C
#define DISABLE_IRQS_2		0x220
#define DISABLE_BASIC_IRQS	0x224

#define BCM_DEFAULT_IRQ_TIMER	0x0

void bcm_irq_controller_init(struct irq_controller *irq);

struct irq_bcm_priv {
	struct irq_controller *ctrl;
};

#endif
