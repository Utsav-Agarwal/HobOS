#include "hobos/smp.h"
#include "hobos/mmu.h"
#include "hobos/asm/barrier.h"
#include "hobos/kstdio.h"

extern void setup_stack(void);
struct worker smp_worker[MAX_REMOTE_CORE_ID+1];
struct worker_job smp_worker_jobs[MAX_REMOTE_CORE_ID+1]; //just one job per 
							 //proc for now
struct jobs_meta smp_worker_jobs_meta[MAX_REMOTE_CORE_ID+1]; //only need 1

static void pop_worker_job(struct worker_job *jobs)
{
	struct worker_job *n = jobs->next;
	struct jobs_meta *meta = jobs->meta;

	meta->head = n;
	n->meta = meta;
	
	n->job_pos = JOBS_HEAD;
	jobs = n;
}

//TODO: sort out memory mamangement for malloc
//TODO: add generic list helpers
//NOTE: assumes queue is not empty
static void push_worker_job(struct worker_job *jobs, struct worker_job *n)
{
	//we assume that the last job will always be followed by park_and_wait.
	struct worker_job *tail = jobs->meta->tail;
	struct jobs_meta *meta = jobs->meta;

	
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
//that have been queued on it
void __park_and_wait (void);
static void worker_process(void)
{
	uint8_t core = curr_core_id();
	struct worker *w = &smp_worker[core]; 
	void (*execute) (void);

	//TODO: Add locking to queue so that it can be dynamically
	//be modified by another core. This would result in us not having
	//to go back to __park_and_wait. This will be critical once processes
	//come into the picture.
	while (1) {
		execute = (void (*)(void)) (w->jobs->fn_addr);
		execute();

		if (w->jobs->next)
			pop_worker_job(w->jobs);
		else
			break;
	}

	__park_and_wait();
}

static void set_job_queue_head(struct worker_job *job, uint64_t fn_addr)
{
	//TODO:
	//job.job_meta = malloc(sizeof(struct jobs_meta));

	job->job_pos = JOBS_HEAD;
	job->fn_addr = fn_addr;
	job->next = 0;

	job->meta->head = job;
	job->meta->tail = job;
}

static void __init_worker(uint8_t core)
{
	uint64_t *spin_table = (volatile uint64_t *) SPIN_TABLE_BASE;
	struct worker *w = &smp_worker[core];

	w->exec_addr = &spin_table[core];
	w->jobs = &smp_worker_jobs[core];
	w->core_id = core;
	w->mutex[0] = 0;
	w->mutex[1] = 0;
}

//kickstart the core to flush the job queue
static int __run_core(uint8_t core)
{
	if (core > MAX_REMOTE_CORE_ID)
	    return -1;
	
	struct worker *w = &smp_worker[core];
	volatile uint32_t *m0 = &w->mutex[0];
	volatile uint32_t *m1 = &w->mutex[1];
	uint64_t *exec_addr = w->exec_addr;

	//we need some sync/ordering here since 2 processors
	//are competing to write to a common memory location (semaphores[..])
	//i.e - the scheduler(master) and the worker(slave)
	acquire_mutex(m0); 
	smp_store_mb(*exec_addr, (uint64_t) worker_process); 
	acquire_mutex(m1);

	asm volatile("sev");
	return 0;
}

static int __setup_core(uint8_t core)
{
	if (core > MAX_REMOTE_CORE_ID)
	    return -1;
	
	struct worker *w = &smp_worker[core];
	volatile uint32_t *m0 = &w->mutex[0];
	volatile uint32_t *m1 = &w->mutex[1];
	volatile uint64_t *exec_addr = w->exec_addr;

	//we need some sync/ordering here since 2 processors
	//are competing to write to a common memory location (semaphores[..])
	//i.e - the scheduler(master) and the worker(slave)
	acquire_mutex(m0); 
	smp_store_mb(*exec_addr, (uint64_t) setup_stack); 
	acquire_mutex(m1);

	asm volatile("sev");
	return 0;
}

//core has nothing to do, so sleep
//TODO: maybe we can hardcode now instead of using trigger?
void __park_and_wait (void)
{

	uint8_t core = curr_core_id();
	struct worker *w = &smp_worker[core];
	volatile uint64_t *exec_addr = w->exec_addr;
	volatile uint32_t *m0 = &w->mutex[0];
	volatile uint32_t *m1 = &w->mutex[1];
	void (*trigger) (void);

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

static void smp_heartbeat()
{
	kprintf("core %d up and running!\n", curr_core_id());
}

void init_smp(void)
{
	int i;

	for(i=1; i<=MAX_REMOTE_CORE_ID; i++) {
		smp_worker_jobs[i].meta = &smp_worker_jobs_meta[i];
		smp_worker_jobs[i].meta = &smp_worker_jobs_meta[i];
		set_job_queue_head(&smp_worker_jobs[i], 
				(uint64_t) smp_heartbeat);
		__init_worker(i);
		__setup_core(i);
		__run_core(i);
	}
}

//TODO:
uint64_t smp_switch_vmem(void);
uint64_t smp_jump_to_EL1(void);
