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
#define	GPFSEL0 0x200000 //0-9
#define	GPFSEL1 0x200004 //10-19
#define	GPFSEL2 0x200008 //20-29
#define	GPFSEL3 0x20000C //30-39
#define	GPFSEL4 0x200010 //40-49
#define	GPFSEL5 0x200014 //50-59

#define GPF_IN 0x000
#define GPF_OUT 0x001

//NOTE: For pin 0, please hardcode value
#define GPF_CFG(pin, val) \
		val << ((pin * 3) - 1)

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
#define GPSET0 0x20001C
#define GPSET1 0x200020

#define GPSET_CFG(pin_nr) \
	pin_nr

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
#define	GPCLR0 0x200028
#define	GPCLR1 0x20002C

//TODO: implement feature for runtime detection
inline uint8_t get_rpi_version(void)
{
	return 3;
}

inline void mmio_write(uint32_t offset, uint32_t val)
{
	*(volatile uint32_t *)(mmio_base + offset) = val;
}

inline uint32_t mmio_read(uint32_t offset, uint32_t val)
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

void clear_gpio(uint8_t pin_nr)
{
	int clr_bank = 0;

	if (pin_nr <= 31)
		clr_bank = GPCLR0;
	else
		clr_bank = GPCLR1;

	mmio_write(clr_bank, BITP(pin_nr));
}

//true = output, false = input
void set_gpio_dir(uint8_t pin_nr, uint8_t dir)
{
	uint16_t gpf_val = GPF_CFG(pin_nr, dir ? GPF_OUT : GPF_IN);
	uint32_t bank_addr = GPFSEL0 + (4 * (pin_nr/10));

	mmio_write(bank_addr, gpf_val);
}

void set_gpio(uint8_t pin_nr, uint8_t val)
{
	//clear gpio
	clear_gpio(pin_nr);
	
	//set gpio to output
	set_gpio_dir(pin_nr, val);
	
	//set val
	if (val) {
		if (pin_nr > 31)
			mmio_write(GPSET1, BITP(pin_nr));
		else
			mmio_write(GPSET0, BITP(pin_nr));
	}

}

#endif
