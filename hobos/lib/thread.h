#ifndef __THREAD_H
#define __THREAD_H

#include "task.h"
#include <stdint.h>
#include "../kstdio.h"

/* 
 * We want simple thread implementation
 * 
 * 1) A task is created and added to workqueue
 * 2) When the requested core is available, create a
 *    thread and send it over to be executed
 * 3) if the core is not available, move to other tasks
 *    that can be sent over
 * 
 * This concept assumes that core 0 takes the place
 * of a dedicated thread scheduler for now
 * 
 * Execution is done at a first come first serve basis
 * since currently there is no wait for IO which might
 * suspend execution
 * 
 * TODO: suspend / migration
 * */

#define	ANY_CORE	0xFF

#define INACTIVE	0xFF

#define PRINT_THREAD(t)	kprintf("tid:%d\ncore:%d\nfn:%x\n", t->tid, t->core_id, t->fn_addr);

struct thread_struct {
	uint8_t tid;
	uint8_t core_id;

	uint64_t fn_addr;
//	uint64_t *args;
//	uint64_t *result;
//	TODO: Add more meta data

};


void init_threading(void);
uint8_t incoming_thread_exists(uint8_t core_id);
void queue_thread(struct task_struct *tsk);
void check_thread_status(void);

#endif
