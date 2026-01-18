#include "hobos/timer.h"
#include "hobos/lib/stdlib.h"
#include "hobos/kstdio.h"

extern uint8_t rpi_version;
struct timer global_timer;

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

//TODO: extract these register addresses from a priv structure
static void set_timer (struct timer *t, uint32_t val) 
{
	uint32_t target_val = read_timer32(0, t);

	target_val += val;
	mmio_write(BCM2835(C1), target_val);
}

static void reset_timer (struct timer *t)
{
	mmio_write(BCM2835(BASE_OFF), BITP(1));
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
	
	ioremap(BCM2835(BASE_OFF) + (uint64_t) mmio_base);

	/* populate the remaining functions with generic implementations IF 
	 * no user/platform specific functions. */
	if (!t->read_timer32) {
		t->read_timer32 = read_timer32; 
		t->read_timer64 = read_timer64;
	}

	t->set_timer = set_timer;
	t->reset_timer = reset_timer;
	//t->enable_interrupts = enable_interrupts;
	//t->disable_interrupts = disable_interrupts;

	return;	
}

uint32_t read_timer(bool msb, struct timer *t)
{
	return t->read_timer32(msb, t);
}
