#include <stddef.h>
#include "hobos/lib/stdlib.h"
#include "hobos/kstdio.h"
#include "hobos/mmio.h"
#include "hobos/timer.h"
#include "hobos/smp.h"
#include "hobos/gpio.h"
#include "hobos/mmu.h"

extern uint8_t curr_core_el(void);
extern uint8_t curr_core_id(void);
extern void switch_el(void);

#define KERNEL_START	0xFFFFFF8000000000
#define USER_END        ((volatile unsigned int*) (KERNEL_START + 0x1f000))

void setup_console() 
{
	struct gpio_controller ctrl;

	init_console(&ctrl);
	kprintf("\n\nConsole set by %d\n", curr_core_id());
}


int cntr = 0;
void test(void) 
{
	//setup_console();
	smp_store_release(&cntr, READ_ONCE(cntr)+1); 	
	kprintf("Hello again %d\n", cntr);
}
smp_process(test, 1)
smp_process(test, 2)

extern void setup_stack(void);

/* I'm alive */
void heartbeat(void)
{
	__run_process((uint64_t) setup_stack, 1);
	smp_store_release(&semaphores[1], 0); 
	
	__run_process((uint64_t) setup_stack, 2);
	smp_store_release(&semaphores[2], 0); 

	smp_run_process(test, 1);
	smp_run_process(test, 2);
}


void kernel_panic()
{
	uint64_t *x = (uint64_t *) 0x1f000;
	
	*x = 0xdeadbeef;
}

void init_kernel() 
{
	init_mmu();
	switch_vmem();
}

void main()
{

	//init_kernel();
	heartbeat();

	while (1) {
		//start shell here
	}
	

}
