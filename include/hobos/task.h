/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef PROCESS_H
#define PROCESS_H

#include <hobos/lib/pt_lib.h>
#include <hobos/compiler_types.h>

extern pid_t pid_cntr;

struct ctxt {
	u64 x[21];
};

//TODO: maybe consider shared memory processes
struct task {
	pid_t pid;
	struct page_table_desc *base_pt;		//memory map
	struct ctxt ctxt;				//context
};

//TODO: we need to make sure proc_ctxt is not stored on stack
//since stack is per task/ctxt - it changes.
//
//in short - we need a working malloc which allocates
//memory in kernel before using this
void save_curr_context(struct ctxt *proc_ctxt);
void resume_from_context(struct ctxt *proc_ctxt);

#endif
