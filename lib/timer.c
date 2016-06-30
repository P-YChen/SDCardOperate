/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "timer.h"
#include "speed.h"
#include "s3c24xx.h"
#include "eabi_compat.h"


int timer_load_val = 0;
static ulong timer_clk;

static inline ulong READ_TIMER(void);

/* macro to read the 16 bit timer */
static inline ulong READ_TIMER(void)
{
	ulong reg;
	reg = TCNTO4;

	return reg & 0xffff;
}

static ulong timestamp;
static ulong lastdec;

int timer_init(void)
{
	ulong tmr;

	/* use PWM Timer 4 because it has no output */
	/* prescaler for Timer 4 is 16 */
	TCFG0 = 0xf00;
	if (timer_load_val == 0) {
		/*
		 * for 10 ms clock period @ PCLK with 4 bit divider = 1/2
		 * (default) and prescaler = 16. Should be 10390
		 * @33.25MHz and 15625 @ 50 MHz
		 */
		timer_load_val = get_PCLK() / (2 * 16 * 100);
		timer_clk = get_PCLK() / (2 * 16);
	}
	/* load value for 10 ms timeout */
	lastdec = timer_load_val;
	TCNTB4 = timer_load_val;
	/* auto load, manual update of Timer 4 */
	tmr = TCON;
	tmr = (tmr & ~0x0700000) | 0x0600000;
	TCON = tmr;
	/* auto load, start Timer 4 */
	tmr = (tmr & ~0x0700000) | 0x0500000;
	TCON = tmr;
	timestamp = 0;

	// Debug
	printf ("\r\nFCLK=%ld\r\n", get_FCLK());
	printf ("HCLK=%ld\r\n", get_HCLK());
	printf ("PCLK=%ld\r\n", get_PCLK());
	printf ("timer_load_val=%ld\r\n", timer_load_val);
	printf ("timer_clk=%ld\r\n", timer_clk);


	return (0);
}

/*
 * timer without interrupts
 */

void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void set_timer(ulong t)
{
	timestamp = t;
}

void udelay(unsigned long usec)
{
	ulong tmo;
	ulong start = get_ticks();

	tmo = usec / 1000;
	tmo *= (timer_load_val * 100);
	tmo /= 1000;

	while ((ulong) (get_ticks() - start) < tmo)
		/*NOP*/;
}

void reset_timer_masked(void)
{
	/* reset time */
	lastdec = READ_TIMER();
	timestamp = 0;
}

ulong get_timer_masked(void)
{
	ulong tmr = get_ticks();

	return tmr / (timer_clk / CONFIG_SYS_HZ);
}

void udelay_masked(unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	if (usec >= 1000) {
		tmo = usec / 1000;
		tmo *= (timer_load_val * 100);
		tmo /= 1000;
	} else {
		tmo = usec * (timer_load_val * 100);
		tmo /= (1000 * 1000);
	}

	endtime = get_ticks() + tmo;

	do {
		ulong now = get_ticks();
		diff = endtime - now;
	} while (diff >= 0);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	ulong now = READ_TIMER();

	if (lastdec >= now) {
		/* normal mode */
		timestamp += lastdec - now;
	} else {
		/* we have an overflow ... */
		timestamp += lastdec + timer_load_val - now;
	}
	lastdec = now;

	return timestamp;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	ulong tbclk;

	tbclk = CONFIG_SYS_HZ;

	return tbclk;
}

/*
 * reset the cpu by setting up the watchdog timer and let him time out
 */
void reset_cpu(ulong ignored)
{
	/* Disable watchdog */
	WTCON = 0x0000;

	/* Initialize watchdog timer count register */
	WTCNT = 0x0001;

	/* Enable watchdog timer; assert reset at timer timeout */
	printf ("### Board ### Reset Now!\n");
	WTCON = 0x0021;

	while (1)
		/* loop forever and wait for reset to happen */;

	/*NOTREACHED*/
}
