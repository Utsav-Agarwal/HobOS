#include "hobos/smp.h"
#include "hobos/asm/barrier.h"
#include "hobos/kstdio.h"

extern int curr_core_id(void);
uint64_t *cpu_spin_table = (uint64_t *) SPIN_TABLE_BASE;

uint8_t tmp[MAX_REMOTE_CORE_ID+1] = {0}; 
uint8_t *semaphores = tmp;

int __run_process(uint64_t fn_addr, uint8_t cpu_id)
{
	if (cpu_id > MAX_REMOTE_CORE_ID)
	    return -1;

	//TODO: this function needs to indicate back that it has completed
	//execution and wants to complete the task. 
	//Perhaps a lockless acquire/release?
	while (READ_ONCE(semaphores[cpu_id]))
	    asm volatile ("nop");

	smp_store_release(&semaphores[cpu_id], 1); 
	smp_store_mb(cpu_spin_table[cpu_id], fn_addr);

	//wake up processors with "SEND EVENT"
	asm volatile("sev");

	return 0;
}

void __park_and_wait (void)
{
	uint8_t id = curr_core_id();
	void (*trigger) (void);

	kprintf ("core %d waiting\n", id);

	//keep waiting until you see something new
	while (1) {
		while (READ_ONCE(cpu_spin_table[id]) == (uint64_t) __park_and_wait)
			asm volatile ("wfe");

		kprintf("got: %x\n", cpu_spin_table[id]);
		trigger = (void (*)(void))cpu_spin_table[id];
		trigger();
	}
}
