#include <stddef.h>
#include "hobos/lib/stdlib.h"
#include "hobos/kstdio.h"
#include "hobos/mmio.h"
#include "hobos/smp.h"

/* test print */
void test_print(void)
{
	puts("Hello\n");
}

/* I'm alive */
void heartbeat(void)
{
	run_process((uint64_t) test_print, 1);
	run_process((uint64_t) test_print, 2);
	run_process((uint64_t) test_print, 3);
}

void main()
{
	get_rpi_version();
	mmio_init();
	init_console();

	heartbeat();

	while (1) {
		//start shell here
		delay(1000);
	}

}
