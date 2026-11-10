/* SPDX-License-Identifier: GPL-2.0-only */

#include <console/console.h>
#include <delay.h>
#include <device/smbus_host.h>
#include <soc/meminit.h>
#include <soc/romstage.h>
#include <string.h>
#include <types.h>

#include "pmic.h"

static const struct mb_cfg ddr5_mem_config = {
	.type = MEM_TYPE_DDR5,
	.ect = true,
	.UserBd = BOARD_TYPE_DESKTOP,
	.LpDdrDqDqsReTraining = 1,
	.ddr_config = {
		.dq_pins_interleaved = true,
	},
};

static const struct mem_spd dimm_module_spd_info = {
	.topo = MEM_TOPO_DIMM_MODULE,
	.smbus = {
		[0] = {
			.addr_dimm[0] = 0x50,
			.addr_dimm[1] = 0x51,
		},
		[1] = {
			.addr_dimm[0] = 0x52,
			.addr_dimm[1] = 0x53,
		},
	},
};

/*
 * Print the DDR5 PMIC registers of each DIMM, for a failed memory training (die_notify()).
 * Reads only; a PMIC is at SMBus 0x48 plus the low three bits of its DIMM's SPD hub address.
 */
void z890_dump_pmics(const char *when)
{
	static const u8 spd_addr[] = { 0x50, 0x51, 0x52, 0x53 };

	for (size_t i = 0; i < ARRAY_SIZE(spd_addr); i++) {
		const u8 pmic = 0x48 | (spd_addr[i] & 7);

		printk(BIOS_DEBUG, "PMIC 0x%02x %s:", pmic, when);
		for (u8 reg = 0; reg < 0x40; reg++) {
			const int value = smbus_read_byte(pmic, reg);
			if (value < 0)
				printk(BIOS_DEBUG, " --");
			else
				printk(BIOS_DEBUG, " %02x", value);
		}
		printk(BIOS_DEBUG, "\n");
	}
}

/* DDR5 PMIC registers the setup below reads or writes (JESD301 names where known). */
#define PMIC_R14_CLEAR		0x14
#define PMIC_R21_SWA_VOLT	0x21
#define PMIC_R25_SWC_VOLT	0x25
#define PMIC_R27_LDO_VOLT	0x27
#define PMIC_R2B_VOLT_RANGE	0x2b
#define PMIC_R2F_MODE		0x2f
#define  PMIC_R2F_PROGRAMMABLE	(1 << 2)
#define PMIC_R32_VR_CTRL	0x32
#define  PMIC_R32_VR_ENABLE	(1 << 7)
#define PMIC_R3C_VENDOR_ID0	0x3c
#define PMIC_R3D_VENDOR_ID1	0x3d
#define PMIC_RF8_VENDOR		0xf8	/* Vendor specific, meaning not documented */
#define  PMIC_RF8_UNLOCK	(1 << 7)

/* Read a PMIC register into *value; false on an SMBus error. */
static bool pmic_read(u8 pmic, u8 reg, u8 *value)
{
	const int ret = smbus_read_byte(pmic, reg);

	if (ret < 0) {
		printk(BIOS_ERR, "PMIC 0x%02x: read of R%02X failed\n", pmic, reg);
		return false;
	}
	*value = ret;
	return true;
}

static bool pmic_write(u8 pmic, u8 reg, u8 value)
{
	if (smbus_write_byte(pmic, reg, value) < 0) {
		printk(BIOS_ERR, "PMIC 0x%02x: write of R%02X failed\n", pmic, reg);
		return false;
	}
	return true;
}

/*
 * Put a DIMM PMIC with vendor ID 0x2a0b in programmable mode and enable it, as the vendor
 * firmware's DDR5PMICPei does before its MRC on every boot (routine 0xfff0567f at default
 * Setup). The PMICs keep this through a warm reset and lose it in S5, and the IoT FSP-M's own
 * setup, which lacks the RF8 bracket, leaves them as they were; memory training then fails. No
 * voltage register is written: the PMIC is left alone unless its voltages already read the
 * vendor's targets, 1100, 1100 and 1800 mV.
 */
