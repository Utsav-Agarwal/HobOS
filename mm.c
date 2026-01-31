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

	map_pa_to_va_pg(addr, vaddr, global_page_tables[0], PTE_FLAGS_NC);

	return (void *)vaddr;
}

void *kmalloc(unsigned int size)
{
	//TODO: write a memory allocator
	unsigned int pages = size / PAGE_SIZE;

	return page_alloc(pages);
}

void kfree(unsigned long addr)
{
//TODO: impl depends on kmalloc
}

void memcpy(void *dst, void *src, unsigned int size)
{
	int i;

	for (i = 0; i < size; i++)
		*((char *)dst + i) = *((char *)src + i);
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
