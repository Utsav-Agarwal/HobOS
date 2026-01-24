/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef CHARDEV_H
#define CHARDEV_H


struct char_device {
	unsigned long *base;
	void *priv;
	void (*init)(void *priv);
	void (*quirks)(void *priv);
	void (*putc)(char c);
	void (*puts)(char *c);
	char (*getc)(void);
};

void puts(char *s);
void putc(char c);
char getc(void);

#endif
