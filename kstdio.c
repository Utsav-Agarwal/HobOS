// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/kstdio.h>

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

struct char_device *_console;

void init_console(struct char_device *console, void *priv)
{
    _console = console;
    _console->init(priv);

    //any extra init?
    if (_console->quirks)
	_console->quirks(priv);
}

void putc(char c)
{
    _console->putc(c);
}

char getc(void)
{
    return _console->getc();
}

void puts(char *s)
{
    _console->puts(s);
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
