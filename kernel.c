#include "hobos/kstdio.h"
#include "hobos/mmu.h"
#include "hobos/smp.h"
#include "hobos/timer.h"
#include "hobos/asm/barrier.h"

extern struct char_device uart_dev;
extern void enable_global_interrupts(void);

void setup_console() 
{
	struct gpio_controller ctrl;

	init_gpio(&ctrl);
	init_console(&uart_dev, (void *)&ctrl);
}

/* I'm alive */
void heartbeat(void)
{
    
	struct timer t;

	kprintf("%d\n", read_timer(1, &global_timer));
	kprintf("Hello from vmem\n");
	kprintf("%d\n", read_timer(1, &global_timer));
	dmb(ish);
}

//TODO:
void kernel_panic(void)
{
	while (1);
}

void init_device_drivers(void)
{
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
	}
	
}
