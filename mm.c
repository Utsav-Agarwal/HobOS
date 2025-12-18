#include "hobos/lib/pt_lib.h"
#include "hobos/lib/stdlib.h"
#include "hobos/mmu.h"

extern struct page_table_desc *global_page_tables[10];

void *ioremap(uint64_t addr) 
{
	//TODO: do not assume which page table is being used
	//currently we will assume that all operations are being done
	//on kernel page table.
	map_pa_to_va_pg(addr, addr, global_page_tables[0]);
	
	return (void *) (addr + KERNEL_START);

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
