#include "hobos/gpio.h"
#include "hobos/lib/stdlib.h"
#include "hobos/uart.h"

uint64_t uart_base;
extern uint64_t rpi_version;

/* Mini UART */
void mini_uart_init(struct gpio_controller *ctrl)
{
	
	init_gpio(ctrl);

	/* enable and halt uart peripheral */
	read_reg(gpio_reg(ctrl->base, GPFSEL1), 32) &= ~(BITP(14)|BITP(15));
	read_reg(AUX_IO_REG(AUX_ENABLES), 8) |= 0x1;
	clear_reg(AUX_IO_REG(AUX_MU_CNTL), 8); 

	/* Disable all features for now */
	clear_reg(AUX_IO_REG(AUX_MU_IER), 8);
	clear_reg(AUX_IO_REG(AUX_MU_MCR), 8);

	/* set baud and enable 8 bit data */
	//NOTE: for some reason BITP(2) also needs to be set in LCR
	write_reg(AUX_IO_REG(AUX_MU_LCR), 8, 0x3); 
	write_reg(AUX_IO_REG(AUX_MU_BAUD), 16, MINI_UART_BAUD); 
	
	/* get GPIO pins ready */
	gpio_set_fn(14, GPF_ALT0, ctrl);
	gpio_set_fn(15, GPF_ALT0, ctrl);

	clear_reg(gpio_reg(ctrl->base, GPPUD), 32);
	delay(150);
	read_reg(gpio_reg(ctrl->base, GPPUDCLK0), 32) |= ((BITP(14) | BITP(15)));
	delay(150);
	clear_reg(gpio_reg(ctrl->base, GPPUDCLK0), 32);

	/* enable transmit and recieve */
	write_reg(AUX_IO_REG(AUX_MU_CNTL), 8, 0x3);
}

static void mini_uart_wait_for_idle(void)
{
	/* wait for transmitter to idle */
	while(!(read_reg(AUX_IO_REG(AUX_MU_LSR), 8) & BITP(6)))
		asm volatile("nop");
}

void mini_uart_putc(char c)
{
	mini_uart_wait_for_idle();
	write_reg(AUX_IO_REG(AUX_MU_IO), 8, c);

}

void mini_uart_puts(char *c)
{
	char *s = c;
	while(*s)
		mini_uart_putc(*s++);
}


/* PL011 UART */
uint64_t uart0_base;

void uart_init(void)
{
	if (rpi_version == 5) 
		uart0_base = RPI_5_UART0_BASE;
	else
		uart0_base = RPI_LEGACY_UART0_BASE;

	//TODO: enable uart init implementation
 
	/* 
	* NOTE: Current implementation is not complete and is assisted
	* by the EEPROM enable_rp1_uart option.
	*/

}

static void uart_wait_for_idle(uint8_t tx_rx)
{
	if (tx_rx)
		while (read_reg(UART0_REG(FR), 8) & BITP(5)); //TX is FULL
	else
		while (read_reg(UART0_REG(FR), 8) & BITP(4)); //RX is EMPTY

}

void uart_putc(char c)
{
	uart_wait_for_idle(1);
	write_reg(UART0_REG(DR), 8, c);
}

void uart_puts(char *c)
{
	char *s = c;
	
	while(*s)
		uart_putc(*s++);
}

char uart_getc(void)
{
	uart_wait_for_idle(0);
	return read_reg(UART0_REG(DR), 8);
}

