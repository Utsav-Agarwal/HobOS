#ifndef __MUTEX_H
#define __MUTEX_H

#include <stdbool.h>
#include <stdint.h>

#define LOCKED			1
#define UNLOCKED		0

#define MUTEX(x)		mutex_t	x
#define MUTEX_VECTOR(x, n)	mutex_t	x[n]

/* This needs to be 64 bits, else 
 * writes trample multiple entries due to 
 * 64 bit function addresses 
 * */
typedef uint64_t		mutex_t;

extern void lock_mutex (mutex_t *mutex);
extern void unlock_mutex (mutex_t *mutex);
extern mutex_t get_mutex_state (mutex_t *mutex);

#endif
