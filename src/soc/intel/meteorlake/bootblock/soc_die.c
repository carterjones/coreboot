/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/io.h>
#include <device/mmio.h>
#include <device/device.h>
#include <device/pci_ops.h>
#include <intelblocks/fast_spi.h>
#include <intelblocks/gspi.h>
#include <intelblocks/lpc_lib.h>
#include <intelblocks/p2sb.h>
#include <intelblocks/pcr.h>
#include <intelblocks/pmclib.h>
#include <intelblocks/rtc.h>
#include <intelblocks/systemagent.h>
#include <intelblocks/tco.h>
#include <intelblocks/uart.h>
#include <intelpch/espi.h>
#include <soc/bootblock.h>
#include <soc/iomap.h>
#include <soc/p2sb.h>
#include <soc/pci_devs.h>
#include <soc/pcr_ids.h>
#include <soc/pm.h>

#if CONFIG(SOC_INTEL_ARROWLAKE_S)
/* 0xA00 on Meteor Lake; FSP-T uses 0x280 on Arrow Lake-S. */
#define PCR_PSF8_TO_SHDW_PMC_REG_BASE	0x280
#else
#define PCR_PSF8_TO_SHDW_PMC_REG_BASE	0xA00
#endif
#define PCR_PSFX_TO_SHDW_BAR0	0
#define PCR_PSFX_TO_SHDW_BAR1	0x4
#define PCR_PSFX_TO_SHDW_BAR2	0x8
#define PCR_PSFX_TO_SHDW_BAR3	0xC
#define PCR_PSFX_TO_SHDW_BAR4	0x10
#define PCR_PSFX_TO_SHDW_PCIEN_IOEN	0x01
#define PCR_PSFX_T0_SHDW_PCIEN	0x1C

#define PCR_DMI_ACPIBA		0x27B4
#define PCR_DMI_ACPIBDID	0x27B8
#define PCR_DMI_PMBASEA		0x27AC
#define PCR_DMI_PMBASEC		0x27B0

static void soc_die_config_pwrmbase(void)
{
	/*
	 * Assign Resources to PWRMBASE
	 * Clear BIT 1-2  Command Register
	 */
	pci_and_config16(PCI_DEV_PMC, PCI_COMMAND, ~(PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER));

	/* Program PWRM Base */
	pci_write_config32(PCI_DEV_PMC, PWRMBASE, PCH_PWRM_BASE_ADDRESS);

	/* Enable Bus Master and MMIO Space */
	pci_or_config16(PCI_DEV_PMC, PCI_COMMAND, (PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER));

	/* Enable PWRM in PMC */
	setbits32((void *)PCH_PWRM_BASE_ADDRESS + ACTL, PWRM_EN);
}

#if CONFIG(SOC_INTEL_ARROWLAKE_S)
/* Offsets in the eSPI controller's config space, the same on the SoC and the chipset. */
#define ESPI_IO_DECODE			0x80
#define ESPI_IO_ENABLES			0x82
#define ESPI_GENERIC_IO_RANGE(n)	(0x84 + 4 * (n))

/*
 * Program the SoC's own eSPI controller (bus 0) with the same decode as the chipset's, which
 * the vendor firmware does too. This does not read the chipset's registers: bus 0x80 may not
 * be reachable this early, and a read of 0xffffffff would be written back as all ones.
 */
static void soc_die_espi_decode_init(uint16_t io_decode, uint16_t io_enables)
{
	const pci_devfn_t espi = PCI_DEV(0, PCI_DEV_SLOT_ESPI, 0);
	uint32_t gen_io_dec[LPC_NUM_GENERIC_IO_RANGES];

	soc_get_gen_io_dec_range(gen_io_dec);
	for (unsigned int i = 0; i < LPC_NUM_GENERIC_IO_RANGES; i++)
		pci_write_config32(espi, ESPI_GENERIC_IO_RANGE(i), gen_io_dec[i]);

	pci_write_config16(espi, ESPI_IO_DECODE, io_decode);
	pci_or_config16(espi, ESPI_IO_ENABLES, io_enables);
}

/*
 * The vendor firmware's SEC writes 0x0010 to the fixed decode (COM A at 0x3f8, COM B at 0x2f8)
 * and ORs 0x7f0f into the enables, on both eSPI controllers and the IOC mirror, before its
 * first POST code that reaches the UART1 header. The same values are used here. Bit 14 of the
 * enables has no name in coreboot.
 */
#define VENDOR_IO_DECODE	0x0010
#define VENDOR_IOE_BIT14	(1 << 14)

