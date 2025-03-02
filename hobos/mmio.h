#ifndef __HOBOS_H__
#define __HOBOS_H__

uint32_t mmio_base;

#define BITP(pos) (1 << pos)

#define	GPIO_BASE 0x200000
	
/*
 *FSELn - Function Select n
 * 000 = GPIO Pin n is an input
 * 001 = GPIO Pin n is an output
 * 100 = GPIO Pin n takes alternate function 0
 * 101 = GPIO Pin n takes alternate function 1
 * 110 = GPIO Pin n takes alternate function 2
 * 111 = GPIO Pin n takes alternate function 3
 * 011 = GPIO Pin n takes alternate function 4
 * 010 = GPIO Pin n takes alternate function 5 
 */
#define	GPFSEL0 0x00 //0-9
#define	GPFSEL1 0x04 //10-19
#define	GPFSEL2 0x08 //20-29
#define	GPFSEL3 0x0C //30-39
#define	GPFSEL4 0x10 //40-49
#define	GPFSEL5 0x14 //50-59

#define GPF_IN 0x0
#define GPF_OUT 0x1


/* No effect when pin is set as input
 * 
 * If pin is set as output:
 * 1 = Set GPIO pin n
 * 
 * Note: Writing 0 is ignored
 *
 * SET0: 0-31
 * SET1: 32-53
 */
#define GPSET0 0x1C
#define GPSET1 0x20

/* No effect when pin is set as input
 * 
 * If pin is set as output:
 * 1 = Clear GPIO pin n
 * 
 * Note: Writing 0 is ignored
 *
 * SET0: 0-31
 * SET1: 32-53
 */
#define	GPCLR0 0x28
#define	GPCLR1 0x2C

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

//there are <32 GPIO for 3b+
void inline clear_gpio(uint8_t pin_nr)
{
	mmio_write(GPIO_BASE + GPCLR0, BITP(pin_nr));
}

//each config register can only hold 10 GPIO configs
#define GPF_CFG(pin, val) \
		(val << ((pin%10) * 3))

//true = output, false = input
void inline set_gpio_dir(uint8_t pin_nr, uint32_t dir)
{
	uint32_t gpf_val = GPF_CFG(pin_nr, dir);
	uint32_t bank_addr = GPIO_BASE + GPFSEL0 + (4 * (pin_nr/10));

	*(volatile uint32_t *)(mmio_base + bank_addr) |= gpf_val;
}

void inline set_gpio_val(uint8_t pin_nr, uint8_t val)
{
	*(volatile uint32_t *)(mmio_base + GPIO_BASE + GPSET0) |= BITP(pin_nr);
}

#endif
