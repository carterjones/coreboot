/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _SOC_METEORLAKE_GPIO_H_
#define _SOC_METEORLAKE_GPIO_H_

#include <soc/gpio_defs.h>
#include <intelblocks/gpio.h>

#define CROS_GPIO_NAME		"INTC1083"
#define CROS_GPIO_DEVICE_NAME	"INTC1083:00"

/* Enable GPIO community power management configuration */
#define MISCCFG_GPIO_PM_CONFIG_BITS (MISCCFG_GPVNNREQEN | \
	MISCCFG_GPPGCBDPCGEN | MISCCFG_GPSIDEDPCGEN | \
	MISCCFG_GPRCOMPCDLCGEN | MISCCFG_GPRTCDLCGEN | MISCCFG_GSXSLCGEN \
	| MISCCFG_GPDPCGEN | MISCCFG_GPDLCGEN)

#if CONFIG(SOC_INTEL_ARROWLAKE_S) && !defined(__ACPI__)
#include <stddef.h>
/*
 * Pads that the SMI handler locks at the end of ramstage. The board lists them; the default is
 * none. The chipset's pads are reached through its P2SB, which the sideband helper unhides in
 * SMM.
 */
const struct gpio_lock_config *mainboard_gpio_lock_config(size_t *num);
#endif

#endif
