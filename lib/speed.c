/*
 * (C) Copyright 2001-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
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

/* This code should work for both the S3C2400 and the S3C2410
 * as they seem to have the same PLL and clock machinery inside.
 * The different address mapping is handled by the s3c24xx.h files below.
 */

#include <stdio.h>
#include "s3c24xx.h"
#include "speed.h"
#include "eabi_compat.h"

#define MPLL 0
#define UPLL 1

/* ------------------------------------------------------------------------- */
/* NOTE: This describes the proper use of this file.
 *
 * CONFIG_SYS_CLK_FREQ should be defined as the input frequency of the PLL.
 *
 * get_FCLK(), get_HCLK(), get_PCLK() and get_UCLK() return the clock of
 * the specified bus in HZ.
 */
/* ------------------------------------------------------------------------- */
static ulong get_PLLCLK(int pllreg);

void hang (void)
{
	printf ("### ERROR ### Please reset the board\n");
	for (;;);
}

static ulong get_PLLCLK(int pllreg)
{
	ulong r, m, p, s;

	if (pllreg == MPLL)
		r = MPLLCON;
	else if (pllreg == UPLL)
		r = UPLLCON;
	else
		hang();

	m = ((r & 0xFF000) >> 12) + 8;
	p = ((r & 0x003F0) >> 4) + 2;
	s = r & 0x3;

	if (pllreg == MPLL)
		return ((CONFIG_SYS_CLK_FREQ * m * 2)/(p << s));
	else if (pllreg == UPLL)
		return (CONFIG_SYS_CLK_FREQ * m) / (p << s);
	else
		return 0;
}

/* return FCLK frequency */
ulong get_FCLK(void)
{
	return get_PLLCLK(MPLL);
}

/* return HCLK frequency */
ulong get_HCLK(void)
{
	ulong tmp = CLKDIVN;
	u32 hdivn = (tmp >> 1) & 0x3;
	if (hdivn == 0x0)
	{
		return (get_FCLK());
	}
	else if (hdivn == 0x1)
	{
		return (get_FCLK()/2);
	}
	else if (hdivn == 0x2)
	{
		u32 tCAMDIVN= CAMDIVN;
		return ((tCAMDIVN & 0X200) ? get_FCLK()/8 : get_FCLK()/4);
	}
	else /* hdivn == 0x3 */
	{
		u32 tCAMDIVN= CAMDIVN;
		return ((tCAMDIVN & 0x100) ? get_FCLK()/6 : get_FCLK()/3);
	}
}

/* return PCLK frequency */
ulong get_PCLK(void)
{
	u32 tmp = CLKDIVN;

	return (tmp & 1) ? get_HCLK() / 2 : get_HCLK();
}

/* return UCLK frequency */
ulong get_UCLK(void)
{
	return get_PLLCLK(UPLL);
}