#include "hobos/kstdio.h"
#include "hobos/mmu.h"

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

void kernel_panic()
{
	while (1);
}

void main()
{

	mmio_init();
	init_mmu();
	
	//initialize drivers
	setup_console();
	
	switch_vmem();
	init_smp();
	heartbeat();

	while (1) {
		//start shell here
	}
	

}
