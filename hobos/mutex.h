#ifndef __MUTEX_H
#define __MUTEX_H

#include <stdbool.h>
#include <stdint.h>

#define LOCKED		1
#define UNLOCKED	0

#define MUTEX(x)	mutex_t	x;

typedef uint8_t	mutex_t;

extern void lock_mutex (mutex_t *mutex);
extern void unlock_mutex (mutex_t *mutex);
extern mutex_t get_mutex_state (mutex_t *mutex);

#endif
