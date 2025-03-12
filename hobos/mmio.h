#ifndef __MMIO_H_
#define __MMIO_H_

#include <stdint.h>

extern uint64_t mmio_base;

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

//TODO: implement feature for runtime detection
extern uint8_t rpi_version;

inline void get_rpi_version(void)
{
	rpi_version = 5;
}

inline void mmio_write(uint32_t offset, uint32_t val)
{
	*(volatile uint32_t *)(mmio_base + offset) = val;
}

inline uint32_t mmio_read(uint32_t offset)
{
	return *(volatile uint32_t *)(mmio_base + offset);
}

//TODO: add rpi 5
inline void mmio_init(uint8_t rpi_version)
{
	switch(rpi_version)
	{
		case 3:
			mmio_base = 0x3f000000;
			break;
		case 4:
			mmio_base = 0xfe000000;
			break;
		case 5:
			//uart offset: 0x30000
			//uart address: 0x107d001000
			//uart address = BAR + offset
			mmio_base = 0x107cfd1000;
			break;
		default:
			mmio_base = 0x20000000;
	}
}

inline void delay(int32_t count)
{
	while(count--) {asm volatile("nop");}
}


#endif
