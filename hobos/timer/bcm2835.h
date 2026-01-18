#ifndef __BCM2835_TIMER
#define	__BCM2835_TIMER

/* BCM2835 - rpi3b */


/* Since we are not compiling selectively, all these definitions
 * may end up being redefined. So we need to find a way to overload
 * them since multiple platforms may have similar registers at different addresses. 
 * Using platform specific macros to retrieve them allows easy 
 * access */
#define BCM2835(reg) BCM2835_TIMER_##reg

/* SYSTEM TIMER */
#define	BCM2835_TIMER_BASE_OFF		0x3000
#define	BCM2835_TIMER_MSB		0x3004
#define	BCM2835_TIMER_LSB		0x3008

#define	BCM2835_TIMER_C0		0x300c
#define	BCM2835_TIMER_C1		0x3010
#define	BCM2835_TIMER_C2		0x3014
#define	BCM2835_TIMER_C3		0x3018


#endif
