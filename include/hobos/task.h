/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef PROCESS_H
#define PROCESS_H

#include <hobos/compiler_types.h>
#include <hobos/lib/pt_lib.h>
#include <hobos/slub.h>

extern pid_t pid_cntr;

struct ctxt {
	/* We only care about the Callee-saved registers. Caller-saved
	 * regs would be saved by the caller (or atleast we make that
	 * assumption)
	 *
	 * Look at AAPCS64 for clarification if needed
	 */
	void *sp;
	u64 pad;
	u64 x[12];
	struct kmem_obj *used_kmem;
};

/*
 * Each task needs its own stack. Due to interrupts, another task
 * might just come in and write over the current stack frame completely.
 */
//TODO: maybe consider shared memory processes
struct task {
	pid_t pid;
	struct page_table_desc *base_pt;	//memory map
	struct ctxt *ctxt;			//context
	char stack[8192];			//stack
	int (*pc)(void *data);			//start exec here
	void *data;				//start exec here
	bool running;				//start exec here
	struct task *next;			//tasks are always in a queue
	bool completed;				//completed?
};

//TODO: we need to make sure proc_ctxt is not stored on stack
//since stack is per task/ctxt - it changes.
//
//in short - we need a working malloc which allocates
//memory in kernel before using this
void save_curr_context(struct ctxt *proc_ctxt);
void resume_from_context(struct ctxt *proc_ctxt);
struct task *get_curr_task(void);
void set_curr_task(struct task *t);
int kthread_start(struct task *t);
struct task *kthread_create(int (*thread_fn)(void *data), void *data);
void kthread_init(void);
bool has_completed(struct task *t);
bool is_running(struct task *t);
void task_free(struct task *t);
void kthread_queue(struct task *t);

#endif
