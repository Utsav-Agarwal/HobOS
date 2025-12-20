#ifndef __GPIO_H
#define __GPIO_H

#include "chardev.h"
#include "mmio.h"
#include "gpio/bcm2712.h"
#include "gpio/bcm2835.h"

extern uint64_t gpio_base;
extern uint64_t mmio_base;

struct gpio_controller {
	uint64_t *base;

	void (*ctrl_set_gpio_fn) (uint8_t pin_nr, uint32_t val, uint64_t base);
	void (*ctrl_clear_gpio)  (uint8_t pin_nr, uint64_t base);
	void (*ctrl_set_gpio_val)(uint8_t pin_nr, uint64_t base);
};


//TODO: maybe static?
void init_gpio(struct gpio_controller *ctrl);
void gpio_set_fn(uint8_t pin_nr, uint32_t val, struct gpio_controller *ctrl);
void gpio_clear(uint8_t pin_nr, struct gpio_controller *ctrl);
void gpio_set(uint8_t pin_nr, struct gpio_controller *ctrl);

/* generic helpers */
uint64_t gpio_reg(uint64_t base, uint32_t offset);

#endif
