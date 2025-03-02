#include <stddef.h>
#include <stdint.h>
#include "hobos/mmio.h"

extern void set_gpio(uint8_t pin_nr, uint8_t dir);

static inline void delay(int32_t count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

/* I'm alive */
void heartbeat(void)
{
	set_gpio_dir(8, 1);
	set_gpio_val(8, 1);

	set_gpio_dir(18, 1);
	set_gpio_val(18, 1);
}

void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
	int rpi_version = 0;

	rpi_version = get_rpi_version();
	mmio_init(rpi_version);
	while (1)
		heartbeat();
}
