// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/io.h>

void iowrite8(unsigned char *addr, unsigned val)
{
	dma_mb();
	WRITE_ONCE(*addr, val);
}

void iowrite16(unsigned char *addr, unsigned val)
{
	unsigned short *x = (unsigned short *)(addr);

	dma_mb();
	WRITE_ONCE(*x, val);
}

void iowrite32(unsigned char *addr, unsigned val)
{
	unsigned int *x = (unsigned int *)(addr);

	dma_mb();
	WRITE_ONCE(*x, val);
}

void iowrite64(unsigned char *addr, unsigned val)
{
	unsigned long *x = (unsigned long *)(addr);

	dma_mb();
	WRITE_ONCE(*x, val);
}

unsigned char ioread8(unsigned long addr)
{
	dma_mb();
	return READ_ONCE(addr);
}

unsigned short ioread16(unsigned long addr)
{
	dma_mb();
	return READ_ONCE(addr);
}

unsigned int ioread32(unsigned long addr)
{
	dma_mb();
	return READ_ONCE(addr);
}

unsigned long ioread64(unsigned long addr)
{
	dma_mb();
	return READ_ONCE(addr);
}
