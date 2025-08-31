#include <stddef.h>
#include "hobos/lib/stdlib.h"
#include "hobos/kstdio.h"
#include "hobos/mmio.h"
#include "hobos/timer.h"
#include "hobos/smp.h"
#include "hobos/gpio.h"
#include "hobos/mmu.h"

extern int setup_stack(void);

/* I'm alive */
void heartbeat(void)
{
	struct timer t;
	
	init_timer(&t);
	kprintf("timer: %d\n", read_timer(1, &t));
	
//	run_process((uint64_t) setup_stack, 1);
//	run_process((uint64_t) setup_stack, 2);
//	run_process((uint64_t) setup_stack, 3);
	
	uint32_t *x = (uint32_t *)0x00000010000000; 
	*x = 0xDEADBEEF;
	kprintf("low mem: %d\n", *x);

	kprintf("timer: %d\n", read_timer(1, &t));
	x = (uint32_t *) 0x0000FF00000004;
	*x = 0xDEADBEEF;
	kprintf("timer: %d\n", read_timer(1, &t));
	kprintf("low mem: %d\n", *x);
	kprintf("timer: %d\n", read_timer(1, &t));
}

void main()
{
	struct gpio_controller ctrl;
	

	get_rpi_version();
	mmio_init();
	init_mmu();
	
	init_gpio(&ctrl);
	
	init_console(&ctrl);


	heartbeat();

	while (1) {
		//start shell here
	}

}
