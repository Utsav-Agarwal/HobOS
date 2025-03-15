#include "hobos/nostdlibc_arg.h"
#include <stdarg.h>

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
	return 0;
}
