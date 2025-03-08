#include <stddef.h>
#include <stdint.h>
#include "hobos/gpio.h"
#include "hobos/uart.h"

uint32_t mmio_base;

extern void set_gpio(uint8_t pin_nr, uint8_t dir);

/* I'm alive */
void heartbeat(void)
{
	mini_uart_init();
	mini_uart_puts("hello world\r\n");
}

void main()
{
	int rpi_version = 0;

	rpi_version = get_rpi_version();
	mmio_init(rpi_version);
	heartbeat();
	while (1)
		;
}
