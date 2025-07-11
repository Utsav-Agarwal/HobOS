#include <stddef.h>
#include "hobos/lib/stdlib.h"
#include "hobos/kstdio.h"
#include "hobos/mmio.h"
#include "hobos/smp.h"
#include "hobos/mutex.h"

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

MUTEX(x);
/* I'm alive */
void heartbeat(void)
{

	queue_on_proc((uint64_t) test_print1, 0);
	unlock_mutex(&x);

	lock_mutex(&x);
	queue_on_proc((uint64_t) test_print2, 2);
	unlock_mutex(&x);

	//pause core 2
	queue_on_proc((uint64_t) 0x0, 2);

}

void main()
{
	get_rpi_version();
	mmio_init();
	init_console();
	init_smp();

	heartbeat();

	while (1) {
		//start shell here
	}

}
