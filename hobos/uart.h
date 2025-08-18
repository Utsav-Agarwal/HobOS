#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include "gpio.h"

extern uint64_t mmio_base;

/*
 * NOTE: This currently only implements mini UART - which is supported upto
 * RPI 4
 *
 * For RPI 5, PLL01 UART is needed, which is currently being worked on.
 * */

#define AUX_IO_BASE 0x215000

#define AUX_IO_REG(offset) \
	(mmio_base + AUX_IO_BASE + offset)

#define AUX_ENABLES     0x04
#define AUX_MU_IO       0x40
#define AUX_MU_IER      0x44
#define AUX_MU_IIR      0x48
#define AUX_MU_LCR      0x4C
#define AUX_MU_MCR      0x50
#define AUX_MU_LSR      0x54
#define AUX_MU_MSR      0x58
#define AUX_MU_SCRATCH  0x5C
#define AUX_MU_CNTL     0x60
#define AUX_MU_STAT     0x64
#define AUX_MU_BAUD     0x68

//Clock assumed to be 250MHz for 115200 Baud
#define MINI_UART_BAUD 270

void mini_uart_init(struct gpio_controller *ctrl);
void mini_uart_putc(char c);
void mini_uart_puts(char *c);

/*
 *
 * This is PL011 UART, supported by all RPI models.
 *
 */

extern uint64_t uart0_base;

#define RPI_LEGACY_UART0_BASE	0x20100
#define RPI_5_UART0_BASE	0x30000

#define UART0_REG(offset) \
	(mmio_base + uart0_base + offset)


#define DR		0x00
#define FR		0x18
#define IBRD		0x24
#define FBRD		0x28
#define CR		0x30
#define IFLS		0x34
#define IMSC		0x38
#define RIS		0x3c
#define MIS		0x40
#define ICR		0x44
#define DMACR		0x48

void uart_init(void);
void uart_putc(char c);
void uart_puts(char *c);
char uart_getc(void);

#endif
