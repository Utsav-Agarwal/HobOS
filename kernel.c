#include <stddef.h>
#include "hobos/lib/stdlib.h"
#include "hobos/kstdio.h"
#include "hobos/mmio.h"
#include "hobos/timer.h"
#include "hobos/smp.h"
#include "hobos/gpio.h"
#include "hobos/mmu.h"

extern int setup_stack(void);
extern uint8_t curr_core_el(void);
extern void switch_el(void);

#define KERNEL_START	0xFFFFFF8000000000
#define USER_END        ((volatile unsigned int*) (KERNEL_START + 0x1f000))

/* I'm alive */
void heartbeat(void)
{
	*USER_END = 0xcacacaca;	
}

void setup_console() 
{
	struct gpio_controller ctrl;

	init_console(&ctrl);
	kprintf("\n\nConsole set\n");
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

	init_kernel();
	heartbeat();

	//should trigger data abort
	//setup_console();

	while (1) {
		//start shell here
	}
	

}
