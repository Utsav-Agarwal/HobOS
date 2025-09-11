#include <stddef.h>
#include "hobos/lib/stdlib.h"
#include "hobos/kstdio.h"
#include "hobos/mmio.h"
#include "hobos/timer.h"
#include "hobos/smp.h"
#include "hobos/gpio.h"

extern int setup_stack(void);
extern uint8_t curr_core_el(void);
extern void switch_el(void);

/* I'm alive */
void heartbeat(void)
{
	//run_process((uint64_t) setup_stack, 1);
	//run_process((uint64_t) setup_stack, 2);
	//run_process((uint64_t) setup_stack, 3);
}

void main()
{
	get_rpi_version();
	mmio_init();
	
	struct gpio_controller ctrl;
	init_gpio(&ctrl);
	init_console(&ctrl);

	//kprintf("EL: %d\n", curr_core_el());
//	kprintf("EL: %d\n", curr_core_el());
//

	uint8_t x; 

	x = 0xdeadbeef;
	x = curr_core_el();
	//kprintf("Hello\n");

//	struct timer t;
//	init_timer(&t);
//	kprintf("timer: %d\n", read_timer(1, &t));
//	kprintf("timer: %d\n", read_timer(1, &t));
//
//	heartbeat();

	
	while (1) {
		//start shell here
	}

}
