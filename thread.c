#include "hobos/smp.h"
#include "hobos/mutex.h"
#include "hobos/lib/thread.h"
#include "hobos/kstdio.h"

/* In order to queue a workload we need to go through the following
 * steps:
 *
 * 1) Get mutex to signal incoming event
 * 2) Get mutex to modify the relevant core spin table entry
 * 3) On success, modify the spintable entry (else we block and wait)
 * 4) Release the mutex for the spintable
 * 5) Once the function starts executing, unlock incoming event
 * 6) If no incoming event (mutex is unlocked), get spintable mutex set spintable entry to 0x0
 * 7) Unlock spintable mutex 
 * 8) set spintable entry to 0x0
 * */


/* for now, lets assume that all args and return
 * is dealt with using pointers/explicit memory locations
 * by the function itself - i.e, its the functions responsibility
 * to be self contained.
 */

MUTEX_VECTOR(core_incoming_event, MAX_CORES) = {0};
MUTEX(tid_cntr_mut) = 0;
uint8_t tid_cntr = 0;

/* For now, 1 core = 1 exec thread + 1 incoming thread.
 * 
 *  Incoming and Executing threads keep interchanging
 *  indexes. Eg, we are executing on core 0:
 *
 *  	- Check for active threads for core 1
 *  		- Both threads (at index 2,3) have .tid set as INACTIVE, so no active threads
 *  	- Use thread at the first available index (2 in this case)
 *  	- Convert incoming *task* to *thread* and pass it over to be queued on proc
 *  	- Wait for execution to complete and cleanup
 *
 * Once threads can be paused/suspended - for instance, in
 * case of IO, we can increase this number
 * */
struct thread_struct active_threads[2*MAX_CORES] = {0x0};

static void set_thread_active(struct thread_struct *t)
{
    //get tid from pool
    lock_mutex(&tid_cntr_mut);
    t->tid = tid_cntr++;
    unlock_mutex(&tid_cntr_mut);
}


static void set_thread_inactive(struct thread_struct *t)
{
    //send tid back to pool
    lock_mutex(&tid_cntr_mut);
    tid_cntr--;
    t->tid = INACTIVE;
    unlock_mutex(&tid_cntr_mut);
}

//TODO: return some error code
void queue_thread(struct task_struct *tsk)
{
    uint8_t pref_core = tsk->core_id;
    struct thread_struct *t;

    lock_mutex(&core_incoming_event[pref_core]);
    if (pref_core >= MAX_CORES)
	return;

    t = &active_threads[pref_core*2];

    if (t->tid != INACTIVE) 
	t = &active_threads[pref_core*2+1];
    
    
    t->core_id = tsk->core_id; 
    t->fn_addr =  tsk->fn_addr;
    set_thread_active(t);
    
    tsk->tid = t->tid;

    //PRINT_THREAD(t);

   // if (pref_core == ANY_CORE) {
   //     while(1) {
   //         //TODO:
   //         //for(i=0; i<MAX_CORES; i++)
   //        //try_and_get_mutex(); 	    
   //     }
   // } else {
    //	queue_on_proc(t->fn_addr, t->core_id);
    //}

    queue_on_proc(t->fn_addr, t->core_id);
    set_thread_inactive(t);

}

uint8_t incoming_thread_exists(uint8_t core_id)
{
    return get_mutex_state(&core_incoming_event[core_id]);
}
