#include <stddef.h>
#include <stdint.h>
#include "hobos/gpio.h"
#include "hobos/uart.h"

uint8_t rpi_version;
uint64_t mmio_base;
uint64_t uart_base;

extern void set_gpio(uint8_t pin_nr, uint8_t dir);

/* I'm alive */
void heartbeat(void)
{
	char *x = "Hello World\r\n";

	uart_init();
	uart_puts(x);
}

void main()
{
	get_rpi_version();
	mmio_init(rpi_version);
	heartbeat();
	while (1)
		;
}
