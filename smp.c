// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/barrier.h>
#include <hobos/kstdio.h>
#include <hobos/lib/stdlib.h>
#include <hobos/mmu.h>
#include <hobos/smp.h>

extern void setup_stack(void);
extern void jump_to_EL1(void);

static struct worker smp_worker[MAX_REMOTE_CORE_ID + 1];

static inline void pop_worker_job(struct worker *w)
{
	struct worker_job *head = w->jobs;
	struct worker_job *next_job = head->next;
	struct jobs_meta *meta = head->meta;

	next_job->meta = meta;
	meta->head = next_job;

	next_job->job_pos = JOBS_HEAD;
	w->jobs = next_job;
	//TODO: free all mallocs
}

static struct worker_job *create_job(void *fn)
{
	struct worker_job *job = kmalloc(sizeof(*job));

	if (!job) {
		kprintf("Failed to create job!\n");
		return 0;
	}

	memset(job, 0, sizeof(struct worker_job));
	job->fn_addr = fn;

	return job;
}

static void push_worker_job(struct worker_job *head, void *fn)
{
	//we assume that the last job will always be followed by park_and_wait.
	struct jobs_meta *meta = head->meta;
	struct worker_job *n, *tail;

	if (!meta) {
		kprintf("Could not find queue metadata!\n");
		return;
	}

	n = create_job(fn);
	tail = meta->tail;
	tail->next = n;

	//make sure its not a single element list
	if (tail->job_pos != JOBS_HEAD)
		tail->job_pos = 0;

	n->job_pos = JOBS_TAIL;
	meta->tail = n;
}

//For now, we assume that there is no context switching taking place
//and all execution is completed before moving to the other

//lets leave the responsibility for context swtich to yield(). Once yield()-ed,
//the job saves its context in the process structure and essentially completes
//itself, exiting out of the job queue. It now needs to enter again through the
//process pool.

//essentially the entire purpose of this worker process is to flush the jobs
//that have been queued on it. Let yield be called with an interrupt and the job will
//be flushed.
void __park_and_wait(void);
static void worker_process(void)
{
	u8 core = curr_core_id();
	struct worker *w = &smp_worker[core];
	void (*execute)(void);

	//TODO: Add locking to queue so that it can be dynamically
	//be modified by another core. This would result in us not having
	//to go back to __park_and_wait. This will be critical once processes
	//come into the picture.

	while (1) {
		execute = (void (*)(void)) (w->jobs->fn_addr);
		execute();

		if (!w->jobs->next)
			break;

		pop_worker_job(w);
	}

	__park_and_wait();
}

static void set_job_queue_head(struct worker_job **job, void *fn_addr)
{
	*job = create_job(fn_addr);
	struct jobs_meta *meta;

	(*job)->meta = kmalloc(sizeof(*meta));
	meta = (*job)->meta;
	if (!meta) {
		kprintf("meta malloc failed!\n");
		return;
	}

	(*job)->job_pos = JOBS_HEAD;
	meta->head = *job;
	meta->tail = *job;
}

static void __init_worker(unsigned char core, struct worker_job *jobs)
{
	volatile u64 *spin_table = (volatile unsigned long *)SPIN_TABLE_BASE;
	struct worker *w = &smp_worker[core];

	w->exec_addr = &spin_table[core];
	w->jobs = jobs;
	w->core_id = core;
	w->mutex[0] = 0;
	w->mutex[1] = 0;
}

//kickstart the core to flush the job queue
static int __run_core(unsigned char core)
{
	if (core > MAX_REMOTE_CORE_ID)
	return -1;

	struct worker *w = &smp_worker[core];
	mutex_t *m0 = &w->mutex[0];
	mutex_t *m1 = &w->mutex[1];
	volatile u64 *exec_addr = w->exec_addr;

	//we need some sync/ordering here since 2 processors
	//are competing to write to a common memory location (semaphores[..])
	//i.e - the scheduler(master) and the worker(slave)
	acquire_mutex(m0);
	smp_store_mb(*exec_addr, (unsigned long)worker_process);
	acquire_mutex(m1);

	asm volatile("sev");
	return 0;
}

static int __setup_core(int core)
{
	if (core > MAX_REMOTE_CORE_ID)
	return -1;

	struct worker *w = &smp_worker[core];
	mutex_t *m0 = &w->mutex[0];
	mutex_t *m1 = &w->mutex[1];

	//we need some sync/ordering here since 2 processors
	//are competing to write to a common memory location (semaphores[..])
	//i.e - the scheduler(master) and the worker(slave)
	acquire_mutex(m0);
	smp_store_mb(*w->exec_addr, (unsigned long)setup_stack);
	acquire_mutex(m1);

	asm volatile("sev");
	return 0;
}

//core has nothing to do, so sleep
//TODO: maybe we can hardcode now instead of using trigger?
void __park_and_wait(void)
{
	u8 core = curr_core_id();
	struct worker *w = &smp_worker[core];
	volatile u64 *exec_addr = w->exec_addr;
	mutex_t *m0 = &w->mutex[0];
	mutex_t *m1 = &w->mutex[1];
	void (*trigger)(void);

	release_mutex(m1);
	release_mutex(m0);
	//keep waiting until you see something new
	while (1) {
		while (!READ_ONCE(*m1))
			asm volatile ("wfe");

		trigger = (void (*)(void)) *(exec_addr);
		trigger();
	}
}

void init_smp(void)
{
	struct worker_job *jobs;
	int core;

	for (core = 1; core <= MAX_REMOTE_CORE_ID; core++) {
		set_job_queue_head(&jobs, jump_to_EL1);
		push_worker_job(jobs, init_mmu);
		push_worker_job(jobs, switch_vmem);

		__init_worker(core, jobs);
		if (__setup_core(core))
			kprintf("Core %d setup failed!\n", core);

		if (__run_core(core))
			kprintf("Core %d startup failed!\n", core);
	}
}
