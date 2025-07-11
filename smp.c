#include "hobos/smp.h"
#include "hobos/mutex.h"

extern int curr_core_id(void);
extern int setup_stack(void);

uint64_t *cpu_spin_table = (uint64_t *) SPIN_TABLE_BASE;


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
}

/* Note that this does not guarantee that the processor
 * will start executing the given function straightaway.
 *
 * It only updates the spintable function address the 
 * processor uses (see park_and_wait).
 * */
int queue_on_proc(uint64_t fn_addr, uint8_t cpu_id)
{

	if (cpu_id >= MAX_CORES)
		return -1;

	if (!cpu_id) {
		((void (*)(void)) fn_addr)();
		return 0;
	}

	cpu_spin_table[cpu_id] = fn_addr;
	__asm__ volatile("dsb sy; sev");
	return 0;
}


void park_and_wait (void)
{
	uint8_t core_id = curr_core_id();

	while (1) {
		if (cpu_spin_table[core_id]) 
			( (void (*)(void)) cpu_spin_table[core_id])();

		/* suspend and wait for event */
		__asm__ volatile ("wfe");
	}
}