static void soc_die_early_iorange_init(void)
{
	uint16_t io_enables = LPC_IOE_COMA_EN | LPC_IOE_COMB_EN |
			      LPC_IOE_LPT_EN | LPC_IOE_FDD_EN |
			      LPC_IOE_LGE_200 | LPC_IOE_HGE_208 |
			      LPC_IOE_KBC_60_64 | LPC_IOE_EC_62_66 |
			      LPC_IOE_SUPERIO_2E_2F | LPC_IOE_EC_4E_4F |
			      VENDOR_IOE_BIT14;

	/* The vendor SEC's first chipset access: halt the TCO timer at 0x400 (TCO1_CNT). */
	outw(0x0800, TCO_BASE_ADDRESS + 0x08);

	/* IO Decode Range */
	lpc_set_fixed_io_ranges(VENDOR_IO_DECODE, 0xffff);

	/* IO Decode Enable */
	lpc_enable_fixed_io_ranges(io_enables);

	/* Program generic IO Decode Range */
	pch_enable_lpc();

	/* The vendor firmware gives the SoC's eSPI controller the same decode settings. */
	soc_die_espi_decode_init(VENDOR_IO_DECODE, io_enables);
}
#else
static void soc_die_early_iorange_init(void)
{
	uint16_t io_enables = LPC_IOE_LPT_EN | LPC_IOE_FDD_EN |
			      LPC_IOE_LGE_200 | LPC_IOE_HGE_208 |
			      LPC_IOE_KBC_60_64 | LPC_IOE_EC_62_66 |
			      LPC_IOE_SUPERIO_2E_2F | LPC_IOE_EC_4E_4F;

	/* IO Decode Range */
	if (CONFIG(DRIVERS_UART_8250IO))
		lpc_io_setup_comm_a_b();

	/* IO Decode Enable */
	lpc_enable_fixed_io_ranges(io_enables);

	/* Program generic IO Decode Range */
	pch_enable_lpc();
}
#endif

static void soc_die_early_ip_init(void)
{
	/*
	 * Perform P2SB configuration before any another controller initialization as the
	 * controller might want to perform PCR settings.
	 */
	p2sb_enable_bar();
	p2sb_configure_hpet();

	/* FSP-T's SoC and chipset setup, including the bridge to bus 0x80 */
	if (CONFIG(SOC_INTEL_ARROWLAKE_S))
		bootblock_pch_early_init();

	fast_spi_early_init(SPI_BASE_ADDRESS);
	gspi_early_bar_init();

	/*
	 * Enabling SoC PMC PWRM Base for accessing
	 * Global Reset Cause Register.
	 */
	soc_die_config_pwrmbase();
}

static void soc_die_early_sa_init(void)
{
	const struct sa_mmio_descriptor soc_fixed_pci_resources[] = {
		{ MCHBAR, MCH_BASE_ADDRESS, MCH_BASE_SIZE, "MCHBAR" },
	};

	bootblock_systemagent_early_init();

	/* Enable MCHBAR early, needed by IOC driver */
	sa_set_pci_bar(soc_fixed_pci_resources, ARRAY_SIZE(soc_fixed_pci_resources));
}

void bootblock_soc_die_early_init(void)
{
	/*
	 * The vendor's reset vector code: a PCIEXBAR that is already set means the CPU was reset
	 * without the chipset, and it forces a full reset through 0xcf9.
	 */
	if (CONFIG(SOC_INTEL_ARROWLAKE_S) && pci_io_read_config32(PCI_DEV(0, 0, 0), PCIEXBAR))
		outb(0x06, 0xcf9);

	/*
	 * Ensure performing SA related programming including MCHBAR prior to accessing
	 * IOC driver.
	 */
	soc_die_early_sa_init();

	soc_die_early_ip_init();

	fast_spi_cache_bios_region();
	soc_die_early_iorange_init();
	if (CONFIG(INTEL_LPSS_UART_FOR_CONSOLE))
		uart_bootblock_init();
}

static void soc_die_config_acpibase(void)
{
	uint32_t pmc_reg_value;
	uint32_t pmc_base_reg = PCR_PSF8_TO_SHDW_PMC_REG_BASE;

	pmc_reg_value = pcr_read32(PID_PSF8, pmc_base_reg + PCR_PSFX_TO_SHDW_BAR4);

	if (pmc_reg_value != 0xffffffff) {
		/* Disable Io Space before changing the address */
		pcr_rmw32(PID_PSF8, pmc_base_reg + PCR_PSFX_T0_SHDW_PCIEN,
				~PCR_PSFX_TO_SHDW_PCIEN_IOEN, 0);
		/* Program ABASE in PSF3 PMC space BAR4 */
		pcr_write32(PID_PSF8, pmc_base_reg + PCR_PSFX_TO_SHDW_BAR4,
				ACPI_BASE_ADDRESS);
		/* Enable IO Space */
		pcr_rmw32(PID_PSF8, pmc_base_reg + PCR_PSFX_T0_SHDW_PCIEN,
				~0, PCR_PSFX_TO_SHDW_PCIEN_IOEN);
	}
}

void bootblock_soc_die_init(void)
{
	/*
	 * Enabling ABASE for accessing PM1_STS, PM1_EN, PM1_CNT,
	 * GPE0_STS, GPE0_EN registers.
	 */
	soc_die_config_acpibase();

	/* Set up GPE configuration */
	pmc_gpe_init();

	/*
	 * Arrow Lake-S has no enable_rtc_upper_bank() or tco_configure() here. The RTC is on the
	 * chipset, and bootblock_pch_early_init() has set its upper bank. The SoC TCO is at 0x400
	 * and the chipset TCO at 0x8e00, both halted; tco_configure() would move the chipset TCO
	 * onto 0x400.
	 */
	if (CONFIG(SOC_INTEL_ARROWLAKE_S))
		return;

	enable_rtc_upper_bank();

	/* Programming TCO_BASE_ADDRESS and TCO Timer Halt */
	tco_configure();
}
