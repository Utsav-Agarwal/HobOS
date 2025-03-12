#include <stddef.h>
#include <stdint.h>
#include "hobos/gpio.h"
#include "hobos/uart.h"

extern uint8_t rpi_version;
extern uint64_t mmio_base;
extern uint64_t uart_base;

extern void set_gpio(uint8_t pin_nr, uint8_t dir);

/* I'm alive */
void heartbeat(void)
{
	char *x = "Hello World\r\n";

	mini_uart_init();
	mini_uart_puts(x);
}

void main()
{
	get_rpi_version();
	mmio_init();
	heartbeat();
	while (1) {
		uart_putc(uart_getc());
		delay(1000000);
	}

}
