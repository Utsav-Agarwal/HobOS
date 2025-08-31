#ifndef __MMU_H
#define __MMU_H

//references:
/* https://lowenware.com/blog/aarch64-mmu-programming/ */
/* https://github.com/bztsrc/raspi3-tutorial/tree/master/10_virtualmemory */
/* https://developer.arm.com/documentation/100940/0101/ */

/* MMU translates 64bit addresses, but not all addresses are 64bits */

//-----
//TTBR : Translation Table Base Registers - which then point to translation tables
//-----
//EL1/EL0 - Range: 
//			TTBR0  0x0000FFFF_FFFFFFFF - 0x00000000_00000000
//			TTBR1  0xFFFF0000_00000000 - 0x0000FFFF_FFFFFFFF
//	           
//EL2/EL3 - Range: 
//			TTBR0	0x0000FFFF_FFFFFFFF - 0x00000000_00000000
//
//NOTE: These are ofc the virtual address spaces in each execution level.
//i.e, EL2/EL3 (privileged) levels have limited VM 
//

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

//
//		T1SZ[21:16], T0SZ[5:0]
//		==========================
//defines the no. of MSB that are checked for an address
//
// i.e, 2^(64-T0SZ) = area addressable by TTBR0, same for TTRB1
// Min/Max depends on granule size and starting table level

#define T0_SZ_POS	0
#define T1_SZ_POS	16

// 		IPS[34:32] (Intermediate Physical Address Size)
//		==========================
// defines the max size of the address after which a fault is generated
//
//	0b000	32 bits, 4GB.
//	0b001	36 bits, 64GB.
//	0b010	40 bits, 1TB.
//	0b011	42 bits, 4TB.
//	0b100	44 bits, 16TB.
//	0b101	48 bits, 256TB.
//	0b110	52 bits, 4PB.
//	0b111	56 bits, 64PB.
//

#define IPS_POS		32    //Starting bit position

#define IPS_32		0x0   // 32-bit PA:  4GB
#define IPS_36		0x1   // 36-bit PA:  64GB
#define IPS_40		0x2   // 40-bit PA:  1TB
#define IPS_42		0x3   // 42-bit PA:  4TB
#define IPS_44		0x4   // 44-bit PA:  16TB
#define IPS_48		0x5   // 48-bit PA:  256TB
#define IPS_52		0x6   // 52-bit PA:  4PB
#define IPS_56		0x7   // 56-bit PA:  64PB

//
//		TG1/0 (Translation Granule)
//		==========================
//defines the granule size for kernel and user space respectively
//	00	4KB 
//	01	16KB 
//	11	64KB
//NOTE: Granule = Smallest block of mappable memory to translation tables

// First level of lookup is determined using the TGI and TnSZ fields.
// This can ofcourse be seperate for TTBR1_EL1/TTBR0_EL1
// NOTE: Translation lookups can sometimes require 3-4 levels
//
//
//

//NOTE: Writes to System Regs for MMU are context changing events
//results are not guaranteed until a synchronization event (barrier)

#define TG0_POS			14
#define TG1_POS			30


#define TG0_GRANULE_SZ_4KB		0x00
#define TG0_GRANULE_SZ_16KB		0x01
#define TG0_GRANULE_SZ_64KB		0x11

#define TG1_GRANULE_SZ_16KB		0x00
#define TG1_GRANULE_SZ_4KB		0x01
#define TG1_GRANULE_SZ_64KB		0x11

void init_mmu(void);

#endif
