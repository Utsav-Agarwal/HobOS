/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __STDLIB_H
#define __STDLIB_H

#define BITP(pos) 	(1UL << (pos))
#define BITM(pos)	((BITP(pos)) - 1UL)

void *kmalloc(unsigned size);
void kfree(void *p);

void strcpy(void *dst, void *src);
void memcpy(void *dst, void *src, unsigned size);
void memset(void *buf, const char c, unsigned size);

void delay(unsigned ticks);

extern struct char_device uart_dev;
extern void enable_global_interrupts(void);

#endif
