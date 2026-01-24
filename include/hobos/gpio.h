/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __GPIO_H
#define __GPIO_H

#include "chardev.h"
#include "gpio/bcm2712.h"
#include "gpio/bcm2835.h"
#include "mmio.h"

struct gpio_controller {
	unsigned long *base;

	void (*ctrl_set_gpio_fn)(char pin_nr, unsigned val, unsigned long base);
	void (*ctrl_clear_gpio)(char pin_nr, unsigned long base);
	void (*ctrl_set_gpio_val)(char pin_nr, unsigned long base);
};

//TODO: maybe static?
void init_gpio(struct gpio_controller *ctrl);
void gpio_set_fn(char pin_nr, unsigned val, struct gpio_controller *ctrl);
void gpio_clear(char pin_nr, struct gpio_controller *ctrl);
void gpio_set(char pin_nr, struct gpio_controller *ctrl);

/* generic helpers */
unsigned long gpio_reg(unsigned long base, unsigned offset);

#endif
