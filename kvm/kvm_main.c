#include <fcntl.h>
#include <linux/kvm.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

// Define guest params
#define PAGE_SIZE	0x1000
#define GUEST_MEM_SZ	512*PAGE_SIZE
#define GUEST_MEM_START 0x400000

#define NR_VCPU		1

// Lets emulate mini UART for now
// AUX_MU_IO
#define UART_ADDR	0x3F215040

// Error
// TODO: Introduce more error types
#define ERR_GENERIC	-1	

// Debug
#define pr_err(fmt, ...)	do { printf("[ERROR]\t"fmt,##__VA_ARGS__); fflush(stdout);}while(0); 
#define pr_info(fmt, ...)	do { printf("[INFO]\t"fmt,##__VA_ARGS__); fflush(stdout);} while (0);
#define pr_success(fmt, ...)	do {printf("[SUCCESS]\t"fmt,##__VA_ARGS__); fflush(stdout);} while (0);


// Misc
#define KVM	"/dev/kvm"
#define GUEST_NAME	"/kernel8.bin"


#define ARM64_SYSREG(op0, op1, crn, crm, op2) \
	    (((uint64_t)(op0) << 20) | \
	     ((uint64_t)(op1) << 17) | \
	     ((uint64_t)(crn) << 14) | \
	     ((uint64_t)(crm) << 11) | \
	     ((uint64_t)(op2) << 8))

void print_esr(int vcpu) {
	struct kvm_one_reg reg;
	uint64_t esr;

	reg.id = KVM_REG_ARM64 | KVM_REG_SIZE_U64 |
		 KVM_REG_ARM64_SYSREG |
		 ARM64_SYSREG(3, 0, 5, 2, 0);  // ESR_EL1
	if (ioctl(vcpu, KVM_GET_ONE_REG, &reg) == 0) {
	    printf("ESR_EL1 = 0x%lx\n", esr);
	}
}

