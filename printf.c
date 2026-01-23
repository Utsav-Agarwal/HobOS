#include <hobos/lib/stdlib.h>
#include <hobos/nostdlibc_arg.h>
#include <hobos/kstdio.h>
#include <stdarg.h>


#define DEBUG_PRINT_C(c) \
	puts("\nDEBUG_START\n"); \
	putc(c); \
	puts("\nDEBUG_END\n");

#define DEBUG_PRINT_S(s) \
	puts("\nDEBUG_START\n"); \
	puts(s); \
	puts("\nDEBUG_END\n");

#define D_BUF_SZ	128

union data {
	int i;
	char c;
	char *s;
};

char *d_to_s(int i, char *c, uint8_t sz) 
{

	//keep shifting the next digit to the units place
	//convert unit to ascii and store in the buffer


	uint8_t index = sz-1;
	int d = i;

	//d is always 0-9
	while(d) {
		c[index--] = (char) (d%10 + '0');
		d = d/10;
	}

	//store an inverted string from the end and just
	//return the point where the string should start
	return &c[index+1];
}

char *x_to_s(int i, char *c, uint8_t sz) 
{

	//keep shifting the next digit to the units place
	//convert unit to ascii and store in the buffer


	uint8_t index = sz-1;
	int res, d = i;

	//d is always 0-9
	while(d) {
		res = 0xF & d;	
		if (res < 0xa)
			c[index--] = (char) (res + '0');
		else
			c[index--] = (char) ((res-0xa) + 'A');

		d = d >> 4;
	}

	//store an inverted string from the end and just
	//return the point where the string should start
	return &c[index+1];
}

int vprintf(const char *format, va_list args)
{
	//keep printing characters until you encounter
	//a special symbol (%*). 
	//
	//Replace it with the first arg in the args stack.
	//
	//Each arg type will require processing it in a
	//different way (for instance, number will need
	//converstion to ASCII chars)

	char *c = format;
	union data arg;
	char buf[D_BUF_SZ];

	while(*c) {
		/*
		 * Possibly a formatted option, check
		 * which format it is
		 */
		if (*c == '%' ) {
			c++;
			switch (*c++) {
				case 's':
					arg.s = va_arg(args, char*);
					puts(arg.s);
					break;
				case 'c':
					arg.c = va_arg(args, char);
					putc(arg.c);
					break;
				case 'd':
					arg.i = va_arg(args, int);
					puts(d_to_s(arg.i, buf, D_BUF_SZ));
					break;
				case 'x':
					arg.i = va_arg(args, int);
					puts(x_to_s(arg.i, buf, D_BUF_SZ));
					break;
				default:
					puts("Format option not supported\n");
			}
			continue;
		}

		putc(*c++);
	}

	return 0;
}
