/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/romstage.h>
#include <cbmem.h>
#include <console/console.h>
#include <fsp/api.h>
#include <intelblocks/cfg.h>
#include <intelblocks/cse.h>
#include <intelblocks/pmclib.h>
#include <intelblocks/smbus.h>
#include <intelblocks/thermal.h>
#include <soc/iomap.h>
#include <soc/pm.h>
#include <soc/romstage.h>
#include <soc/soc_chip.h>
#include <string.h>

/*
 * Codes between the stock ones, so that a stall in romstage points at one step. 0x50 to 0x5f are
 * unused by coreboot's post_codes.h.
 */
#define POSTCODE_ROMSTAGE_ENTRY		0x50
#define POSTCODE_ROMSTAGE_CSE_INIT_DONE	0x51
#define POSTCODE_ROMSTAGE_CSE_SYNC_DONE	0x52
#define POSTCODE_ROMSTAGE_SA_INIT_DONE	0x53
#define POSTCODE_ROMSTAGE_SMBUS_DONE	0x54
#define POSTCODE_ROMSTAGE_THERMAL_DONE	0x55
#define POSTCODE_ROMSTAGE_MEM_RETURNED	0x56
#define POSTCODE_ROMSTAGE_DISB_DONE	0x57
#define POSTCODE_ROMSTAGE_DIMM_SAVED	0x58

static void romstage_post_code(uint8_t value)
{
	if (CONFIG(SOC_INTEL_METEORLAKE_ROMSTAGE_POST_CODES))
		post_code(value);
}

void platform_fill_dimm_info_args(const DIMM_INFO *src_dimm,
	    const MEMORY_INFO_DATA_HOB *meminfo_hob,
	    struct dimm_fill_args *args)
{
	args->data_width = meminfo_hob->DataWidth;
	args->mfg_id_arg = src_dimm->MfgId;
}

void mainboard_romstage_entry(void)
{
	struct chipset_power_state *ps = pmc_get_power_state();
	bool s3wake = pmc_fill_power_state(ps) == ACPI_S3;

	romstage_post_code(POSTCODE_ROMSTAGE_ENTRY);

	/* Initialize HECI interface */
	cse_init(HECI1_BASE_ADDRESS);
	romstage_post_code(POSTCODE_ROMSTAGE_CSE_INIT_DONE);

	if (!s3wake && CONFIG(SOC_INTEL_CSE_LITE_SKU)) {
		cse_fill_bp_info();
		if (CONFIG(SOC_INTEL_CSE_LITE_SYNC_IN_ROMSTAGE))
			cse_fw_sync();
	}
	romstage_post_code(POSTCODE_ROMSTAGE_CSE_SYNC_DONE);

	/* Update coreboot timestamp table with CSE timestamps */
	if (CONFIG(SOC_INTEL_CSE_PRE_CPU_RESET_TELEMETRY))
		cse_get_telemetry_data();

	/* Program MCHBAR, DMIBAR, GDXBAR and EDRAMBAR */
	systemagent_early_init();
	romstage_post_code(POSTCODE_ROMSTAGE_SA_INIT_DONE);
	/* Program SMBus base address and enable it */
	smbus_common_init();
	romstage_post_code(POSTCODE_ROMSTAGE_SMBUS_DONE);

	/*
	 * Set low maximum temp threshold value used for dynamic thermal sensor
	 * shutdown consideration.
	 *
	 * If Dynamic Thermal Shutdown is enabled then PMC logic shuts down the
	 * thermal sensor when CPU is in a C-state and LTT >= DTS Temp.
	 */
	pch_thermal_configuration();
	romstage_post_code(POSTCODE_ROMSTAGE_THERMAL_DONE);
	fsp_memory_init(s3wake);
	romstage_post_code(POSTCODE_ROMSTAGE_MEM_RETURNED);
	pmc_set_disb();
	romstage_post_code(POSTCODE_ROMSTAGE_DISB_DONE);
	if (!s3wake)
		fsp_save_dimm_info();
	romstage_post_code(POSTCODE_ROMSTAGE_DIMM_SAVED);
}
