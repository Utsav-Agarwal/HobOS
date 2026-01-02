#ifndef __MMU_H
#define __MMU_H

#include "lib/pt_lib.h"
#include "lib/stdlib.h"


struct ttbr_cfg {
	uint64_t table_base_addr;
	uint16_t asid;
	uint8_t	skl;
	uint8_t cnp;
};

//		BADDR = TTBR0_EL1 [87:80] [47:5]
//		==========================
//Base address of the stage 1 translation table

#define BADDR_1_POS	80
#define ASID_POS	48
#define BADDR_0_POS	5
#define SKL_POS		1
#define CnP_POS		0

//		SKL [2:1]
//		==========================
//skips the regular start level when starting to walk from 
//TTBR0_EL1
#define SKIP_LEVEL(n)	n


//		CnP (common not private) [0] 
//		==========================
// Indicates TTBR0_EL1 is a member of a common set

#define CNP_PRIVATE 		0x0
#define CNP_COMMON 		0x1

//
//NOTE: By convention, kernel is mapped on the higher phyiscal address range

//-----
//TCR_EL1 : 
//-----
struct tcr_el1_cfg {
	uint8_t	t0_sz;
	uint8_t t1_sz;
	uint8_t tg0;
	uint8_t tg1;
	uint8_t ips_sz;
};

#define EPD0_POS	7
#define EPD1_POS	23
#define EPD_WALK	0b0
#define EPD_FAULT	0b1

#define IRGN0_POS	8

#define ORGN0_POS	10

#define SH0_POS		12

#define T0SZ_POS	0
#define T1SZ_POS	16

#define IPS_POS		32    //Starting bit position

#define IPS_32		0x0   // 32-bit PA:  4GB
#define IPS_36		0x1   // 36-bit PA:  64GB
#define IPS_40		0x2   // 40-bit PA:  1TB
#define IPS_42		0x3   // 42-bit PA:  4TB
#define IPS_44		0x4   // 44-bit PA:  16TB
#define IPS_48		0x5   // 48-bit PA:  256TB
#define IPS_52		0x6   // 52-bit PA:  4PB
#define IPS_56		0x7   // 56-bit PA:  64PB

#define TG0_POS			14
#define TG1_POS			30


#define TG0_GRANULE_SZ_4KB		0b00LL
#define TG0_GRANULE_SZ_16KB		0b10LL
#define TG0_GRANULE_SZ_64KB		0b01LL

#define TG1_GRANULE_SZ_4KB		0b10LL
#define TG1_GRANULE_SZ_16KB		0b01LL
#define TG1_GRANULE_SZ_64KB		0b00LL
//----
//MAIR_EL0
//----

#define ALLOCATE	1
#define NO_ALLOCATE	0

#define NORMAL_DEVICE_MEM	0b0000
#define NORMAL_NON_CACHEABLE	0b0100


//MAIR

#define	NO_ALLOC	0b0
#define ALLOC		0b1

//Device memory
#define MAIR_DEV(g, r, e)	((g + r + e) << 2) 

//Normal memory

#define MAIR_MEM_RW(msb_2, r, w)	((msb_2 << 2) | (r < 1) | w)  

//Outer cache
#define MAIR_MEM_O_NC	0b0100

#define MAIR_MEM_O_WT_T(r, w)	MAIR_MEM_RW(0b00, r, w)
#define MAIR_MEM_O_WB_T(r, w)	MAIR_MEM_RW(0b01, r, w)
#define MAIR_MEM_O_WT_T(r, w)	MAIR_MEM_RW(0b10, r, w)
#define MAIR_MEM_O_WB_NT(r, w)	MAIR_MEM_RW(0b11, r, w)

//Inner cache
#define MAIR_MEM_I_NC	0b0100

#define MAIR_MEM_I_WT_T(r, w)	MAIR_MEM_RW(0b00, r, w)
#define MAIR_MEM_I_WB_T(r, w)	MAIR_MEM_RW(0b01, r, w)
#define MAIR_MEM_I_WT_T(r, w)	MAIR_MEM_RW(0b10, r, w)
#define MAIR_MEM_I_WB_NT(r, w)	MAIR_MEM_RW(0b11, r, w)

//kernel offset after switch to high memory addressing
#define KERNEL_START	0xFFFFFF8000000000

struct mair_attr {
	uint8_t outer_cache: 4;
	uint8_t inner_cache: 4;
};

struct mair {
	struct mair_attr attr0;
	struct mair_attr attr1;
	struct mair_attr attr2;
	struct mair_attr attr3;
};


extern volatile unsigned char __data_start;
extern volatile unsigned char __end;
extern volatile unsigned char __core0_stack;

void init_mmu(void);
uint64_t switch_vmem(void);
void map_pa_to_va_pg(uint64_t pa, uint64_t va, struct page_table_desc *pt_top,
		uint64_t flags);


#endif
