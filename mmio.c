#include "hobos/mmio.h"

uint8_t rpi_version;
uint64_t mmio_base;

inline void get_rpi_version(void)
{
	unsigned int reg;

	/* read the system register */
#if __aarch64__
	asm volatile ("mrs %x0, midr_el1" : "=r" (reg));
#else
	asm volatile ("mrc p15,0,%0,c0,c0,0" : "=r" (reg));
#endif

	switch ((reg >> 4) & 0xFFF) {
		case 0xB76: 
			rpi_version=1;
			break;
		case 0xC07: 
			rpi_version=2;
			break;
		case 0xD03: 
			rpi_version=3; 
			break;
		case 0xD08: 
			rpi_version=4; 
			break;
		case 0xD0B:
			rpi_version=5;
		default: 
			rpi_version=0;
			break;
	}
}

void mmio_write(uint32_t offset, uint32_t val)
{
	*(volatile uint32_t *)(mmio_base + offset) = val;
}

uint32_t mmio_read(uint32_t offset)
{
	return *(volatile uint32_t *)(mmio_base + offset);
}

void mmio_write_long(uint64_t offset, uint64_t val)
{
	*(volatile uint64_t *)(mmio_base + offset) = val;
}

uint64_t mmio_read_long(uint64_t offset)
{
	return *(volatile uint64_t *)(mmio_base + offset);
}

void mmio_init(void)
{
	get_rpi_version();
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