static void pmic_setup(u8 pmic)
{
	u8 id0, id1, range, swa, swc, ldo, mode, vendor, ctrl;

	if (!pmic_read(pmic, PMIC_R3C_VENDOR_ID0, &id0) ||
	    !pmic_read(pmic, PMIC_R3D_VENDOR_ID1, &id1) ||
	    !pmic_read(pmic, PMIC_R2B_VOLT_RANGE, &range) ||
	    !pmic_read(pmic, PMIC_R21_SWA_VOLT, &swa) ||
	    !pmic_read(pmic, PMIC_R25_SWC_VOLT, &swc) ||
	    !pmic_read(pmic, PMIC_R27_LDO_VOLT, &ldo) ||
	    !pmic_read(pmic, PMIC_R2F_MODE, &mode))
		return;

	if (id0 != 0x0b || id1 != 0x2a) {
		printk(BIOS_INFO, "PMIC 0x%02x: vendor ID %02x%02x, left alone\n", pmic, id1, id0);
		return;
	}
	if (range != 0x42 || swa != 0x78 || swc != 0x78 || ldo != 0x78) {
		printk(BIOS_WARNING, "PMIC 0x%02x: voltages R2B %02x R21 %02x R25 %02x R27 %02x "
		       "not the expected 42 78 78 78, left alone\n", pmic, range, swa, swc, ldo);
		return;
	}
	if (mode & PMIC_R2F_PROGRAMMABLE) {
		printk(BIOS_INFO, "PMIC 0x%02x: already programmable (R2F %02x)\n", pmic, mode);
	} else {
		if (!pmic_read(pmic, PMIC_RF8_VENDOR, &vendor) ||
		    !pmic_write(pmic, PMIC_RF8_VENDOR, vendor | PMIC_RF8_UNLOCK))
			return;
		pmic_write(pmic, PMIC_R2F_MODE, mode | PMIC_R2F_PROGRAMMABLE);
		mdelay(10);
		if (!pmic_read(pmic, PMIC_RF8_VENDOR, &vendor) ||
		    !pmic_write(pmic, PMIC_RF8_VENDOR, vendor & ~PMIC_RF8_UNLOCK))
			return;
	}

	if (!pmic_write(pmic, PMIC_R14_CLEAR, 0x01) ||
	    !pmic_read(pmic, PMIC_R32_VR_CTRL, &ctrl))
		return;
	if (!(ctrl & PMIC_R32_VR_ENABLE)) {
		if (!pmic_write(pmic, PMIC_R32_VR_CTRL, ctrl | PMIC_R32_VR_ENABLE))
			return;
		/* The vendor's wait from the R2C/R2D timing fields, 8 ms on these PMICs. */
		mdelay(8);
	}

	if (pmic_read(pmic, PMIC_R2F_MODE, &mode) && pmic_read(pmic, PMIC_R32_VR_CTRL, &ctrl))
		printk(BIOS_INFO, "PMIC 0x%02x: R2F %02x R32 %02x after setup\n", pmic, mode, ctrl);
}

/* A PMIC is at SMBus 0x48 plus the low three bits of its DIMM's SPD hub address. */
static void pmics_setup(void)
{
	static const u8 spd_addr[] = { 0x50, 0x51, 0x52, 0x53 };

	for (size_t i = 0; i < ARRAY_SIZE(spd_addr); i++)
		pmic_setup(0x48 | (spd_addr[i] & 7));
}

#define FSP_CLK_NOTUSED			0xFF
#define FSP_CLK_FREE_RUNNING		0x80

/*
 * Which clock source and CLKREQ# pin serve which root port is not known for this board, and
 * the vendor firmware's own tables were not found. A source marked free running is always on,
 * so every PCIe device gets its reference clock whatever the wiring is, with no CLKREQ#
 * dependence. That costs the clock gating a CLKREQ# based power saving would give, which the
 * other Intel desktop boards in coreboot and Heads also go without.
 */
static void run_all_pcie_clocks_free(FSP_M_CONFIG *m_cfg)
{
	memset(m_cfg->PcieClkSrcUsage, FSP_CLK_FREE_RUNNING, sizeof(m_cfg->PcieClkSrcUsage));
	memset(m_cfg->PchPcieClkSrcUsage, FSP_CLK_FREE_RUNNING,
	       sizeof(m_cfg->PchPcieClkSrcUsage));
	memset(m_cfg->PcieClkSrcClkReq, FSP_CLK_NOTUSED, sizeof(m_cfg->PcieClkSrcClkReq));
	memset(m_cfg->PchPcieClkSrcClkReq, FSP_CLK_NOTUSED, sizeof(m_cfg->PchPcieClkSrcClkReq));
}

void mainboard_memory_init_params(FSPM_UPD *memupd)
{
	/* DRAM ends at 2 GiB on the vendor firmware, with the PCI hole above it. */
	memupd->FspmConfig.MmioSize = 0x800;

	memcfg_init(memupd, &ddr5_mem_config, &dimm_module_spd_info, false);

	pmics_setup();

	run_all_pcie_clocks_free(&memupd->FspmConfig);

	/*
	 * FSP's own POST codes on port 0x80 instead of I2C ("Enable (Default): Postcode via I2C,
	 * Disable: Postcode via Port80", FspmUpd.h), so they reach the UART1 header with
	 * coreboot's.
	 */
	memupd->FspmConfig.I2cPostCodeEnable = 0;

	/*
	 * The iGPU is the primary display, for the payload's framebuffer on the board's HDMI.
	 * On Auto (the default, 3) a card in PCIE1 becomes primary and FSP-S's GOP leaves the
	 * iGPU dark: no graphics hand-off block. System76's dGPU boards do the same
	 * (src/mainboard/system76/adl).
	 */
	memupd->FspmConfig.PrimaryDisplay = 0;
}
