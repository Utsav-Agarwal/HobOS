#ifndef __KSTDIO_H
#define __KSTDIO_H

void init_console(void);

int kprintf(const char *format, ...);

void puts(const char *c);
void putc(const char c);
char getc(void);

#endif
