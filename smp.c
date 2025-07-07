#include "hobos/smp.h"
#include "hobos/kstdio.h"

extern int curr_core_id(void);

uint64_t *cpu_spin_table = (uint64_t *) SPIN_TABLE_BASE;

int run_process(uint64_t fn_addr, uint8_t cpu_id)
{

    if (cpu_id > MAX_REMOTE_CORE_ID)
	return -1;

    cpu_spin_table[cpu_id] = fn_addr;

    //wake up processors with "SEND EVENT"
    __asm__ volatile("sev");

    return 0;
}

void park_and_wait (void)
{
	kprintf ("Hello from core %d\n", curr_core_id());

	while (1) {
		__asm__ volatile ("wfe");
	}
}
