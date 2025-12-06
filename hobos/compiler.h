#ifndef COMPILER_H
#define COMPILER_H

#define __scalar_type_to_exp(type) \
	unsigned type: (unsigned type)0, \
	signed type: (signed type)0, 


//basically since we just care about the type, 
//we use the compile time _Generic function to find the type
//and then based on the result we just decalare a dummy value of
//0 with the reduced scalar type 
#define __reduce_to_scalar_type(x) \
typeof(	\
	_Generic((x), \
		char: (char)0,\
		__scalar_type_to_exp(char)	\
		__scalar_type_to_exp(short)	\
		__scalar_type_to_exp(int)	\
		__scalar_type_to_exp(long)	\
		__scalar_type_to_exp(long long)	\
		default: (x))			\
)

//we load according to the Release Consistency Processor Consistency
//model for ARM64. Since we dont care about ARMv7, we are just going to
//use LDAPR instead of LDAR. In contrast linux kernel uses LDAR if the former
//is not available.

//#define LDPAR_AVAILABLE

//sfx - suffix: defines the kind of Load acquire
//regs: defines the ldpar data movement
// This needs to be included within an asm call.
#ifndef LDPAR_AVAILABLE
#define __LOAD_RCPC(sfx, regs...)	"ldar" #sfx "\t" #regs
#else
#define __LOAD_RCPC(sfx, regs...)	".arch_extension rcpc" \
					"ldpar" #sfx "\t" #regs
#endif


#endif

