// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/lib/pt_lib.h>
#include <hobos/lib/stdlib.h>
#include <hobos/mmio.h>
#include <hobos/mmu.h>
#include <hobos/page_alloc.h>

void *ioremap(unsigned long addr)
{
	//all of this is going to be before switching to high memory addressing
	//so we dont mind if we get an identity mapped pointer.
	//
	//TODO: make it so that its compatible after high mem switch has taken
	//place so we can add data during runtime (loadable drivers for instace)
	u64 vaddr = addr;

	map_pa_to_va_pg(addr, vaddr, global_page_tables[0], PTE_FLAGS_NC, 0);

	return (void *)vaddr;
}

void *kmalloc(unsigned int size)
{
	//TODO: Add support for scatterlists
	unsigned int pages = size / PAGE_SIZE;

	return page_alloc(pages);
}

void kfree(void *p)
{
	page_free(p);
}

void strcpy(void *dst, void *src)
{
	volatile char *s = (volatile char *)src;
	volatile char *d = (volatile char *)dst;

	while (*s != '\0')
		*d++ = *s++;
}

void memcpy(void *dst, void *src, unsigned int size)
{
	volatile char *s = (volatile char *)src;
	volatile char *d = (volatile char *)dst;
	int i;

	for (i = 0; i < size; i++)
		*(d + i) = *(s + i);
}

void memset(void *buf, const char c, unsigned int size)
{
	int i;

	for (i = 0; i < size; i++)
		*((char *)buf + i) = c;
}

void delay(unsigned int ticks)
{
	while (ticks--)
		asm volatile("nop");
}
