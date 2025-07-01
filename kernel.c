#include <stddef.h>
#include "hobos/lib/stdlib.h"
#include "hobos/kstdio.h"
#include "hobos/mmio.h"

/* I'm alive */
void heartbeat(void)
{
	char *x = malloc(2);
	char *a = malloc(3);
	
	x[0] = 'x';
	x[1] = 'y';

	a[0] = 'a';
	a[1] = 'b';
	a[2] = '\0';

	puts("test\n");
	kprintf("%s %d %x\n", a, 1123, 0xa12b);

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
