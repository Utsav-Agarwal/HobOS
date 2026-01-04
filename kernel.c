#include "hobos/kstdio.h"
#include "hobos/mmu.h"
#include "hobos/smp.h"

extern struct char_device uart_dev;

void setup_console() 
{
	struct gpio_controller ctrl;

	init_gpio(&ctrl);
	init_console(&uart_dev, (void *)&ctrl);
}

/* I'm alive */
void heartbeat(void)
{
    
	kprintf("Hello from vmem\n");
}

//TODO: write 
void kernel_panic(void)
{
	while (1);
}

void init_device_drivers(void)
{
}

void main()
{

	mmio_init();
	init_mmu();
	setup_console();
	
	init_device_drivers();
	
	init_smp();
	switch_vmem();
	
	heartbeat();

	while (1) {
		//start shell here
	}
	
}
