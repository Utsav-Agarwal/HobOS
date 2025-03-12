#include "hobos/mmio.h"

uint8_t rpi_version;
uint64_t mmio_base;

inline void get_rpi_version(void)
{
	rpi_version = 3;
}

inline void mmio_write(uint32_t offset, uint32_t val)
{
	*(volatile uint32_t *)(mmio_base + offset) = val;
}

inline uint32_t mmio_read(uint32_t offset)
{
	return *(volatile uint32_t *)(mmio_base + offset);
}

void mmio_init(void)
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

inline void delay(uint32_t count)
{
	while(count--) {asm volatile("nop");}
}

