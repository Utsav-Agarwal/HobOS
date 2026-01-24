/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __TIMER_H
#define	__TIMER_H

#include "mmio.h"
#include "timer/bcm2835.h"

#include <stdbool.h>

//use the SoC timer by default for now
extern struct timer global_timer;

struct timer {
	unsigned long base;		//base addr
	unsigned int msb;		//Most Sig 32 bits
	unsigned int lsb;		//Least Sig 32 bits

	int (*set_timer_freq_div)(unsigned div);

	void (*set_timer_val32)(bool ms, unsigned val, struct timer *t);
	void (*set_timer_val64)(unsigned long val, struct timer *t);	//sets MS+LS

	unsigned (*read_timer32)(bool msb, struct timer *t);	//returns LS
	unsigned long (*read_timer64)(struct timer *t);

	//TODO:maybe declare weak impl?
	void (*enable_interrupts)(struct timer *t);
	void (*disable_interrupts)(struct timer *t);

	void (*set_timer)(struct timer *t, unsigned val);
	void (*reset_timer)(struct timer *t);

	void *plat_feats;
};

void init_timer(struct timer *t);
unsigned read_timer(bool msb, struct timer *t);

#endif
