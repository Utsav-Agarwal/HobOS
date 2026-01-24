#ifndef __TIMER_H
#define	__TIMER_H

#include "mmio.h"
#include "timer/bcm2835.h"

#include <stdbool.h>
#include <stdint.h>

//use the SoC timer by default for now
extern struct timer global_timer;

struct timer {
	unsigned long base;		//base addr
	unsigned msb;		//Most Sig 32 bits
	unsigned lsb;		//Least Sig 32 bits

	int (*set_timer_freq_div)(uint32_t div);

	void (*set_timer_val32)(bool ms, uint32_t val, struct timer *t);
	void (*set_timer_val64)(unsigned long val, struct timer *t);	//sets MS+LS

	uint32_t (*read_timer32)(bool msb, struct timer *t);	//returns LS
	unsigned long (*read_timer64)(struct timer *t);

	//TODO:maybe declare weak impl?
	void (*enable_interrupts)(struct timer *t);
	void (*disable_interrupts)(struct timer *t);

	void (*set_timer)(struct timer *t, uint32_t val);
	void (*reset_timer)(struct timer *t);

	void *plat_feats;
};

void init_timer(struct timer *t);
uint32_t read_timer(bool msb, struct timer *t);

#endif
