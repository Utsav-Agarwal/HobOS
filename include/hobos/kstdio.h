/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __KSTDIO_H
#define __KSTDIO_H

#include <hobos/chardev.h>
#include <hobos/gpio.h>
#include <hobos/lib/vsprintf.h>
#include <hobos/mmio.h>
#include <hobos/nostdlibc_arg.h>

extern u8 rpi_version;

void init_console(struct char_device *console, void *priv);

int kprintf(const char *format, ...);

void puts(char *c);
void putc(char c);
char getc(void);

#endif
