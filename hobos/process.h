#ifndef PROCESS_H
#define PROCESS_H

#include "lib/pt_lib.h"

//callee-saved registers
#define X(n)	x[n-19]
#define FP	X(29)
#define LR	X(30)

struct ctxt {
	uint64_t x[12];
	uint64_t sp;
};

//TODO: maybe consider shared memory processes
struct process {
	uint32_t pid;
	uint64_t tcr;				//save mmu config
	struct page_table_desc *pt_baddr;	//page tables
	struct ctxt *proc_state;			//context
};


//TODO: we need to make sure proc_ctxt is not stored on stack
//since stack is per task/ctxt - it changes.
//
//in short - we need a working malloc which allocates
//memory in kernel before using this
extern void save_curr_context(struct ctxt *proc_ctxt);
extern void resume_from_context(struct ctxt *proc_ctxt);

#endif
