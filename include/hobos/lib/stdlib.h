#ifndef __STDLIB_H
#define __STDLIB_H

#include <stdint.h>

#define BITP(pos) (1 << pos)
#define BITM(pos) ((BITP(pos)) - 1)

void *kmalloc (uint32_t size);
void free (uint64_t addr);

void memcpy (void *dst, void *src, uint32_t size);
void memset (void *buf, const char c, uint32_t size);

void delay(uint32_t ticks);

#endif
