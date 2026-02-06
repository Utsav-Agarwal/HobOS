/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef MUTEX_H
#define MUTEX_H

#include <hobos/asm/barrier.h>

inline void acquire_mutex(volatile unsigned int *lock)
{
	unsigned int tmp, val;

	val = 1;
	asm volatile("1: ldaxr  %w0, [%2]\n"    // load current value
		"   cbnz   %w0, 2f\n"      // if != 0, somebody else holds it
		"   stxr   %w0, %w1, [%2]\n" // try to store 1
		"   cbnz   %w0, 1b\n"      // if store failed, retry
		"   b      3f\n"           // acquired
		"2: wfe\n"                 // contended: sleep
		"   b      1b\n"           // and retry
		"3:\n"
		: "=&r"(tmp), "+r"(val)
		: "r"(lock)
		: "memory");
}

//at this point we already own the mutex, so we dont really care
//about exclusive access for writing. It will only be contested after
//being released
inline void release_mutex(volatile unsigned int *m)
{
	asm volatile("stlr %w1, %0;\n"
		     "sev;\n"
		     : "=Q"(*(m))
		     : "r"(0)
		     : "memory");
}

#endif
