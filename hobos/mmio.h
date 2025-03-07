#ifndef __HOBOS_H__
#define __HOBOS_H__

uint32_t mmio_base;

#define BITP(pos) (1 << pos)

//TODO: implement feature for runtime detection
inline uint8_t get_rpi_version(void)
{
	return 3;
}

inline void mmio_write(uint32_t offset, uint32_t val)
{
	*(volatile uint32_t *)(mmio_base + offset) = val;
}

inline uint32_t mmio_read(uint32_t offset)
{
	return *(volatile uint32_t *)(mmio_base + offset);
}

inline void mmio_init(int rpi_version)
{
	switch(rpi_version)
	{
		case 2:
		case 3:
			mmio_base = 0x3f000000;
			break;
		case 4:
			mmio_base = 0xfe000000;
			break;
		//RPI 2/3
		default:
			mmio_base = 0x20000000;
			break;
	}
}

#endif
