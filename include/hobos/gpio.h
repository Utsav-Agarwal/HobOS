#ifndef __GPIO_H
#define __GPIO_H

#include "chardev.h"
#include "gpio/bcm2712.h"
#include "gpio/bcm2835.h"
#include "mmio.h"

struct gpio_controller {
	unsigned long *base;

	void (*ctrl_set_gpio_fn)(unsigned char pin_nr, uint32_t val, unsigned long base);
	void (*ctrl_clear_gpio)(unsigned char pin_nr, unsigned long base);
	void (*ctrl_set_gpio_val)(unsigned char pin_nr, unsigned long base);
};

//TODO: maybe static?
void init_gpio(struct gpio_controller *ctrl);
void gpio_set_fn(unsigned char pin_nr, uint32_t val, struct gpio_controller *ctrl);
void gpio_clear(unsigned char pin_nr, struct gpio_controller *ctrl);
void gpio_set(unsigned char pin_nr, struct gpio_controller *ctrl);

/* generic helpers */
unsigned long gpio_reg(unsigned long base, uint32_t offset);

#endif
