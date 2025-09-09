#include <stdint.h>
#include "hobos/mmu.h"

#include "hobos/mmu/bcm2835.h"
#include "hobos/kstdio.h"

extern uint64_t mmio_base;

static void __set_ttbr(struct ttbr_cfg *cfg, uint8_t index, uint8_t el)
{
	uint64_t ttbr_config;

	ttbr_config = 
		((uint32_t)cfg->table_base_addr & 0x7FFFFFFFFFF);

	ttbr_config |= cfg->cnp < CnP_POS;
	ttbr_config |= cfg->skl < SKL_POS;
	ttbr_config |= cfg->asid < ASID_POS;



	switch (el) {
		case 2:
			__asm__ volatile ("msr ttbr0_el2, %0"
				:
				:"r"(ttbr_config));
			break;
		case 3:
			__asm__ volatile ("msr ttbr0_el3, %0"
				:
				:"r"(ttbr_config));
			break;
		default:
			//EL1/0
			if (!index)
				__asm__ volatile ("msr ttbr0_el1, %0"
					:
					:"r"(ttbr_config));
			else
				__asm__ volatile ("msr ttbr1_el1, %0"
					:
					:"r"(ttbr_config));
	}
}

static void __set_tcr_el1 (struct tcr_el1_cfg *cfg) {
//	uint64_t tcr_config;
//
//	tcr_config = cfg->t0_sz < T0_SZ_POS;
//	tcr_config |= cfg->t1_sz < T1_SZ_POS;
//	
//	tcr_config |= cfg->tg0 < TG0_POS;
//	tcr_config |= cfg->tg1 < TG1_POS;
//
//	tcr_config |= cfg->ips_sz < IPS_POS;
//
//	__asm__ volatile ("msr tcr_el1, %x0"
//			:
//			:"r"(tcr_config));

    uint64_t r,b;
    
    asm volatile ("mrs %0, id_aa64mmfr0_el1" : "=r" (r));
    b=r&0xF;
    if(r&(0xF<<28)/*4k*/ || b<1/*36 bits*/) {
        kprintf("ERROR: 4k granule or 36 bit address space not supported\n");
        return;
    }

    kprintf("b=%x\n", b);

    r=  (0b00LL << 37) | // TBI=0, no tagging
        (b << 32) |      // IPS=autodetected
        (0b10LL << 30) | // TG1=4k
        (0b11LL << 28) | // SH1=3 inner
        (0b01LL << 26) | // ORGN1=1 write back
        (0b01LL << 24) | // IRGN1=1 write back
        (0b0LL  << 23) | // EPD1 enable higher half
        (25LL   << 16) | // T1SZ=25, 3 levels (512G)
        (0b00LL << 14) | // TG0=4k
        (0b11LL << 12) | // SH0=3 inner
        (0b01LL << 10) | // ORGN0=1 write back
        (0b01LL << 8) |  // IRGN0=1 write back
        (0b0LL  << 7) |  // EPD0 enable lower half
        (25LL   << 0);   // T0SZ=25, 3 levels (512G)
			 
    asm volatile ("msr tcr_el1, %0; isb" : : "r" (r));


}

static void inline __enable_mmu(void) 
{
	uint64_t r;

	// finally, toggle some bits in system control register to enable page translation
	asm volatile ("dsb ish; isb; mrs %0, sctlr_el1" : "=r" (r));
	r|=0xC00800;     // set mandatory reserved bits
	r&=~((1<<25) |   // clear EE, little endian translation tables
	     (1<<24) |   // clear E0E
	     (1<<19) |   // clear WXN
	     (1<<12) |   // clear I, no instruction cache
	     (1<<4) |    // clear SA0
	     (1<<3) |    // clear SA
	     (1<<2) |    // clear C, no cache at all
	     (1<<1));    // clear A, no aligment check
	
	r|=  (1<<0);     // set M, enable MMU
	asm volatile ("msr sctlr_el1, %0; isb" : : "r" (r));
}


static inline uint64_t __get_next_table(uint64_t curr_table, const uint32_t pg_sz) 
{
	uint64_t next = curr_table + pg_sz;
	
	return next;
}

static inline uint64_t *__get_table(uint64_t curr_table) 
{
	return (uint64_t *) curr_table;
}

static inline uint64_t __get_table_level(uint64_t lvl_0_baddr, uint8_t lvl, const uint32_t pg_sz) 
{
	int i; 
	uint64_t addr;

	for(i = 0; i<lvl; i++)
		addr = __get_next_table(lvl_0_baddr, pg_sz);

	return addr; 
}


void __set_mair_el1(struct mair *mair_el1)
{

	kprintf("mair: %x\n", *mair_el1);
	__asm__ volatile ("msr mair_el1, %0"
			:
			: "r" (*mair_el1));
}

