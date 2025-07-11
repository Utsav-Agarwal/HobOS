.section ".text.mutex"

.global lock_mutex
.global unlock_mutex
.global get_mutex_state

//https://developer.arm.com/documentation/dht0008/a/arm-synchronization-primitives/practical-uses/implementing-a-mutex

#define LOCKED		1
#define UNLOCKED	0

/* arg1 = mutex addr */
lock_mutex:
1:  ldaxr   w1, [x0]           // load lock
    cbnz    w1, 1b             // if locked, retry
    mov     w1, #LOCKED
    stxr    w2, w1, [x0]       // try to store 1 (lock)
    cbnz    w2, 1b             // if store failed, retry
    ret


get_mutex_state:
    dmb 	st		
1:  ldaxr   w0, [x0]           // load lock
    ret

unlock_mutex:
    ldr		x1, =UNLOCKED
    dmb		st			//data barrier, needed before releasing exclusive resource
    str		x1, [x0]	//unlock
    sev				//signal that lock is ready to the ones waiting
    ret

