#include "hobos/smp.h"
#include "hobos/kstdio.h"

extern int curr_core_id(void);
extern int setup_stack(void);

uint64_t *cpu_spin_table = (uint64_t *) SPIN_TABLE_BASE;

//TODO: We need a mutex here to preserve a read/write order
//for the spin table

int init_smp(void)
{	
	int i=0;

	for ( i=0; i<MAX_CORES; i++)
		run_process((uint64_t) setup_stack, i);
}

int run_process(uint64_t fn_addr, uint8_t cpu_id)
{

	if (cpu_id >= MAX_CORES)
		return -1;
	
	cpu_spin_table[cpu_id] = fn_addr;
	__asm__ volatile("dsb sy");
	return 0;
}


//TODO:
//if we attempt to clear or set the spin table, we might
//lose a read or write incoming from another processor.
//
//This is why we need a mutex before queueing more than
//one processors
//
//This current implementation works, just about, with the 
//current implementation, but has no guarantees.
void park_and_wait (void)
{
	uint8_t core_id = curr_core_id();

	while (1) {
		if (cpu_spin_table[core_id]) 
			( (void (*)(void)) cpu_spin_table[core_id])();

		__asm__ volatile ("wfe");
	}
}
