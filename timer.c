#include "hobos/timer.h"
#include "hobos/kstdio.h"

extern uint8_t rpi_version;

static uint32_t read_timer32(bool msb, struct timer *t)
{
	if (msb) 
		return mmio_read(t->msb);

	return mmio_read(t->lsb);
}

static uint64_t read_timer64(struct timer *t)
{
	uint64_t val = 0;

	val = mmio_read(t->lsb);
	val |= (mmio_read(t->msb) << 32);

	return val;
}

static void write_timer32(bool msb, uint32_t val, struct timer *t)
{
	if (msb) 
		mmio_write(t->msb, val);

	mmio_write(t->lsb, val);
}

static void write_timer64(uint64_t val, struct timer *t)
{
	mmio_write(t->lsb, (val & 0xFFFF));
	mmio_write(t->msb, ((val >> 32) & 0xFFFF));
}

/*
 * We dont want just 1 global timer, since each core can 
 * have individual timers. As a result, we let the user/
 * author for a particular code section decide on how and
 * what to initialize
 * */
void init_timer(struct timer *t)
{
	//TODO: check further for variations in the cpu model
	//eg: rpi3, rpi3b, rpi3b+
	switch(rpi_version){
		case 3:
			t->base = BCM2835(BASE_OFF);
			t->msb = BCM2835(MSB);
			t->lsb = BCM2835(LSB);

			break;
		default:
			kprintf("RPI rev not supported\n");
			return;
	}

	/* populate the remaining functions with generic implementations IF 
	 * no user/platform specific functions. */
	if (!t->read_timer32)
		t->read_timer32 = read_timer32; 
		t->read_timer64 = read_timer64;

	if (!t->set_timer32)
		t->set_timer32 = write_timer32; 
		t->set_timer64 = write_timer64; 


	return;	
}

uint32_t read_timer(bool msb, struct timer *t)
{
	return t->read_timer32(msb, t);
}
