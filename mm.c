#include "hobos/lib/pt_lib.h"
#include "hobos/lib/stdlib.h"
#include "hobos/mmio.h"
#include "hobos/mmu.h"

extern struct page_table_desc *global_page_tables[10];

void *ioremap(uint64_t addr) 
{
	//all of this is going to be before switching to high memory addressing
	//so we dont mind if we get an identity mapped pointer.
	//
	//TODO: make it so that its compatible after high mem switch has taken
	//place so we can add data during runtime (loadable drivers for instace).
	uint64_t vaddr = addr;

	map_pa_to_va_pg(addr, vaddr, global_page_tables[0], PTE_FLAGS_NC);
	
	return (void *) vaddr;

}

void *kmalloc (uint32_t size) 
{
//TODO: write a memory allocator 
}

void kfree(uint64_t addr)
{
//TODO: impl depends on kmalloc
}

void memcpy (void *dst, void *src, uint32_t size)
{
	int i;

	for (i=0; i<size; i++)
		*((char *)dst + i) = *((char *)src + i);
}

void memset (void *buf, const char c, uint32_t size) 
{
	int i;

	for (i=0; i<size; i++)
		*((char *)buf + i) = c;
}

void delay(uint32_t ticks) 
{
	while (ticks--) 
		asm volatile("nop"); 
}
