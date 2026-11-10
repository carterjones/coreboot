/* SPDX-License-Identifier: GPL-2.0-only */

#include <acpi/acpi.h>
#include <device/device.h>
#include <intelblocks/cse.h>
#include <soc/ramstage.h>
#include <types.h>

void mainboard_fill_fadt(acpi_fadt_t *fadt)
{
	fadt->preferred_pm_profile = PM_DESKTOP;
	fadt->iapc_boot_arch |= ACPI_FADT_LEGACY_DEVICES | ACPI_FADT_8042;
}

void mainboard_silicon_init_params(FSP_S_CONFIG *params)
{
	/*
	 * Use the TPM module on the header instead of Intel PTT. The vendor firmware does the same
	 * when its Intel PTT setting is off. This has to happen before End of Post, and when it
	 * changes the ME's state it triggers a global reset, after which the module is usable.
	 */
	if (CONFIG(Z890_PTT_OFF))
		cse_enable_ptt(false);
}
