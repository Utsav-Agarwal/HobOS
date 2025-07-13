#include <stddef.h>
#include "hobos/lib/stdlib.h"
#include "hobos/kstdio.h"
#include "hobos/mmio.h"
#include "hobos/smp.h"
#include "hobos/lib/thread.h"
#include "hobos/lib/task.h"

int test_cntr1 = 0;
int test_cntr2 = 0;
int test_cntr3 = 0;

void test_print1(void)
{
	kprintf("Hello again 1 %d*\n", test_cntr1++);
	return;
}

void test_print2(void)
{
	kprintf("Hello again 2 %d*\n", test_cntr2++);
	return;
}

void test_print3(void)
{
	kprintf("Hello again 3 %d*\n", test_cntr3++);
	return;
}

/* I'm alive */
void heartbeat(void)
{

    struct task_struct tsk1 = { 
        .core_id = 3, 
        .fn_addr = (uint64_t)(test_print1) 
    };
    
    struct task_struct tsk2 = { 
        .core_id = 1, 
        .fn_addr = (uint64_t)(test_print2) 
    };

    struct task_struct tsk3 = { 
        .core_id = 2, 
        .fn_addr = (uint64_t)(test_print3) 
    };

//	TASK(tsk1, 3, test_print1);
//	TASK(tsk2, 3, test_print2);

	queue_thread(&tsk1);
	queue_thread(&tsk2);
	
	queue_thread(&tsk1);
	queue_thread(&tsk2);

	queue_thread(&tsk1);
	queue_thread(&tsk2);
	
	queue_thread(&tsk1);
	queue_thread(&tsk2);
	
	queue_thread(&tsk3);
	queue_thread(&tsk2);
	queue_thread(&tsk3);
}

void main()
{
	get_rpi_version();
	mmio_init();
	init_console();

	init_smp();
	heartbeat();

	while (1) {
		//start shell here
		__asm__ volatile ("wfe");
	}

}
