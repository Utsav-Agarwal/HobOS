#include "hobos/kstdio.h"
#include "hobos/lib/vsprintf.h"
#include "hobos/mmio.h"
#include "hobos/nostdlibc_arg.h"
#include "hobos/uart.h"


extern uint8_t rpi_version;

/*
 * This library links the serial drivers to commonly
 * used functions such as printf.
 *
 * It is also responsible for initializing the appropriate
 * serial interface based on the device being used
 */

// kprintf = printf
// init_console
// clr_console
// read_console
// write_console
//
// TODO: Establish locks when accessing peripherals
// TODO: Establish a device registration layer, 
// enabling more code reuse, maybe use ifdefs and override function
// names with selective compilation


void init_console(struct gpio_controller *ctrl)
{
	if (rpi_version == 5)
		uart_init();
	else
		mini_uart_init(ctrl);
}

void putc(char c)
{
	if (rpi_version == 5)
		uart_putc(c);
	else
		mini_uart_putc(c);
}


char getc(void)
{
	if (rpi_version == 5)
		return uart_getc();
//	else
//		return	mini_uart_getc(); //TODO

	return 0;
}

void puts(char *c)
{
	if (rpi_version == 5)
		uart_puts(c);
	else
		mini_uart_puts(c);
}

int kprintf(const char *format, ...)
{
	va_list args;
	int printed;

	va_start(args, format);
	printed = vprintf(format, args);
	va_end(args);
	return printed;

}
