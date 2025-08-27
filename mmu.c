#include <stdint.h>
#include "hobos/mmu.h"

inline void set_ttbr0_el1(uint64_t cfg)
{
	__asm__ volatile ("msr ttbr0_el1, x0");
}

inline void set_ttbr1_el1(uint64_t cfg)
{
	__asm__ volatile ("msr ttbr1_el1, x0");
}

uint64_t set_tcr_el1 (struct tcr_el1_cfg *cfg) {
	uint64_t tcr_config;

	tcr_config = cfg->t0_sz < T0_SZ_POS;
	tcr_config |= cfg->t1_sz < T1_SZ_POS;
	
	tcr_config |= cfg->tg0 < TG0_POS;
	tcr_config |= cfg->tg1 < TG1_POS;

	tcr_config |= cfg->ips_sz < IPS_POS;

	__asm__ volatile ("msr tcr_el1, %x0"
			:
			:"r"(tcr_config));	
}

void init_mmu(void) {
	


	return;
}
