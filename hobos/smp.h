#ifndef SMP_H
#define SMP_H

#include <stdint.h>

/*
 * refer to https://github.com/raspberrypi/tools/blob/439b6198a9b340de5998dd14a26a0d9d38a6bcac/armstubs/armstub8.S#L175
 *
 * RPI FW initializes the remote processors using a spin table
 * where CPU IDs 1-3 are put to sleep using a spin table
 * */

//TODO: Add error codes
//TODO: Add mailbox support for reading processor messages

#define SPIN_TABLE_BASE		0xD8
#define MAX_REMOTE_CORE_ID	3

int run_process(uint64_t fn_addr, uint8_t cpu_id);

#endif
