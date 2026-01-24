// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/io.h>
#include <stdint.h>

void iowrite8(unsigned char *addr, unsigned val)
{
	dma_mb();
	WRITE_ONCE(*(volatile unsigned char *)addr, val);
}

void iowrite16(unsigned char *addr, unsigned val)
{
	volatile unsigned short *x = (volatile unsigned short *)(addr);

	dma_mb();
	WRITE_ONCE(*x, val);
}

void iowrite32(unsigned char *addr, unsigned val)
{
	volatile unsigned int *x = (volatile unsigned int *)(addr);

	dma_mb();
	WRITE_ONCE(*x, val);
}

void iowrite64(unsigned char *addr, unsigned val)
{
	volatile unsigned long *x = (volatile unsigned long *)(addr);

	dma_mb();
	WRITE_ONCE(*x, val);
}

unsigned char ioread8(unsigned char *addr)
{
	dma_mb();
	return *(volatile unsigned char *)addr;
}

unsigned short ioread16(unsigned char *addr)
{
	dma_mb();
	return *(volatile unsigned short *)addr;
}

unsigned int ioread32(unsigned char *addr)
{
	dma_mb();
	return *(volatile unsigned int *)addr;
}

unsigned long ioread64(unsigned char *addr)
{
	dma_mb();
	return *(volatile unsigned long *)addr;
}
