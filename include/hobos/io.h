#include <stdint.h>
#include "asm/barrier.h"

uint32_t iowrite32(uint32_t *addr, uint32_t val)
{
	dma_mb();
	WRITE_ONCE(*addr, val);
}


uint32_t ioread32(uint64_t addr)
{
	dma_mb();
	return READ_ONCE(addr);
}
