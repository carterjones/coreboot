/* SPDX-License-Identifier: GPL-2.0-only */

#include <bootblock_common.h>
#include <gpio.h>

#include "gpio.h"

void bootblock_mainboard_init(void)
{
	gpio_configure_pads(early_gpio_table, early_gpio_table_size);

	/* The chipset's pads are reached through its P2SB, which must be visible here. */
	gpio_configure_pads(pch_gpio_table, pch_gpio_table_size);
}
