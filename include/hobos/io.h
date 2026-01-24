/* SPDX-License-Identifier: GPL-2.0-only */

#include "asm/barrier.h"

void iowrite8(unsigned char *addr, unsigned val);
void iowrite16(unsigned char *addr, unsigned val);
void iowrite32(unsigned char *addr, unsigned val);
void iowrite64(unsigned char *addr, unsigned val);

unsigned char ioread8(unsigned long addr);
unsigned short ioread16(unsigned long addr);
unsigned int ioread32(unsigned long addr);
unsigned long ioread64(unsigned long addr);
