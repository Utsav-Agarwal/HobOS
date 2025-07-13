#ifndef __TASK_H
#define __TASK_H

#include <stdint.h>

/* We want simple thread implementation
 *
 * 1: A task is created and added to workqueue
 * 2: When the requested core is available, create a
 * thread and send it over to be executed
 */

#define TASK(tsk, core_id, fn_addr) \
    struct task_struct tsk = { \
        .core_id = core_id, \
        .fn_addr = (uint64_t)(fn_addr) \
    } 


struct task_struct {
	uint8_t tid;		//thread associated to task
	uint8_t core_id; 	//any preferred core id

	uint64_t fn_addr; 
//	uint64_t *args;
//	uint64_t *result

	uint8_t completed;
};

struct wk_queue {
	//LIST(tasks)
};

void queue_task(struct task_struct *tsk);

#endif
