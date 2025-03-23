//This is the memory manager
/*
 * This is responsible for virtual memory:
 * 1) Making sure stack is not overwritten by heap
 * 2) track heap/stack
 * 3) Manage dynamic memory allocation (memory fragmentation)
 * */

#include "hobos/lib/stdlib.h"

uint64_t __heap_start;
uint64_t __heap_size;
uint64_t __heap_end;


void *vm_remap(uint64_t addr);


//TODO: Add a list to keep track of all memory transactions
// This is required when it comes to managing multiple frees

//increment address to the next size
//all byte addressable
void *malloc (uint32_t size) 
{
	uint64_t malloc_start;

	if (!__heap_end)
		__heap_end = __heap_start;
	
	malloc_start  = __heap_end;
	__heap_end += size;

	if (__heap_end > (__heap_start + __heap_size))
		return 0;

	return (void *) malloc_start;
}

//dont need to do anything for now
void free(uint64_t addr)
{
	//TODO: Free last memory transaction
	//from the list
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
