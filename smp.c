#include "hobos/smp.h"
#include "hobos/mutex.h"
#include "hobos/lib/thread.h"
#include "hobos/kstdio.h"

extern MUTEX_VECTOR(core_incoming_event, MAX_CORES);
extern struct thread_struct active_threads[2*MAX_CORES];

MUTEX_VECTOR(core_spin_entry, MAX_CORES) = {0};
uint64_t *cpu_spin_table = (uint64_t *) SPIN_TABLE_BASE;

//makes sure only 1 fn executes at a time
MUTEX_VECTOR(core_exec, MAX_CORES) = {0};

/* In order to queue a workload we need to go through the following
 * steps:
 *
 * 1) Get mutex to signal incoming event
 * 2) Get mutex to modify the relevant core spin table entry
 * 3) On success, modify the spintable entry (else we block and wait)
 * 4) Release the mutex for the spintable
 * 5) Once the function starts executing, unlock incoming event
 * 6) If no incoming event (mutex is unlocked), get spintable mutex set spintable entry to 0x0
 * 7) Unlock spintable mutex 
 * 8) set spintable entry to 0x0
 * */


int init_smp(void)
{	
	int i=0;

	for ( i=1; i<MAX_CORES; i++)
		queue_on_proc((uint64_t) setup_stack, i);

	return 0;
}

/* Note that this does not guarantee that the processor
 * will start executing the given function straightaway.
 *
 * It only updates the spintable function address the 
 * processor uses (see park_and_wait).
 * */
int queue_on_proc(uint64_t fn_addr, uint8_t core_id)
{
	if (core_id >= MAX_CORES)
		return -1;

	/* This assumes for now that queue_on_proc can
	 * ONLY be called from core 0, otherwise, which makes
	 * it a special case since all ops will be sequential 
	 * */
	if (!core_id) {

		lock_mutex(&core_exec[core_id]);
		unlock_mutex(&core_incoming_event[core_id]);
	
		((void (*)(void)) fn_addr)();
	
		unlock_mutex(&core_exec[core_id]);

		return 0;
	}

	lock_mutex(&core_spin_entry[core_id]);
	cpu_spin_table[core_id] = fn_addr;
	unlock_mutex(&core_spin_entry[core_id]);
	
	/* Wakeup suspended processors, barrier is already there
	 * in mutex functions 
	 * */
	__asm__ volatile("dsb sy; sev");
	return 0;
}

static void cleanup(uint8_t core_id)
{
	if (incoming_thread_exists(core_id))
		return;

	lock_mutex(&core_spin_entry[core_id]);
	cpu_spin_table[core_id] = 0x0;
	unlock_mutex(&core_spin_entry[core_id]);
}

static uint64_t read_spin_table(uint8_t core_id)
{
	uint64_t val;

	lock_mutex(&core_spin_entry[core_id]);
	val = cpu_spin_table[core_id];
	unlock_mutex(&core_spin_entry[core_id]);

	return val;
}

void park_and_wait (void)
{
	uint8_t core_id = curr_core_id();

	cleanup(core_id);
	while (1) {
		if (read_spin_table(core_id) != 0x0) {
			
			/* assumes you must have accquired the incoming event lock,
			 * since now you are in execution, you dont need it anymore,
			 * let others queue */

			lock_mutex(&core_exec[core_id]);
			unlock_mutex(&core_incoming_event[core_id]);
			
			((void (*)(void)) cpu_spin_table[core_id])();
			
			cleanup(core_id);
			unlock_mutex(&core_exec[core_id]);
		}

		/* suspend and wait for event */
		__asm__ volatile ("wfe");
	}
}
