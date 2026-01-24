/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __MMIO_H_
#define __MMIO_H_

#include <hobos/io.h>
#include <hobos/types.h>
#include <hobos/lib/pt_lib.h>

#define write_reg(reg_addr, reg_size, val) \
	iowrite##reg_size((unsigned char *)(reg_addr), val)

#define clear_reg(reg_addr, reg_size) write_reg(reg_addr, reg_size, 0)

#define read_reg(reg_addr, reg_size) \
	ioread##reg_size((unsigned char *) (reg_addr))

extern u8 rpi_version;
extern u64 *mmio_base;

void get_rpi_version(void);
void *ioremap(unsigned long addr);
void mmio_init(void);
void mmio_write(u32 offset, unsigned val);
void mmio_write_long(u64 offset, unsigned long val);
unsigned mmio_read(unsigned offset);
unsigned long mmio_read_long(unsigned long offset);

#endif
