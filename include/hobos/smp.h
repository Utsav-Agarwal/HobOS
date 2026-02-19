/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SMP_H
#define SMP_H

#include <hobos/asm/barrier.h>
#include <hobos/asm/mutex.h>

/*
 * RPI FW initializes the remote processors using a spin table
 * where CPU IDs 1-3 are put to sleep using a spin table
 * */

#define SPIN_TABLE_BASE		0xD8
#define MAX_REMOTE_CORE_ID	3

#define JOBS_HEAD	0xFA
#define JOBS_TAIL	0xAF

struct jobs_meta {
	struct worker_job *head;
	struct worker_job *tail;
};

struct worker_job {
	void *fn_addr;
	struct worker_job *next;
	struct jobs_meta *meta;	//only applicable for head
	unsigned char job_pos;
};

struct worker {
	unsigned char core_id;
	mutex_t mutex[2];
	volatile unsigned long *exec_addr;  //spin table entry
	struct worker_job *jobs;
};

int curr_core_id(void);

void init_smp(void);
void smp_switch_vmem(void);
void smp_init_mmu(void);

#endif
