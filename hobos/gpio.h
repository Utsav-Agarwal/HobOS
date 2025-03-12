#ifndef __GPIO_H
#define __GPIO_H

#include "mmio.h"

extern uint64_t gpio_base;
extern uint64_t mmio_base;

#define	RPI_LEGACY_GPIO_BASE 0x200000
#define	RPI_5_GPIO_BASE 0xd0000

#define GPIO_REG(offset) \
	(mmio_base + gpio_base + offset) 

/*
 * Legacy GPIO
 *
 * NOTE: This interface is valid for all RPI devices
 * up till rpi 5, which uses a different IP
 */

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

#define GPF_IN		0x00
#define GPF_OUT 	0x01
#define GPF_ALT0 	0x04
#define GPF_ALT1 	0x05
#define GPF_ALT2 	0x06
#define GPF_ALT3 	0x07
#define GPF_ALT4 	0x03
#define GPF_ALT5 	0x02

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
#define GPSET0		0x1C
#define GPSET1		0x20

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
#define	GPCLR0		0x28
#define	GPCLR1		0x2C

/*
 * GPIO pull up/down
 * */
#define GPPUD		0x94

/* GPIO Pull up/down actuation */
#define GPPUDCLK0	0x98
#define GPPUDCLK1	0x9C

//there are <32 GPIO for 3b+
#define GPCLR_BANK(pin) \
	(GPCLR0 + 4*(pin/32))

//each config register can only hold 10 GPIO configs
#define GPF_CFG(pin, val) \
	(val << ((pin%10) * 3))

#define GPFSEL_BANK(pin) \
	(GPFSEL0 + (4 * (pin_nr/10)))


/*
 * RPI 5 GPIO
 */

#define RP1_NR_GPIO_PINS	27

#define RP1_GPIO_STATUS_ADDR(pin_nr) \
	(gpio_base + 0x08*pin_nr) 

#define RP1_GPIO_CTRL_ADDR(pin_nr) \
	(RP1_GPIO_STATUS_ADDR(pin_nr) + 0x04) 

#define RP1_RW_OFFSET			0x0000
#define RP1_XOR_OFFSET			0x1000
#define RP1_SET_OFFSET			0x2000
#define RP1_CLR_OFFSET			0x3000

#define RP1_GPIO_CTRL_FUNCSEL_LSB	0
#define RP1_GPIO_CTRL_FUNCSEL_MASK	0x0000001f
#define RP1_GPIO_CTRL_OUTOVER_LSB	12
#define RP1_GPIO_CTRL_OUTOVER_MASK	0x00003000
#define RP1_GPIO_CTRL_OEOVER_LSB	14
#define RP1_GPIO_CTRL_OEOVER_MASK	0x0000c000
#define RP1_GPIO_CTRL_INOVER_LSB	16
#define RP1_GPIO_CTRL_INOVER_MASK	0x00030000
#define RP1_GPIO_CTRL_IRQEN_FALLING	BIT(20)
#define RP1_GPIO_CTRL_IRQEN_RISING	BIT(21)
#define RP1_GPIO_CTRL_IRQEN_LOW		BIT(22)
#define RP1_GPIO_CTRL_IRQEN_HIGH	BIT(23)
#define RP1_GPIO_CTRL_IRQEN_F_FALLING	BIT(24)
#define RP1_GPIO_CTRL_IRQEN_F_RISING	BIT(25)
#define RP1_GPIO_CTRL_IRQEN_F_LOW	BIT(26)
#define RP1_GPIO_CTRL_IRQEN_F_HIGH	BIT(27)
#define RP1_GPIO_CTRL_IRQRESET		BIT(28)
#define RP1_GPIO_CTRL_IRQOVER_LSB	30
#define RP1_GPIO_CTRL_IRQOVER_MASK	0xc0000000


/*
 * RP1 uses the RIO interface to manipulate GPIO
 *
 * NOTE: RIO1 and RIO2 are reserved
 * */
#define RP1_SYS_RIO0			0x10000

#define RIO_REG(offset) \
	(GPIO_REG(RP1_SYS_RIO0) + offset)

#define RP1_RIO_OUT			0x00
#define RP1_RIO_OE			0x04
#define RP1_RIO_IN			0x08

void init_gpio(void);
void clear_gpio(uint8_t pin_nr);
void set_gpio_func(uint8_t pin_nr, uint32_t val); 
void set_gpio_val(uint8_t pin_nr);

#endif
