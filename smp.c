#include "hobos/smp.h"
#include "hobos/kstdio.h"

extern int curr_core_id(void);

uint64_t *cpu_spin_table = (uint64_t *) SPIN_TABLE_BASE;

//TODO: We need a mutex here to preserve a read/write order
//for the spin table

int run_process(uint64_t fn_addr, uint8_t cpu_id)
{

	if (cpu_id > MAX_REMOTE_CORE_ID)
		return -1;
	
	cpu_spin_table[cpu_id] = fn_addr;
	
	//data sync barrier
	__asm__ volatile("dsb sy");
	__asm__ volatile("sev");

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
	
		//data sync barrier
		__asm__ volatile("dsb sy");
		
		if (cpu_spin_table[core_id]) 
			( (void (*)(void)) cpu_spin_table[core_id])();

		__asm__ volatile ("wfe");
	}
}
