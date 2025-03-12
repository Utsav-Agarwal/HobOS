#include "hobos/gpio.h"

uint64_t gpio_base;
extern uint8_t rpi_version;

inline void init_gpio(void)
{
	if (rpi_version == 5)
		gpio_base = RPI_5_GPIO_BASE;
	else
		gpio_base = RPI_LEGACY_GPIO_BASE;
}

inline void clear_gpio(uint8_t pin_nr)
{
	if (rpi_version < 5) {
		WRITE_REG(GPIO_REG(GPCLR_BANK(pin_nr)), 32, BITP(pin_nr));
		return;
	}
}

//true = output, false = input
inline void set_gpio_func(uint8_t pin_nr, uint32_t val)
{
	if (rpi_version < 5) {
		REG(GPIO_REG(GPFSEL_BANK(pin_nr)), 32) |= GPF_CFG(pin_nr, val);
		return;
	}
}

void set_gpio_val(uint8_t pin_nr)
{
	if (rpi_version < 5) {
		REG((GPIO_REG(GPSET0 + 4*(pin_nr/32))),32) |= BITP(pin_nr);
		return;
	}
}
