/* SPDX-License-Identifier: GPL-2.0-only */

#include <intelblocks/gpio.h>
#include <soc/gpio.h>

/*
 * The TPM module on the header takes its reset from the chipset's PLTRST# pad. Left unlocked, the
 * pad can be turned into a GPIO and driven to reset the TPM while the system runs, which clears the
 * PCRs (the attack in Heads' doc/TPM_GPIO_Reset_Vulnerability.md). The vendor firmware locks the
 * pad's configuration and output state. That this pad is PLTRST# is inferred from the PCH family
 * and from its state on the running board; nothing in the images names it.
 */
const struct gpio_lock_config *mainboard_gpio_lock_config(size_t *num)
{
	static const struct gpio_lock_config pads[] = {
		{ PCH_GPP_B13, GPIO_LOCK_FULL },
	};

	*num = ARRAY_SIZE(pads);
	return pads;
}
