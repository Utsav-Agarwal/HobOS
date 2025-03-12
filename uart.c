#include "hobos/uart.h"
#include "hobos/gpio.h"

uint64_t uart_base;
extern uint64_t rpi_version;

/* Mini UART */
void mini_uart_init(void)
{
	/* enable and halt uart peripheral */
	REG(GPIO_REG(GPFSEL1), 32) &= ~(BITP(14)|BITP(15));
	REG(AUX_IO_REG(AUX_ENABLES), 8) |= 0x1;
	CLEAR_REG(AUX_IO_REG(AUX_MU_CNTL), 8); 

	/* Disable all features for now */
	CLEAR_REG(AUX_IO_REG(AUX_MU_IER), 8);
	CLEAR_REG(AUX_IO_REG(AUX_MU_MCR), 8);

	/* set baud and enable 8 bit data */
	//NOTE: for some reason BITP(2) also needs to be set in LCR
	WRITE_REG(AUX_IO_REG(AUX_MU_LCR), 8, 0x3); 
	WRITE_REG(AUX_IO_REG(AUX_MU_BAUD), 16, MINI_UART_BAUD); 
	
	/* get GPIO pins ready */
	set_gpio_func(14, GPF_ALT0);
	set_gpio_func(15, GPF_ALT0);

	CLEAR_REG(GPIO_REG(GPPUD), 32);
	delay(150);
	REG(GPIO_REG(GPPUDCLK0), 32) |= ((BITP(14) | BITP(15)));
	delay(150);
	CLEAR_REG(GPIO_REG(GPPUDCLK0), 32);

	/* enable transmit and recieve */
	WRITE_REG(AUX_IO_REG(AUX_MU_CNTL), 8, 0x3);
}

static void mini_uart_wait_for_idle(void)
{
	/* wait for transmitter to idle */
	while(!(REG(AUX_IO_REG(AUX_MU_LSR), 8) & BITP(6)))
		asm volatile("nop");
}

void mini_uart_putc(char c)
{
	mini_uart_wait_for_idle();
	WRITE_REG(AUX_IO_REG(AUX_MU_IO), 8, c);

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
		while (REG(UART0_REG(FR), 8) & BITP(5)); //TX is FULL
	else
		while (REG(UART0_REG(FR), 8) & BITP(4)); //RX is EMPTY

}

void uart_putc(char c)
{
	uart_wait_for_idle(1);
	WRITE_REG(UART0_REG(DR), 8, c);
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
	return REG(UART0_REG(DR), 8);
}

