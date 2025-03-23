#ifndef __KSTDIO_H
#define __KSTDIO_H

void init_console(void);

int kprintf(const char *format, ...);

void puts(char *c);
void putc(char c);
char getc(void);

#endif
