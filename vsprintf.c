#include "hobos/nostdlibc_arg.h"
#include "hobos/kstdio.h"
#include <stdarg.h>

static char *format_string(char *c, va_list args)
{
	char *buf_start = c - 1; //start replacing from %

	//TODO

}

int vsprintf(char *buf, const char *format, va_list args)
{
	//keep dumping into buffer until you encounter
	//a special symbol (%*). 
	//
	//Replace it with the first arg in the args stack.
	//
	//each arg type will require processing it in a
	//different way (for instance, number will need
	//converstion to ASCII chars)
	
	char *c = buf;
	while(*c) {
		/*
		 * Possibly a formatted option, check
		 * which format it is
		 */
		if (*c++ == '%')
			switch (*c) {
				case 's':
					format_string(c, args);
				default:
					continue;
			}
	}

	return 0;
}
