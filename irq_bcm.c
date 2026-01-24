// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/io.h>
#include <hobos/irq/irq_bcm.h>
#include <hobos/lib/stdlib.h>
#include <hobos/mmio.h>
#include <hobos/kstdio.h>

static void bcm_irq_enable_interrupt(void *priv,
				      unsigned long int_nr)
{
	struct irq_bcm_priv *bcm_priv = (struct irq_bcm_priv *)priv;
	struct irq_controller *ctrl = bcm_priv->ctrl;

	u8 *base = (char *)(ctrl->base);
	u32 *basic_interrupts = (unsigned int *)(base + ENABLE_IRQS_1);

	iowrite32((unsigned char *)basic_interrupts, (unsigned int)BITP(int_nr));
}

static void bcm_irq_disable_interrupt(void *priv,
				       unsigned long int_nr)
{
	struct irq_bcm_priv *bcm_priv = (struct irq_bcm_priv *)priv;
	struct irq_controller *ctrl = bcm_priv->ctrl;

	u8 *base = (char *)(ctrl->base);
	char *basic_interrupts = (char *)(base + DISABLE_BASIC_IRQS);

	iowrite32(basic_interrupts, (unsigned int)BITP(int_nr));
}

void bcm_irq_controller_init(struct irq_controller *irq)
{
	struct irq_bcm_priv *priv;
	char name[128] = "bcm-irq";
	u64 base;

	switch (rpi_version) {
	case 3:
		base = IRQ_BASE_OFFSET_BCM_2835;
		break;
	default:
		base = 0;
		return;
	}

	memcpy(irq->name, name, 128);
	irq->base = (unsigned long *)ioremap((unsigned long)mmio_base + base);
	irq->enable_interrupt = bcm_irq_enable_interrupt;
	irq->disable_interrupt = bcm_irq_disable_interrupt;

	priv->ctrl = irq;
	irq->priv = (void *)priv;
}
