// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/lib/pt_lib.h>
#include <hobos/lib/stdlib.h>
#include <hobos/mmio.h>
#include <hobos/mmu.h>
#include <hobos/page_alloc.h>
#include <hobos/slub.h>

extern void kernel_panic(void);

void *ioremap(unsigned long addr)
{
	void *vaddr;

	vaddr = map_pa_to_va_pg(addr, addr,
				&global_page_tables[0], PTE_FLAGS_NC, 0);

	if (!vaddr)
		kernel_panic();

	return vaddr;
}

/*
 * Return contiguous memory of len = size bytes
 */
void *kmalloc(unsigned int size)
{
	unsigned int pages = (size / PAGE_SIZE);

	if (pages) {
		/* If more than 1 page, just add another page to it */
		pages += !!(size % PAGE_SIZE);
		return page_alloc(pages);
	}

	return slub_alloc(size);
}

/*
 * Use for larger memory sizes which dont need to be contiguous
 */
void vmalloc(unsigned int size)
{
	// TODO
}

void kfree(void *p)
{
	page_free(p);
}

void strcpy(void *dst, void *src)
{
	char *s = (char *)src;
	char *d = (char *)dst;

	while (*s != '\0')
		*d++ = *s++;
}

void memcpy(void *dst, void *src, unsigned int size)
{
	char *s = (char *)src;
	char *d = (char *)dst;
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