int main(int argc, char **argv)
{
	int kvm, vcpu, vm;
	void *mem;

	// Close when executing exec()
	kvm = open(KVM, O_RDWR | O_CLOEXEC);
	if (kvm < 0) {
		pr_err("Could not open %s(%d)\n", KVM, kvm);
		return ERR_GENERIC;
	}

	// Create VM/Guest
	pr_info("Creating guest...\n");
	vm = ioctl(kvm, KVM_CREATE_VM, 0);
	if (vm < 0) {
		pr_err("Could not create guest(%d)\n", vm);
		return ERR_GENERIC;
	}

	// Allocate guest memory
	pr_info("Allocating memory for guest...\n");
	mem = mmap(NULL, GUEST_MEM_SZ,
		   PROT_READ | PROT_WRITE,
		   MAP_ANONYMOUS | MAP_PRIVATE,
		   -1, 0);

	if (mem == MAP_FAILED) {
		pr_err("Could not allocate memory\n");
		return ERR_GENERIC;
	}

	// Map memory to guest
	pr_info("Mapping memory to guest...\n");
	struct kvm_userspace_memory_region region = {
		.slot = 0,
		.guest_phys_addr = GUEST_MEM_START,
		.memory_size = GUEST_MEM_SZ,
		.userspace_addr = (uint64_t)mem,
	};

	if (ioctl(vm, KVM_SET_USER_MEMORY_REGION, &region) < 0) {
		pr_err("Could not set guest memory\n");
		return ERR_GENERIC;
	}

	// Load guest binary (HobOS in this case)
	pr_info("Loading %s to memory...\n", GUEST_NAME);
	FILE *f = fopen(GUEST_NAME, "rb");
	if (!f) {
		pr_err("Could not find guest bin:%s\n", GUEST_NAME); 
		return ERR_GENERIC;
	}

	pr_info("Read %dB\n", fread(mem, 1, GUEST_MEM_SZ, f));
	fclose(f);

	// Create vCPU	- For now, only create CPU0
	// TODO: Init 4 cores with dedicated register maps for startup
	pr_info("Creating vCPUs...\n");
	vcpu = ioctl(vm, KVM_CREATE_VCPU, 0);
	if (vcpu < 0) {
		pr_err("Could not create vCPUs\n");
		return ERR_GENERIC;
	}

	// Map kvm_run - this is what userspace uses to interface with kvm
	// 1. Get the size we need for this structure - varies with how
	// the kernel is compiled. So keeping it dymamic is probably a idea
	// for keeping things portable - the kernel it is compiled on may not
	// be the same as the one running it - think of nested virt. with qemu
	int vcpu_mmap_size = ioctl(kvm, KVM_GET_VCPU_MMAP_SIZE, 0);
	// 2. Allocate the structure with mmap
	// NOTE: This structure is for all intents and purposes a snapshot 
	// of the CPU state.
	struct kvm_run *run = mmap(NULL, vcpu_mmap_size,
				   PROT_READ | PROT_WRITE,
				   MAP_SHARED, vcpu, 0);

	if (run == MAP_FAILED) {
		pr_err("Could not map shared run structure\n");
		return ERR_GENERIC;
	}

	// Init vCPU
	struct kvm_vcpu_init init;
	if (ioctl(vm, KVM_ARM_PREFERRED_TARGET, &init) < 0) {
		pr_err("Target not available\n");
		return ERR_GENERIC;
	}

	if (ioctl(vcpu, KVM_ARM_VCPU_INIT, &init) < 0) {
		pr_err("vCPU init failed\n");
		return ERR_GENERIC;
	}

	// Set PC
	pr_info("Setting PC...\n");
	struct kvm_one_reg reg;
	uint64_t pc = GUEST_MEM_START;

	reg.id = KVM_REG_ARM64 | KVM_REG_SIZE_U64 |
		 KVM_REG_ARM_CORE | KVM_REG_ARM_CORE_REG(regs.pc);

	reg.addr = (uint64_t)&pc;
	if (ioctl(vcpu, KVM_SET_ONE_REG, &reg) < 0) {
		pr_err("Could not set PC, aborting...\n");
		return ERR_GENERIC;
	}


	// Set pstate
	uint64_t pstate = 0x3c5;  // EL1h

	reg.id = KVM_REG_ARM64 | KVM_REG_SIZE_U64 |
		 KVM_REG_ARM_CORE |
		 KVM_REG_ARM_CORE_REG(regs.pstate);
	reg.addr = (uint64_t)&pstate;

	ioctl(vcpu, KVM_SET_ONE_REG, &reg);

	// Main loop
	pr_info("Booting into guest...\n");
	while (1) {
		if (ioctl(vcpu, KVM_RUN, 0) < 0) {
			pr_err("Could not run guest\n");
			return ERR_GENERIC;
		}

		switch (run->exit_reason) {
		case KVM_EXIT_MMIO:
			pr_info("mmio(%lx)\n", run->mmio.phys_addr);
			if (run->mmio.is_write) {
				if (run->mmio.phys_addr == UART_ADDR) {
					char c = run->mmio.data[0];
					putchar(c);
					fflush(stdout);
				}
				else {
					pr_err("Unrecognized mmio(%lx)\n",
						run->mmio.phys_addr);
					return ERR_GENERIC;
				}
			}

			break;

		case KVM_EXIT_HLT:
			pr_info("Guest halted\n");
			break;
		
		case KVM_EXIT_EXCEPTION:
			pr_err("exception exit(%d), abort\n", run->exit_reason);
			return ERR_GENERIC;

		default:
			pr_err("Unknown exit(%d), abort\n", run->exit_reason);
			return ERR_GENERIC;
		}

		print_esr(vcpu);
		pr_info("exit reason: %d\n", run->exit_reason);
		pr_info("Resuming guest\n");
	}

	return 0;
}
