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

#define KERNEL_START	((volatile unsigned int*)0xFFFFFF8000000000)
#define USER_END        ((volatile unsigned int*)0x200000)

void identity_mmio_test()
{
        *USER_END=0xdeadbeef;
        *(USER_END + 0x1000) = 0xdeadbeef;	//no operation performed (fault is not enabled)
        //*KERNEL_START='X';
}


/* I'm alive */
void heartbeat(void)
{
	//struct timer t;

	identity_mmio_test();	
}

void setup_console() 
{
	struct gpio_controller ctrl;

	init_gpio(&ctrl);
	init_console(&ctrl);
	kprintf("\n\nConsole set\n");
}

void main()
{

	//heartbeat();
	init_mmu();

	while (1) {
		//start shell here
	}
	

}
