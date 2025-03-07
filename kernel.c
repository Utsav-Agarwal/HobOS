#include <stddef.h>
#include <stdint.h>
#include "hobos/mmio.h"
#include "hobos/gpio.h"

extern void set_gpio(uint8_t pin_nr, uint8_t dir);

static inline void delay(int32_t count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

/* I'm alive */
void heartbeat(void)
{
	set_gpio_set_func(12, GPF_OUT);
	set_gpio_val(12, 1);
}

void main()
{
	int rpi_version = 0;

	rpi_version = get_rpi_version();
	mmio_init(rpi_version);
	while (1)
		heartbeat();
}
