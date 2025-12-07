#ifndef BARRIER_H
#define BARRIER_H

#include "../compiler.h"

//This is a compiler barrier which essentially marks all memory
//to be used and volatile
#define barrier() asm volatile ("" : : :"memory");

#define dmb(opt)	asm volatile ("dmb " #opt :::"memory")

#define mb()	dmb(ish)
#define wmb()	dmb(ishst)
#define rmb()	dmb(ishld)

#define dma_mb()	dmb(osh)
#define dma_wmb()	dmb(oshst)
#define dma_rmb()	dmb(oshld)

#define smp_mb()	mb()

#define isb() asm volatile ("isb");

//CSAN - Concurrency Sanitizer
#define __CSAN_mb	__ATOMIC_SEQ_CST
#define __CSAN_wmb	__ATOMIC_ACQ_REL
#define __CSAN_rmb	__ATOMIC_ACQUIRE
#define __CSAN_release	__ATOMIC_RELEASE

//this does not produce any code, but indicates to the
//compiler that this is a fence/barrier and sync point for signal
//handlers and the current thread
#define csan_barrier(x)	do { \
				barrier(); \
				__atomic_signal_fence(__CSAN_##x); \
				barrier(); \
			} while (0) 


#define csan_mb()	csan_barrier(mb)
#define csan_wmb()	csan_barrier(wmb)
#define csan_rmb()	csan_barrier(rmb)
#define csan_release()	csan_barrier(release)

//NOTE: using a union here allows us to not have alignment issues
#define READ_ONCE(x) \
({ \
	typeof(&(x)) __x = &(x); 					\
	int atomic = 1; 						\
	union {__reduce_to_scalar_type(*__x) __val; char c[1]; } __u; 	\
	switch (sizeof(x)) { 						\
		case 1:							\
			asm volatile(__LOAD_RCPC(b, %w0, %1)		\
					:"=r"(*(uint8_t*) __u.c)	\
					:"Q"(*__x) : "memory");		\
			break;						\
		case 2:							\
			asm volatile(__LOAD_RCPC(h, %w0, %1)		\
					:"=r"(*(uint16_t*) __u.c)	\
					:"Q"(*__x): "memory");		\
			break;						\
		case 4:							\
			asm volatile(__LOAD_RCPC(, %w0, %1)		\
					:"=r"(*(uint32_t*) __u.c)	\
					:"Q"(*__x) : "memory");		\
			break;						\
		case 8:							\
			asm volatile(__LOAD_RCPC(, %0, %1)		\
					:"=r"(*(uint64_t*) __u.c)	\
					:"Q"(*__x) : "memory");		\
			break;						\
		default:						\
			atomic = 0;					\
	}								\
	atomic ? (typeof(*__x)) __u.__val : (*(volatile typeof(__x)) __x);  \
})

#define WRITE_ONCE(x, val) \
	do {						\
		*(volatile typeof(x) *) &(x) = val;	\
	} while (0)

//Generic smp barriers
#define smp_store_mb(x, val)	do {	\
		csan_mb();	\
		WRITE_ONCE(x, val);	\
	} while (0) 

//NOTE: "Q" means that the address needs to be simple and addressable,
//i.e, no temporary buffer, address on stack, etc by the compiler

//NOTE: rZ means use a register and the value can be 0, so the compiler 
//might want to optimize the access to 0 or xzr.
#define smp_store_release(p, v) 					\
do {									\
	typeof(p) __p = p;						\
	union {__reduce_to_scalar_type(*p) __val; char c[1]} __u; 	\
	__u.__val = (__reduce_to_scalar_type(*p)) v;			\
	switch (sizeof(*p)) {						\
		case 1: 						\
			asm volatile("stlrb %w1, %0" 			\
					: "=Q" (*__p) 			\
					: "rZ" (*(uint8_t *) __u.c) 	\
					: "memory"); 			\
			break; 						\
		case 2: 						\
			asm volatile("stlrh %w1, %0" 			\
					: "=Q" (*__p)			\
					: "rZ" (*(uint16_t *) __u.c) 	\
					: "memory"); 			\
			break; 						\
		case 4: 						\
			asm volatile("stlr %w1, %0" 			\
					: "=Q" (*__p)			\
					: "rZ" (*(uint32_t *) __u.c) 	\
					: "memory"); 			\
			break; 						\
		case 8: 						\
			asm volatile("stlr %x1, %0" 			\
					: "=Q" (*__p) 			\
					: "rZ" (*(uint64_t *) __u.c) 	\
					: "memory"); 			\
			break; 						\
	}								\
} while (0)

//looking at the kernel code, it is essentially the exact same
#define smp_load_acquire(p)	READ_ONCE(p)

#endif
