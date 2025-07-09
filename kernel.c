#include <stddef.h>
#include "hobos/lib/stdlib.h"
#include "hobos/kstdio.h"
#include "hobos/mmio.h"
#include "hobos/smp.h"

extern int setup_stack(void);


void test_print1(void)
{
	kprintf("Hello again 1\n");
	return;
}

void test_print2(void)
{
	kprintf("Hello again 2\n");
	return;
}

/* I'm alive */
void heartbeat(void)
{
	run_process((uint64_t) setup_stack, 1);
	run_process((uint64_t) setup_stack, 2);

	//TODO: threading implementation
	//with proper mutex implementation, we
	//should be able to see both these functions executed
	run_process((uint64_t) test_print1, 1);
	run_process((uint64_t) test_print1, 2);
	run_process((uint64_t) test_print1, 2);
	run_process((uint64_t) test_print1, 2);
	run_process((uint64_t) test_print2, 1);

}

void main()
{
	get_rpi_version();
	mmio_init();
	init_console();

	heartbeat();

	while (1) {
		//start shell here
	}

}
