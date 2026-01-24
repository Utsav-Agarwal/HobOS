/* SPDX-License-Identifier: GPL-2.0-only */

#include "asm/barrier.h"

void iowrite8(unsigned char *addr, unsigned val);
void iowrite16(unsigned char *addr, unsigned val);
void iowrite32(unsigned char *addr, unsigned val);
void iowrite64(unsigned char *addr, unsigned val);

unsigned char ioread8(unsigned char *addr);
unsigned short ioread16(unsigned char *addr);
unsigned int ioread32(unsigned char *addr);
unsigned long ioread64(unsigned char *addr);
