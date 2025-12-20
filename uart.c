#include "hobos/uart.h"
#include "hobos/asm/barrier.h"

extern uint64_t rpi_version;

static void uart_init(void *priv);

struct char_device uart_dev = {
    .init = uart_init,
};

static inline uint64_t uart_reg(uint32_t offset)
{
	return ((uint64_t)uart_dev.base + offset);	
}


//TODO: return error codes
/* Mini UART */
static void mini_uart_init(void *priv)
{
	
	struct gpio_controller *ctrl = (struct gpio_controller *)priv;

	//gpio not initialized
	if (!ctrl->base)
		return;

	/* enable and halt uart peripheral */
	read_reg(gpio_reg((uint64_t)ctrl->base, GPFSEL1), 32) &= ~(BITP(14)|BITP(15));
	read_reg(uart_reg(AUX_ENABLES), 8) |= 0x1;
	clear_reg(uart_reg(AUX_MU_CNTL), 8); 

	/* Disable all features for now */
	clear_reg(uart_reg(AUX_MU_IER), 8);
	clear_reg(uart_reg(AUX_MU_MCR), 8);

	/* set baud and enable 8 bit data */
	//NOTE: for some reason BITP(2) also needs to be set in LCR
	write_reg(uart_reg(AUX_MU_LCR), 8, 0x3); 
	write_reg(uart_reg(AUX_MU_BAUD), 16, MINI_UART_BAUD); 
	
	/* get GPIO pins ready */
	gpio_set_fn(14, GPF_ALT0, ctrl);
	gpio_set_fn(15, GPF_ALT0, ctrl);

	clear_reg(gpio_reg((uint64_t)ctrl->base, GPPUD), 32);
	delay(150);
	read_reg(gpio_reg((uint64_t)ctrl->base, GPPUDCLK0), 32) |= ((BITP(14) | BITP(15)));
	delay(150);
	clear_reg(gpio_reg((uint64_t)ctrl->base, GPPUDCLK0), 32);

	/* enable transmit and recieve */
	write_reg(uart_reg(AUX_MU_CNTL), 8, 0x3);
}

static void mini_uart_wait_for_idle(void)
{
	/* wait for transmitter to idle */
	while(!(read_reg(uart_reg(AUX_MU_LSR), 8) & BITP(6))) 
		asm volatile("nop");
}

static void mini_uart_putc(char c)
{
	mini_uart_wait_for_idle();
	dma_mb();
	write_reg(uart_reg(AUX_MU_IO), 8, c);
	dma_mb();

}

static void mini_uart_puts(char *c)
{
	char *s = c;
	while(*s)
		mini_uart_putc(*s++);
}

static char mini_uart_getc(void)
{
	//TODO
	return 0;
}

static void uart_wait_for_idle(uint8_t tx_rx)
{
	if (tx_rx)
		while (read_reg(uart_reg(FR), 8) & BITP(5)); //TX is FULL
	else
		while (read_reg(uart_reg(FR), 8) & BITP(4)); //RX is EMPTY

}

static void uart_putc(char c)
{
	uart_wait_for_idle(1);
	write_reg(uart_reg(DR), 8, c);
}

static void uart_puts(char *c)
{
	char *s = c;
	
	while(*s)
		uart_putc(*s++);
}

static char uart_getc(void)
{
	uart_wait_for_idle(0);
	return read_reg(uart_reg(DR), 8);
}

static void uart_init(void *priv)
{
	uint64_t base;

        if (rpi_version == 5) {
    		base = RPI_5_UART0_BASE;
    		
    		uart_dev.getc = uart_getc;
    		uart_dev.putc = uart_putc;
    		uart_dev.puts = uart_puts;
        } else {
    		base = AUX_IO_BASE;
    		uart_dev.priv = priv;
    
    		uart_dev.quirks = mini_uart_init;
    		uart_dev.getc = mini_uart_getc;
    		uart_dev.putc = mini_uart_putc;
    		uart_dev.puts = mini_uart_puts;
        }

    	uart_dev.base = (uint64_t *)ioremap(base + mmio_base);

	/* 
	* NOTE: Current implementation is not complete and is assisted
	* by the EEPROM enable_rp1_uart option.
	*/
}

