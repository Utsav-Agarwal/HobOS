#ifndef __THREAD_H
#define __THREAD_H

#include <stdint.h>

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
*/

struct thread {
	uint8_t tid;
	uint8_t core_id;

	uint64_t fn_addr;
	uint64_t *args;
	uint64_t *result;

	uint8_t completed;
};

struct wk_queue {
	//LIST(tasks)
};

struct thread_queue {
	//LIST(threads)
};

void queue_thread(void);
void check_thread_status(void);

#endif
