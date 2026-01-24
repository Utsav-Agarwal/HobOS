#include <stdint.h>
#include "asm/barrier.h"

void iowrite32(unsigned *addr, uint32_t val)
{
	dma_mb();
	WRITE_ONCE(*addr, val);
}

uint32_t ioread32(unsigned long addr)
{
	dma_mb();
	return READ_ONCE(addr);
}
