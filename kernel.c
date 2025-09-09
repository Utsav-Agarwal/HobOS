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

#define KERNEL_UART0_DR        ((volatile unsigned int*)0xFFFFFF8000000000)
#define KERNEL_UART0_FR        ((volatile unsigned int*)0xFFFFFF8000000018)

void identity_mmio_test()
{
    	char *s="Writing through MMIO mapped in higher half!\r\n";

	while(*s) {
        /* wait until we can send */
        do{asm volatile("nop");}while(*KERNEL_UART0_FR&0x20);
        /* write the character to the buffer */
        *KERNEL_UART0_DR=*s++;
    }
}


/* I'm alive */
void heartbeat(void)
{
	struct timer t;
	
	identity_mmio_test();	
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
