#ifndef __MMIO_H_
#define __MMIO_H_

#include "lib/stdlib.h"
#include "lib/pt_lib.h"


#define u64	uint64_t
#define u32	uint32_t
#define u16	uint16_t
#define u8	uint8_t

#define write_reg(reg_addr, reg_size, val) \
	*(u##reg_size *) reg_addr = val

#define clear_reg(reg_addr, reg_size) \
	*(u##reg_size *) reg_addr = 0

#define read_reg(reg_addr, reg_size) \
	*(u##reg_size *) reg_addr


void get_rpi_version(void);
void *ioremap (uint64_t addr);
void mmio_init(void);
void mmio_write(uint32_t offset, uint32_t val);
void mmio_write_long(uint64_t offset, uint64_t val);
uint32_t mmio_read(uint32_t offset);
uint64_t mmio_read_long(uint64_t offset);

#endif
