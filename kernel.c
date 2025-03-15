#include <stddef.h>
#include <stdint.h>
#include "hobos/kstdio.h"
#include "hobos/mmio.h"

extern void set_gpio(uint8_t pin_nr, uint8_t dir);

/* I'm alive */
void heartbeat(void)
{
	char *x = "Hello World\r\n";

	init_console();
	puts(x);
}

void main()
{
	get_rpi_version();
	mmio_init();
	heartbeat();
	while (1) {
		//start shell here
	}

}
