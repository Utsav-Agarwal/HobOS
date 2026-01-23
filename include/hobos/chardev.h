#ifndef CHARDEV_H
#define CHARDEV_H

#include <stdint.h>

struct char_device {
	uint64_t *base;	
	void *priv;	
	void (*init) (void *priv);
	void (*quirks) (void *priv);
	void (*putc) (char c);
	void (*puts) (char *c);
	char (*getc) (void);
};
 
void puts(char *s);
void putc(char c);
char getc(void);

#endif
