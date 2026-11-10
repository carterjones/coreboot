/* SPDX-License-Identifier: GPL-2.0-only */

#include <acpi/acpi.h>
#include <bootstate.h>
#include <cbmem.h>
#include <commonlib/memory_info.h>
#include <console/console.h>
#include <device/device.h>
#include <intelblocks/cse.h>
#include <soc/ramstage.h>
#include <types.h>

void mainboard_fill_fadt(acpi_fadt_t *fadt)
{
	fadt->preferred_pm_profile = PM_DESKTOP;
	fadt->iapc_boot_arch |= ACPI_FADT_LEGACY_DEVICES | ACPI_FADT_8042;
}

/* Not reported anywhere else: the speed FSP-M trained the DIMMs at. */
static void report_dram_speed(void *unused)
{
	const struct memory_info *info = cbmem_find(CBMEM_ID_MEMINFO);

	if (info && info->dimm_cnt)
		printk(BIOS_INFO, "DRAM trained at %u MT/s (max %u MT/s)\n",
		       info->dimm[0].configured_speed_mts, info->dimm[0].max_speed_mts);
}

BOOT_STATE_INIT_ENTRY(BS_DEV_ENUMERATE, BS_ON_ENTRY, report_dram_speed, NULL);

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
