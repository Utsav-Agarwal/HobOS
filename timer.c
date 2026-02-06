// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/timer.h>
#include <hobos/lib/stdlib.h>
#include <hobos/kstdio.h>

struct timer global_timer;

static unsigned int read_timer32(bool msb, struct timer *t)
{
	if (msb)
		return mmio_read(t->msb);

	return mmio_read(t->lsb);
}

static unsigned long read_timer64(struct timer *t)
{
	unsigned long val = 0;

	val = mmio_read(t->lsb);
	val |= ((u64)mmio_read(t->msb) << 32);

	return val;
}

//TODO: extract these register addresses from a priv structure
static void set_timer(struct timer *t, unsigned int val)
{
	unsigned int target_val = read_timer32(0, t);

	target_val += val;
	mmio_write(BCM2835(C1), target_val);
}

static void reset_timer(struct timer *t)
{
	mmio_write(BCM2835(BASE_OFF), BITP(1));
}

/*
 * We dont want just 1 global timer, since each core can
 * have individual timers. As a result, we let the user/
 * author for a particular code section decide on how and
 * what to initialize
 */
void init_timer(struct timer *t)
{
	switch (rpi_version) {
	case 3:
		t->base = BCM2835(BASE_OFF);
		t->msb = BCM2835(MSB);
		t->lsb = BCM2835(LSB);
		break;

	default:
		kprintf("RPI rev not supported\n");
			return;
	}

	ioremap(BCM2835(BASE_OFF) + (unsigned long)mmio_base);

	/* populate the remaining functions with generic implementations IF
	 * no user/platform specific functions.
	 */
	if (!t->read_timer32) {
		t->read_timer32 = read_timer32;
		t->read_timer64 = read_timer64;
	}

	t->set_timer = set_timer;
	t->reset_timer = reset_timer;
}

unsigned int read_timer(bool msb, struct timer *t)
{
	return t->read_timer32(msb, t);
}
