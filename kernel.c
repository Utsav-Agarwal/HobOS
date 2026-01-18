#include "hobos/kstdio.h"
#include "hobos/mmu.h"
#include "hobos/smp.h"
#include "hobos/timer.h"
#include "hobos/irq/irq_bcm.h"
#include "hobos/asm/barrier.h"

extern struct char_device uart_dev;
extern void enable_global_interrupts(void);

void test()
{
	kprintf("Hello\n");
}

void setup_console() 
{
	struct gpio_controller ctrl;

	init_gpio(&ctrl);
	init_console(&uart_dev, (void *)&ctrl);
}

/* I'm alive */
void static heartbeat(void)
{
	kprintf("%d\n", read_timer(1, &global_timer));
	kprintf("Hello from vmem\n");
	global_timer.set_timer(&global_timer, 0xFFFFFFFF);
	kprintf("%d\n", read_timer(1, &global_timer));
}

//TODO:
void kernel_panic(void)
{
	kprintf("Kernel panicked!\n");
	while (1);
}

static void enable_interrupts(void)
{
	struct irq_controller soc_irq;
	struct irq_bcm_priv priv;

	soc_irq.priv = &priv;
	init_irq_controller(&soc_irq, IRQ_BCM_SOC);
	
	soc_irq.enable_interrupt(soc_irq.priv, BCM_DEFAULT_IRQ_TIMER);
}

void init_device_drivers(void)
{
	enable_interrupts();
	init_timer(&global_timer);
}

void main()
{
	mmio_init();
	init_mmu();
	setup_console();

	enable_global_interrupts();
	init_device_drivers();
	
	init_smp();
	switch_vmem();
	
	heartbeat();

	while (1) {
			//start shell here
			asm volatile("wfe");
	}
}
