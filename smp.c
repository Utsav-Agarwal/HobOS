#include "hobos/smp.h"
#include "hobos/asm/barrier.h"
#include "hobos/kstdio.h"

extern int curr_core_id(void);
uint64_t volatile *cpu_spin_table = (uint64_t volatile *) SPIN_TABLE_BASE;

static uint32_t volatile __tmp1[MAX_REMOTE_CORE_ID+1] = {0}; 
uint32_t volatile *semaphores = __tmp1;

static uint32_t volatile __tmp2[MAX_REMOTE_CORE_ID+1] = {0}; 
uint32_t volatile *m_fn_addr = __tmp2;

int __run_process(uint64_t fn_addr, uint8_t core)
{
	if (core > MAX_REMOTE_CORE_ID)
	    return -1;

	
	//we need some sync/ordering here since 2 processors
	//are competing to write to a common memory location (semaphores[..])

	acquire_mutex(&semaphores[core]); 
	smp_store_mb(cpu_spin_table[core], fn_addr);
	acquire_mutex(&m_fn_addr[core]); 
	
	//wake up processors with "SEND EVENT"
	asm volatile("sev");

	return 0;
}

void __park_and_wait (void)
{
	uint8_t core = curr_core_id();
	void (*trigger) (void);

	release_mutex(&m_fn_addr[core]);	
	release_mutex(&semaphores[core]);
	//keep waiting until you see something new
	while (1) {
		while (!READ_ONCE(m_fn_addr[core]))
			asm volatile ("wfe");

		trigger = (void (*)(void)) cpu_spin_table[core];
		trigger();
	}
}
