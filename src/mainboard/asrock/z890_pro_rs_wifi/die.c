/* SPDX-License-Identifier: GPL-2.0-only */

#include <console/console.h>
#include <delay.h>

#include "pmic.h"

/* Unused by coreboot's post_codes.h and by the FSP, whose codes are 0x9x and 0xax. */
#define POSTCODE_BOARD_DIE	0x5f

/*
 * A failure is reported on the UART1 POST header, which carries the bytes written to port 0x80.
 * The code is repeated so that a capture started late still sees it. A die_with_post_code()
 * caller's own code comes just before the first 0x5f.
 */
void die_notify(void)
{
	if (ENV_POSTCAR)
		return;

	/*
	 * A failed memory training: the PMIC state (programmable mode R2F bit 2, enable R32 bit 7)
	 * is the first thing to check.
	 */
	if (ENV_RAMINIT)
		z890_dump_pmics("after failure");

	while (1) {
		post_code(POSTCODE_BOARD_DIE);
		mdelay(1000);
	}
}
