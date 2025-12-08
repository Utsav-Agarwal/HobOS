#ifndef SMP_H
#define SMP_H

#include <stdint.h>
#include "asm/barrier.h"
#include "asm/mutex.h"

/*
 * RPI FW initializes the remote processors using a spin table
 * where CPU IDs 1-3 are put to sleep using a spin table
 * */

//TODO: Add error codes
//TODO: Add mailbox support for reading processor messages

#define SPIN_TABLE_BASE		0xD8
#define MAX_REMOTE_CORE_ID	3

extern uint32_t volatile *semaphores; 
extern uint32_t volatile *m_fn_addr; 

//we essentially want every function to block its spot while executing and
//then cleanup after itself. This wrapper does it.
#define smp_wrapper_name(fn)	smp_ps_wrapper_##fn

//we just make sure the function to be threaded was scheduled
#define smp_process(fn, core)	\
void smp_wrapper_name(fn##core) (void)	\
{								\
	fn();							\
	wmb();							\
	__park_and_wait();					\
}								

#define smp_run_process(fn, core) 					\
	do {								\
		__run_process((uint64_t) smp_wrapper_name(fn##core), core);	\
	} while(0)

int __run_process(uint64_t fn_addr, uint8_t core);
void __park_and_wait (void);

#endif
