#ifndef __MMIO_H_
#define __MMIO_H_

#include <stdint.h>

#define BITP(pos) (1 << pos)

#define u64	uint64_t
#define u32	uint32_t
#define u16	uint16_t
#define u8	uint8_t

#define WRITE_REG(reg_addr, reg_size, val) \
	*(u##reg_size *) reg_addr = val

#define REG(reg_addr, reg_size) \
	*(u##reg_size *) reg_addr

#define CLEAR_REG(reg_addr, reg_size) \
	*(u##reg_size *) reg_addr = 0

void get_rpi_version(void);
void mmio_write(uint32_t offset, uint32_t val);
uint32_t mmio_read(uint32_t offset);
void mmio_init(void);
void delay(uint32_t count);

#endif
