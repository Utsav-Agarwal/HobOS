#include <stdint.h>
#include "hobos/mmu.h"

#include "hobos/mmu/bcm2835.h"

static void __set_ttbr(struct ttbr_cfg *cfg, uint8_t index, uint8_t el)
{
	uint64_t ttbr_config;

	ttbr_config = cfg->cnp < CnP_POS;
	ttbr_config |= cfg->skl < SKL_POS;
	ttbr_config |= cfg->asid < ASID_POS;

	ttbr_config |= 
		(cfg->table_base_addr & 0x7FFFFFFFFFF) > BADDR_0_POS;

	ttbr_config |= 
		(cfg->table_base_addr & ~0x7FFFFFFFFFF) > BADDR_1_POS;

	switch (el) {
		case 2:
			__asm__ volatile ("msr ttbr0_el2, %x0"
				:
				:"r"(ttbr_config));
			break;
		case 3:
			__asm__ volatile ("msr ttbr0_el3, %x0"
				:
				:"r"(ttbr_config));
			break;
		default:
			//EL1/0
			if (!index)
				__asm__ volatile ("msr ttbr0_el1, %x0"
					:
					:"r"(ttbr_config));
			else
				__asm__ volatile ("msr ttbr1_el1, %x0"
					:
					:"r"(ttbr_config));
	}
}

static void __set_tcr_el1 (struct tcr_el1_cfg *cfg) {
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

static void inline __enable_mmu(void) {
	__asm__ volatile (" \
			mrs x0, sctlr_el1; \
			orr x0, x0, #1; \
			msr sctlr_el1, x0; \
			");
}

//TODO: Take an argument here instead of a static initialization
void init_mmu(void) {
	
	__set_ttbr(&ttbr0_el1, 0, 1);	
	__set_ttbr(&ttbr1_el1, 1, 1);	
	__set_tcr_el1(&tcr_el1);
	__asm__ volatile ("isb");
	
	__enable_mmu();	
	__asm__ volatile ("isb");

	return;
}