extern volatile unsigned char __data_start;
extern volatile unsigned char __end;

static void set_userland_tables(void)
{
		uint64_t *pt_desc = (uint64_t *) (&__end);
		uint64_t i, data_pg = ((uint64_t) (&__data_start)) / PAGE_SIZE;

		//L0
		pt_desc[0] = (uint64_t)((uint64_t *)(&__end) + 2*PAGE_SIZE)
			| PT_AF_ACCESSED	
			| PT_SH_I		
			| PT_USER		
			| PT_INDEX_MEM		
			| PT_PAGE;

		//skip L1 for now

		//L2
		pt_desc[2*512] = (uint64_t)((uint64_t *)&__end + 3*PAGE_SIZE)
			| PT_AF_ACCESSED	
			| PT_SH_I		
			| PT_USER		
			| PT_INDEX_MEM		
			| PT_PAGE;

		//we are at the penultimate level, so we can skip 0th entry

		//at L2 for 4KB, we care only about bits 29:21 for address
		//translation to L3. Each entry points to a 2MB block.
		
		//we also need to be careful here, since we are mapping physical
		//memory blocks, we might now encounter mmio, which is not
		//normal memory (device memory) and requires different caching
		//strategy
		for(i=1; i<512; i++) {
			pt_desc[2*512 + i] = ((uint64_t ) i << 21)
			| PT_AF_ACCESSED	
			| PT_USER		
			| PT_BLOCK;

			if ((i << 21) > mmio_base)
				pt_desc[2*512] |= PT_INDEX_DEV | PT_SH_O;
			else
				pt_desc[2*512] |= PT_INDEX_MEM | PT_SH_I;
		}

		//L3

		//we can now index memory by pages. We also need to be
		//careful about the code and data sections since code
		//should not be modified. 
		for (i=0; i<512; i++) {
			pt_desc[3*512 + i] = ((uint64_t ) i*PAGE_SIZE)
			| PT_AF_ACCESSED	
			| PT_USER		
			| PT_PAGE;

			if ((i<0x80)||(i >= data_pg)) 
				pt_desc[3*512 + i] |= PT_AP_RW | PT_UXN_NX;
			else
				pt_desc[3*512 + i] |= PT_AP_RO;
		}

}


static void set_kernel_tables(void)
{
	uint64_t *pt_desc = (uint64_t *) (&__end);

	//L1
	pt_desc[512+511] = (uint64_t)((uint64_t *)&__end + 4*PAGE_SIZE)
			| PT_AF_ACCESSED	
			| PT_SH_I		
			| PT_KERNEL		
			| PT_INDEX_MEM		
			| PT_PAGE;

	//L2
	pt_desc[4*512+511] = (uint64_t)((uint64_t *)&__end + 5*PAGE_SIZE)
			| PT_AF_ACCESSED	
			| PT_SH_I		
			| PT_KERNEL		
			| PT_INDEX_MEM		
			| PT_PAGE;

	//L3 - only map uart
	pt_desc[5*512] = (uint64_t) (mmio_base + 0x201000)
			| PT_AF_ACCESSED	
			| PT_SH_O
			| PT_UXN_NX
			| PT_KERNEL		
			| PT_INDEX_DEV		
			| PT_PAGE;
}

//TODO: Split into user/kernel functions, maybe also make it board
//defined
static void __set_translation_tables() {

//	+---+--------+-----+-----+---+------------------------+---+----+----+----+----+------+----+----+
//	| R |   SW   | UXN | PXN | R | Output address [47:12] | R | AF | SH | AP | NS | INDX | TB | VB |
//	+---+--------+-----+-----+---+------------------------+---+----+----+----+----+------+----+----+
//	 63  58    55 54    53    52  47                    12 11  10   9  8 7  6 5    4    2 1    0
//	
//	R    - reserve
//	SW   - reserved for software use
//	UXN  - unprivileged execute never
//	PXN  - privileged execute never
//	AF   - access flag
//	SH   - shareable attribute
//	AP   - access permission
//	NS   - security bit
//	INDX - index into MAIR register
//	TB   - table descriptor bit
//	VB   - validity descriptor bit


	set_userland_tables();
	set_kernel_tables();
}

//TODO: Take an argument here instead of a static initialization
void init_mmu(void) {

	__set_translation_tables(TABLE_BADDR);
	__set_mair_el1(&mair_el1);

	//TODO: Need to skip L0
	__set_tcr_el1(&tcr_el1);
	__set_ttbr(&ttbr0_el1, 0, 1);	
	__set_ttbr(&ttbr1_el1, 1, 1);	
	__asm__ volatile ("isb");
	
	__enable_mmu();	
	__asm__ volatile ("isb");

	return;
}
