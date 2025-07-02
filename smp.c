#include "hobos/smp.h"

int run_process(uint64_t fn_addr, uint8_t cpu_id)
{

    if (cpu_id > MAX_REMOTE_CORE_ID)
	return -1;

    *(uint64_t *) (SPIN_TABLE_BASE + (cpu_id * 8)) = fn_addr;

    //wake up processors with "SEND EVENT"
    __asm__ volatile("sev");
    return 0;
}
