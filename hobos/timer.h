#ifndef __TIMER_H
#define	__TIMER_H

#include "timer/bcm2835.h"

#include <stdbool.h>
#include <stdint.h>
#include "mmio.h"

struct timer {
	uint32_t base;		//base addr
	uint32_t msb;		//Most Sig 32 bits
	uint32_t lsb;		//Least Sig 32 bits

	int (*set_timer_freq_div) (uint32_t div);

	void (*set_timer32) (bool ms, uint32_t val, struct timer *t);	//sets LS
	void (*set_timer64) (uint64_t val, struct timer *t);		//sets MS+LS

	uint32_t (*read_timer32) (bool msb, struct timer *t);	//returns LS
	uint64_t (*read_timer64) (struct timer *t);	//returns MS+LS
	
	void *plat_feats;	//platform specific features which might not be available
				//on other platforms due to different IPs
};

void init_timer(struct timer *t);
uint32_t read_timer(bool msb, struct timer *t);

#endif
