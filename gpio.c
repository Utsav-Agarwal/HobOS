#include "hobos/gpio.h"

extern uint8_t rpi_version;

uint32_t gpio_reg(uint32_t base, uint32_t offset)
{
	return mmio_base + base + offset;
}

static inline void generic_clear_gpio(uint8_t pin_nr, uint32_t base)
{
	write_reg(gpio_reg(base, GPCLR_BANK(pin_nr)), 32, BITP(pin_nr));
}

/* true = output, 
 * false = input */
static inline void generic_set_gpio_func(uint8_t pin_nr, uint32_t val, uint32_t base)
{
	read_reg(gpio_reg(base, GPFSEL_BANK(pin_nr)), 32) |= GPF_CFG(pin_nr, val);
}

static inline void generic_set_gpio_val(uint8_t pin_nr, uint32_t base)
{
	read_reg((gpio_reg(base, GPSET0 + 4*(pin_nr/32))),32) |= BITP(pin_nr);
}


void init_gpio(struct gpio_controller *ctrl)
{
	switch (rpi_version) {
		case 5 :
		/*gpio functionality is not fully supported yet*/
			ctrl->base = BCM2712_GPIO_BASE;
			return;
		case 3:
			ctrl->base = BCM2835_GPIO_BASE;
			break;
		default:
			ctrl->base = 0;
			return;
	}

	/* generic functions */
	ctrl->ctrl_set_gpio_fn = generic_set_gpio_func;
	ctrl->ctrl_set_gpio_val = generic_set_gpio_val;
	ctrl->ctrl_clear_gpio = generic_clear_gpio;
}

/*
 * This is the generic API that is meant to be used by other drivers.
 *
 * The only goal of these functions is to offer a convenient way to 
 * call the controller specific callbacks. The controller specific 
 * callbacks can be customized by initializing the structure with custom
 * functions after initializing it.
 * */
void inline gpio_set_fn(uint8_t pin_nr, uint32_t val, struct gpio_controller *ctrl)
{
	ctrl->ctrl_set_gpio_fn(pin_nr, val, ctrl->base);
}

void inline gpio_clear(uint8_t pin_nr, struct gpio_controller *ctrl)
{
	ctrl->ctrl_clear_gpio(pin_nr, ctrl->base);
}

void inline gpio_set(uint8_t pin_nr, struct gpio_controller *ctrl)
{
	ctrl->ctrl_set_gpio_val(pin_nr, ctrl->base);
}
