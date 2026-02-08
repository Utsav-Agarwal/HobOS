// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/lib/stdlib.h>
#include <hobos/nostdlibc_arg.h>
#include <hobos/kstdio.h>

#define D_BUF_SZ	128

union data {
	int i;
	char c;
	char *s;
};

char *d_to_s(int i, char *c, char sz)
{
	/*
	 * keep shifting the next digit to the units place
	 * convert unit to ascii and store in the buffer
	 */

	int index = sz + 2;
	int d = i;

	c[index--] = '\0';
	//d is always 0-9
	while (index) {
		c[index--] = (char)(d % 10 + '0');
		d = d / 10;
	}

	/*
	 * store an inverted string from the end and just
	 * return the point where the string should start
	 */
	return &c[index + 1];
}

char *x_to_s(int i, char *c, char sz)
{
	/*
	 * keep shifting the next digit to the units place
	 * convert unit to ascii and store in the buffer
	 */
	int index = sz;
	u64 res;
	u64 d = i;

	c[index--] = '\0';
	//d is always 0-9
	while (index > 1) {
		res = 0xF & d;
		if (res < 0xa)
			c[index--] = (char)(res + '0');
		else
			c[index--] = (char)((res - 0xa) + 'A');

		d = d >> 4;
	}

	/*
	 * A hex value should always be appended by '0x'
	 */
	c[index--] = 'x';
	c[index--] = '0';

	/*
	 * store an inverted string from the end and just
	 * return the point where the string should start
	 */
	return &c[index + 1];
}

static int int_chars(int d)
{
	int size = 0;
	int i = d * 10;

	while (i) {
		i = i / 10;
		size++;
	}

	return size - 2;
}

int vprintf(const char *format, va_list args)
{
	/*
	 * keep printing characters until you encounter
	 * a special symbol (%*).
	 *
	 * Replace it with the first arg in the args stack.
	 *
	 * Each arg type will require processing it in a
	 * different way (for instance, number will need
	 * converstion to ASCII chars)
	 */

	const char *c = format;
	union data arg;
	char buf[D_BUF_SZ];
	int nr_chars = 0;

	memset(buf, 0, D_BUF_SZ);
	while (*c) {
		/*
		 * Possibly a formatted option, check
		 * which format it is
		 */
		if (*c == '%') {
			c++;
			switch (*c++) {
			case 's':
				arg.s = va_arg(args, char*);
				puts(arg.s);
				break;
			case 'c':
				arg.c = va_arg(args, int);
				putc(arg.c);
				break;
			case 'd':
				arg.i = va_arg(args, int);
				nr_chars = int_chars(arg.i);
				puts(d_to_s(arg.i, buf, nr_chars));
				break;
			case 'x':
				arg.i = va_arg(args, int);
				puts(x_to_s(arg.i, buf, 8 + 2));
				break;
			case 'l':
				arg.i = va_arg(args, int);
				puts(x_to_s(arg.i, buf, 16 + 2));
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
