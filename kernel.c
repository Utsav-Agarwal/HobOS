// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/compiler_types.h>
#include <hobos/kstdio.h>
#include <hobos/mmu.h>
#include <hobos/smp.h>
#include <hobos/timer.h>
#include <hobos/lib/stdlib.h>
#include <hobos/page_alloc.h>
#include <hobos/irq/irq_bcm.h>
#include <hobos/asm/barrier.h>
#include <hobos/entry.h>

extern __noreturn void jump_to_usr(void);

struct irq_controller soc_irq;
struct irq_bcm_priv priv;

static inline void kernel_init_msg(void)
{
	kprintf("\nStarting kernel...\n");
}

/* I'm alive */
void kernel_splash_msg(void)
{
	kprintf("\n** Welcome to HobOS! **\n");
}

void kernel_test(void)
{
	global_timer.reset_timer(&global_timer);
	global_timer.set_timer(&global_timer, 0x200000);
}

static void setup_console(void)
{
	struct gpio_controller ctrl;

	init_gpio(&ctrl);
	init_console(&uart_dev, (void *)&ctrl);
	kernel_init_msg();
}

//TODO:
__noreturn void kernel_panic(void)
{
	kprintf("Kernel panicked!\n");
	while (1)
		;
}

static void enable_interrupts(void)
{
	enable_global_interrupts();

	soc_irq.priv = &priv;
	init_irq_controller(&soc_irq, IRQ_BCM_SOC);

	soc_irq.enable_interrupt(soc_irq.priv, BCM_DEFAULT_IRQ_TIMER);
}

static void init_device_drivers(void)
{
	init_timer(&global_timer);
	enable_interrupts();
}

__noreturn void main(void)
{
	mmio_init();
	init_free_list((u64)&__phymem_end, PAGE_SIZE * 256);
	init_mmu();
	setup_console();

	if (!global_page_tables)
		kernel_panic();

	init_device_drivers();
	//init_smp();

	switch_vmem();
	jump_to_usr();

	while (1)
		;
}
