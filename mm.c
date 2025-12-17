#include "hobos/lib/stdlib.h"

void *io_remap(uint64_t addr, uint32_t size) 
{
//TODO: 
//1) traverse the page table and check if this
//address has been already mapped or not. 
//2a) if already in use, return null.
//2b) if available, create mapping(s) and return the vaddr
//associated to the first physical pointer.
//3) make sure the mapping contains nGnRnE appropriate flags
}

void *malloc (uint32_t size) 
{
//TODO: 
//1) traverse the page table and create
//a new set of physical pages/blocks mappings. 
//2) return the vaddr associated with them.
}

void free(uint64_t addr)
{
//TODO
//1. remove given address from current page table
//2. invalidate the particular tlb entry for this variable
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
