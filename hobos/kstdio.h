#ifndef __KSTDIO_H
#define __KSTDIO_H
#include "gpio.h"

void init_console(struct gpio_controller *ctrl);

int kprintf(const char *format, ...);

void puts(char *c);
void putc(char c);
char getc(void);

#endif
