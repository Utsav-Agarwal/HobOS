#ifndef __BCM2836_TIMER
#define	__BCM2836_TIMER

// 64-bit core timer 
#define	CTRL				0x00
#define TIMER_PRESCALER			0x08

#define	CORE_TIMER_ACCESS_LS		0x1C
#define	CORE_TIMER_ACCESS_MS		0x20

#define LOCAL_TIMER_CONTROL_STAT	0x34
#define LOCAL_TIMER_WRITE		0x38

//timers
#define	CORE0_TIMER_CTRL		0x40
#define	CORE1_TIMER_CTRL		0x44
#define	CORE2_TIMER_CTRL		0x48
#define	CORE3_TIMER_CTRL		0x4C


struct bcm2835_timer {
	void (*set_timer_inc) (bool inc_by_2);
	void (*set_timer_src) (bool apb);	
};


#endif
