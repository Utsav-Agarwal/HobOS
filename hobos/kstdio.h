#ifndef __KSTDIO_H
#define __KSTDIO_H
#include "gpio.h"
#include "chardev.h"
#include "lib/vsprintf.h"
#include "mmio.h"
#include "nostdlibc_arg.h"

void init_console(struct char_device *console, void *priv);

int kprintf(const char *format, ...);

void puts(char *c);
void putc(char c);
char getc(void);

#endif
