#include "hobos/io.h"
#include "hobos/irq/irq_bcm.h"
#include "hobos/lib/stdlib.h"
#include "hobos/mmio.h"
#include "hobos/kstdio.h"

static void bcm_irq_enable_interrupt (void *priv, 
		uint64_t int_nr)
{
	struct irq_bcm_priv *bcm_priv = (struct irq_bcm_priv *) priv;
	struct irq_controller *ctrl = bcm_priv->ctrl;
	uint8_t *base = (uint8_t *)(ctrl->base);
	uint32_t *basic_interrupts = (uint32_t *)(base + ENABLE_BASIC_IRQS);

	iowrite32(basic_interrupts, (uint32_t) BITP(int_nr));
}

static void bcm_irq_disable_interrupt (void *priv, 
		uint64_t int_nr)
{
	struct irq_bcm_priv *bcm_priv = (struct irq_bcm_priv *) priv;
	struct irq_controller *ctrl = bcm_priv->ctrl;
	uint8_t *base = (uint8_t *)(ctrl->base);
	uint32_t *basic_interrupts = (uint32_t *)(base + DISABLE_BASIC_IRQS);

	iowrite32(basic_interrupts, (uint32_t) BITP(int_nr));
}


void bcm_irq_controller_init(struct irq_controller *irq)
{
	struct irq_bcm_priv *priv;
	char name[128] = "bcm-irq";
	uint64_t base;
	
	switch (rpi_version) {
		case 3:
			base = IRQ_BASE_OFFSET_BCM_2835;
			break;
		default:
			base = 0;
			return;
	}

	memcpy(irq->name, name, 128);
	irq->base = (uint64_t *) ioremap((uint64_t)mmio_base + base);
	irq->enable_interrupt = bcm_irq_enable_interrupt;
	irq->enable_interrupt = bcm_irq_disable_interrupt;

	priv->ctrl = irq;
	irq->priv = (void *) priv;
}
