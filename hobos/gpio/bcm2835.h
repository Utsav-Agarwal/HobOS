#ifndef __BCM2835_GPIO_H
#define __BCM2835_GPIO_H

/*
 * Legacy GPIO
 *
 * NOTE: This interface is valid for all RPI devices
 * up till rpi 5, which uses a different IP
 */

#define	BCM2835_GPIO_BASE 0x200000

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


#endif
