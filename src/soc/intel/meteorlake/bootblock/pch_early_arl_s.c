/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/io.h>
#include <device/mmio.h>
#include <device/pci_def.h>
#include <device/pci_ops.h>
#include <intelblocks/pcr.h>
#include <soc/bootblock.h>
#include <soc/iomap.h>
#include <soc/pci_devs.h>
#include <types.h>

/*
 * What the vendor firmware's FSP-T does to the SoC and the chipset after cache-as-RAM and
 * before returning to SEC (FUN_ffe720fa in the 3.40 image). coreboot does not run FSP-T, so the
 * same accesses are made here, in the same order and width. Without the bridge setup the
 * chipset die, bus 0x80, is not reachable.
 *
 * FSP-T takes these paths when the SoC eSPI device ID masked with 0xffe0 is 0xae00, as on this
 * SoC. It would put the chipset on bus 0x81 if 0:0.0 register 0xfc read 4; the chipset is on
 * bus 0x80 (PCH_PCI_BUS) on this board, so only that branch is kept. The checks FSP-T makes at
 * run time (a device answering, a lock bit, 0:0.0 register 0xfc reading 5) are kept as checks.
 * Sideband ports are reached with plain MMIO or the P2SB's message registers, step for step
 * as FSP-T does, not through the PCR helpers, which reroute some ports and add reads.
 */

#define SOC_P2SB		PCI_DEV(0, 0x1f, 1)
#define SOC_PMC			PCI_DEV(0, 0x1f, 2)
#define SOC_SMBUS		PCI_DEV(0, 0x1f, 4)
#define SOC_TRACE_HUB		PCI_DEV(0, 0x1f, 7)
#define SOC_DEV_13_3		PCI_DEV(0, 0x13, 3)
#define PCH_P2SB		PCI_DEV(PCH_PCI_BUS, 0x1f, 1)
#define PCH_PMC			PCI_DEV(PCH_PCI_BUS, 0x1f, 2)
#define PCH_SMBUS		PCI_DEV(PCH_PCI_BUS, 0x1f, 4)
#define PCH_TRACE_HUB		PCI_DEV(PCH_PCI_BUS, 0x1f, 7)
#define PCH_DEV_14_2		PCI_DEV(PCH_PCI_BUS, 0x14, 2)
#define HOST_BRIDGE		PCI_DEV(0, 0, 0)

#define SOC_PWRM_BASE		0xfe000000
#define SOC_DEV_13_3_BAR	0xfe420000
#define SOC_TRACE_HUB_BAR	0xfe200000
#define SOC_SMBUS_IO		0xefa0
#define PCH_PWRM_BASE		0xbed00000
#define PCH_DEV_14_2_BAR	0xbed3c000
#define PCH_TRACE_HUB_BAR	0xbe800000
#define PCH_ACPI_IO		0x8c00
#define PCH_SMBUS_IO		0x8d00
#define PCH_TCO_IO		0x8e00

/* SMBus TCO registers */
#define TCOBASE			0x50
#define TCOCTL			0x54

/* Sideband ports */
#define PID_SOC_PSF_PMC		0x08
#define PID_SOC_PSF_TRACE_HUB	0x07
#define PID_SOC_BRIDGE		0x2f
#define PID_PCH_PSF_PMC		0xbc
#define PID_PCH_PSF_TRACE_HUB	0xba
#define PID_PCH_DMI		0x88
#define PID_PCH_RTC		0xc3

/* PSF shadow registers of a device */
#define PSF_BAR(n)		(4 * (n))
#define PSF_PCIEN		0x1c

/* SBI opcodes: private configuration registers and configuration space */
#define SBI_CR_READ		6
#define SBI_CFG_READ		4
#define SBI_CFG_WRITE		5

/* Bridge to the chipset die */
#define BRIDGE_A		0xfed90000
#define BRIDGE_B		(BRIDGE_A + 0x1000)
#define MCH_BRIDGE2		0x6da0
#define MCH_BRIDGE2_HI		0x6da4
#define MCH_SEC_BUS		0x6da8
#define MCH_SUB_BUS		0x6da9
#define MCH_IO_BASE		0x6daa
#define MCH_IO_LIMIT		0x6dab
#define MCH_MEM_BASE		0x6dac
#define MCH_MEM_LIMIT		0x6dae
#define MCH_PREF_BASE		0x6db0
#define MCH_PREF_LIMIT		0x6db2
#define MCH_PREF_BASE_HI	0x6db4
#define MCH_BRIDGE		0x6db8
#define MCH_PREF_LIMIT_HI	0x6dc0
#define MCH_COMMAND		0x6dc4
#define MCH_TCOBASE		0x7a78

static uintptr_t soc_pcr(uint8_t pid, uint16_t offset)
{
	return CONFIG_PCR_BASE_ADDRESS + (pid << PCR_PORTID_SHIFT) + offset;
}

/* FSP-T's MMIO sideband write: write, then read back. */
static void soc_pcr_write(uint8_t pid, uint16_t offset, uint32_t value)
{
	write32p(soc_pcr(pid, offset), value);
	read32p(soc_pcr(pid, offset));
}

static void soc_pcr_and_or(uint8_t pid, uint16_t offset, uint32_t and_mask, uint32_t or_mask)
{
	soc_pcr_write(pid, offset, (read32p(soc_pcr(pid, offset)) & and_mask) | or_mask);
}

/* One message through the chipset P2SB's sideband registers, as FSP-T's FUN_ffe728f2 sends. */
static uint32_t pch_sbi(uint8_t pid, uint16_t offset, uint8_t opcode, uint32_t data)
{
	if (pci_read_config16(PCH_P2SB, PCI_VENDOR_ID) == 0xffff)
		return 0;

	while (pci_read_config16(PCH_P2SB, 0xd8) & 1)
		;
	pci_write_config32(PCH_P2SB, 0xd0, (uint32_t)pid << 24 | offset);
	pci_write_config32(PCH_P2SB, 0xdc, 0);
	pci_write_config16(PCH_P2SB, 0xd8,
			   (pci_read_config16(PCH_P2SB, 0xd8) & 0x007f) | opcode << 8);
	pci_read_config8(HOST_BRIDGE, 0xfc);
	pci_write_config16(PCH_P2SB, 0xda, 0xf000);
	pci_write_config32(PCH_P2SB, 0xd4, opcode & 1 ? data : 0);
	pci_or_config16(PCH_P2SB, 0xd8, 1);
	while (pci_read_config16(PCH_P2SB, 0xd8) & 1)
		;
	if (opcode & 1)
		return 0;
	return pci_read_config32(PCH_P2SB, 0xd4);
}

static void pch_sbi_and_or(uint8_t pid, uint16_t offset, uint8_t read_opcode, uint32_t and_mask,
			   uint32_t or_mask)
{
	uint32_t value = pch_sbi(pid, offset, read_opcode, 0);

	pch_sbi(pid, offset, read_opcode + 1, (value & and_mask) | or_mask);
}

static bool dev_absent(pci_devfn_t dev)
{
	return pci_read_config16(dev, PCI_VENDOR_ID) == 0xffff;
}

/* FUN_ffe74342: BAR0 keeps its low 16 bits; the high half is cleared only for a 64-bit BAR. */
static void bar_init_keep_low(pci_devfn_t dev, uint32_t bar)
{
	if (dev_absent(dev))
		return;
	pci_write_config16(dev, PCI_COMMAND, pci_read_config16(dev, PCI_COMMAND) & 0xfffd);
	pci_write_config32(dev, PCI_BASE_ADDRESS_0,
			   (pci_read_config32(dev, PCI_BASE_ADDRESS_0) & 0xffff) | bar);
	if (pci_read_config32(dev, PCI_BASE_ADDRESS_0) & 0x4)
		pci_write_config32(dev, PCI_BASE_ADDRESS_1, 0);
	pci_or_config16(dev, PCI_COMMAND, PCI_COMMAND_MEMORY);
}

/* FUN_ffe71e80 */
static void bar_init(pci_devfn_t dev, uint32_t bar)
{
	if (dev_absent(dev))
		return;
	pci_write_config16(dev, PCI_COMMAND, pci_read_config16(dev, PCI_COMMAND) & 0xfffd);
	pci_write_config32(dev, PCI_BASE_ADDRESS_0, bar);
	pci_write_config32(dev, PCI_BASE_ADDRESS_1, 0);
	pci_or_config16(dev, PCI_COMMAND, PCI_COMMAND_MEMORY);
}

/* FUN_ffe74c32 */
static void bridge_init(void)
{
	const uintptr_t mch = MCH_BASE_ADDRESS;
	const uintptr_t a = BRIDGE_A;
	const uintptr_t b = BRIDGE_B;

	write32p(soc_pcr(PID_SOC_BRIDGE, 0x4e00), BRIDGE_A | 0xc);
	write32p(soc_pcr(PID_SOC_BRIDGE, 0xec), read32p(soc_pcr(PID_SOC_BRIDGE, 0xec)) | 0x10);
	write32p(soc_pcr(PID_SOC_BRIDGE, 0x10),
		 (read32p(soc_pcr(PID_SOC_BRIDGE, 0x10)) & ~0x2) | 0xc);
	write32p(soc_pcr(PID_SOC_BRIDGE, 0xf00), read32p(soc_pcr(PID_SOC_BRIDGE, 0xf00)) | 0x1);
	write16p(soc_pcr(PID_SOC_BRIDGE, 0x04), read16p(soc_pcr(PID_SOC_BRIDGE, 0x04)) | 0x7);

	write32p(mch + MCH_BRIDGE, BRIDGE_A | 1);
	read32p(b);

	/* Bus numbers */
	write8p(mch + MCH_SEC_BUS, PCH_PCI_BUS);
	write8p(mch + MCH_SUB_BUS, 0xdf);
	write8p(b + 0x19, PCH_PCI_BUS);
	write8p(b + 0x1a, 0xdf);
	write8p(a + 0x19, PCH_PCI_BUS);
	write8p(a + 0x1a, 0xdf);

	/* Memory window */
	write16p(mch + MCH_MEM_BASE, 0xb000);
	write16p(mch + MCH_MEM_LIMIT, 0xbfff);
	write16p(b + 0x20, 0xb000);
	write16p(b + 0x22, 0xbfff);
	write16p(a + 0x20, 0xb000);
	write16p(a + 0x22, 0xbfff);

	/* I/O window */
	write8p(mch + MCH_IO_BASE, 0x20);
	write8p(mch + MCH_IO_LIMIT, 0x8f);
	write8p(b + 0x1c, 0x20);
	write8p(b + 0x1d, 0x8f);
	write8p(a + 0x1c, 0x20);
	write8p(a + 0x1d, 0x8f);

	/* Prefetchable memory window */
	write16p(mch + MCH_PREF_BASE, 0);
	write32p(mch + MCH_PREF_BASE_HI, 0x80);
	write16p(mch + MCH_PREF_LIMIT, 0xffff);
	write32p(mch + MCH_PREF_LIMIT_HI, 0x9f);
	write16p(b + 0x24, 0);
	write32p(b + 0x28, 0x80);
	write16p(b + 0x26, 0xffff);
	write32p(b + 0x2c, 0x9f);
	write16p(a + 0x24, 0);
	write32p(a + 0x28, 0x80);
	write16p(a + 0x26, 0xffff);
	write32p(a + 0x2c, 0x9f);

	/* I/O, memory and bus master enables */
	write16p(mch + MCH_COMMAND, read16p(mch + MCH_COMMAND) | 0x7);
	write16p(b + 0x04, read16p(b + 0x04) | 0x7);

	write32p(mch + MCH_BRIDGE2, BRIDGE_A + 0x2000);
	write32p(mch + MCH_BRIDGE2_HI, 0);
}

/* FUN_ffe74736: SoC TCO at 0x400. Bit 0 of the TCOCTL low byte is the lock. */
static void soc_tco_init(void)
{
	write16p(MCH_BASE_ADDRESS + MCH_TCOBASE, 0x0402);
	if (dev_absent(SOC_SMBUS))
		return;
	if (pci_read_config8(SOC_SMBUS, TCOCTL) & 1) {
		pci_read_config16(SOC_SMBUS, TCOBASE);
		return;
	}
	pci_write_config8(SOC_SMBUS, TCOCTL + 1,
			  pci_read_config8(SOC_SMBUS, TCOCTL + 1) & 0xfe);
	pci_write_config16(SOC_SMBUS, TCOBASE,
			   (pci_read_config16(SOC_SMBUS, TCOBASE) & 0x001f) | 0x0400);
	pci_or_config16(SOC_SMBUS, TCOCTL, 0x0101);
}

/* FUN_ffe7492b: chipset TCO, routed through the chipset's DMI port. */
static void pch_tco_init(void)
{
	if (pci_read_config8(HOST_BRIDGE, 0xfc) == 5)
		return;
	pch_sbi(PID_PCH_DMI, 0x778, SBI_CFG_WRITE, PCH_TCO_IO | 0x2);
	if (dev_absent(PCH_SMBUS))
		return;
	if (pci_read_config8(PCH_SMBUS, TCOCTL) & 1)
		return;
	pci_write_config32(PCH_SMBUS, TCOCTL, pci_read_config32(PCH_SMBUS, TCOCTL) & ~0x100);
	pci_write_config16(PCH_SMBUS, TCOBASE,
			   (pci_read_config16(PCH_SMBUS, TCOBASE) & 0x1f) | PCH_TCO_IO);
	pci_or_config16(PCH_SMBUS, TCOCTL, 0x0100);
}

/* FUN_ffe71ef5 */
static void pch_init(void)
{
	bridge_init();

	bar_init_keep_low(PCH_PMC, PCH_PWRM_BASE);
	bar_init(PCH_DEV_14_2, PCH_DEV_14_2_BAR);

	/* ACPI base of the chipset PMC, through its PSF shadow */
	pch_sbi_and_or(PID_PCH_PSF_PMC, 0xb80 + PSF_PCIEN, SBI_CR_READ, ~1, 0);
	pch_sbi_and_or(PID_PCH_PSF_PMC, 0xb80 + PSF_BAR(4), SBI_CR_READ, 0, PCH_ACPI_IO);
	pch_sbi_and_or(PID_PCH_PSF_PMC, 0xb80 + PSF_PCIEN, SBI_CR_READ, ~0, 1);

	pci_write_config16(PCH_SMBUS, 0x20, PCH_SMBUS_IO);

	pch_sbi_and_or(PID_PCH_DMI, 0xd0, SBI_CFG_READ, ~0, 0xc0000000);
	pch_tco_init();
	outw(inw(PCH_TCO_IO + 0x08) | 0x0800, PCH_TCO_IO + 0x08);
	pch_sbi_and_or(PID_PCH_DMI, 0x5f0, SBI_CFG_READ, 0xfffffc00, 0x215);

	if (pci_read_config32(PCH_TRACE_HUB, PCI_VENDOR_ID) == 0xffffffff)
		return;
	pci_write_config32(PCH_TRACE_HUB, 0x70, PCH_TRACE_HUB_BAR);
	pch_sbi_and_or(PID_PCH_PSF_TRACE_HUB, 0xa00 + PSF_BAR(0), SBI_CR_READ, 0,
		       PCH_TRACE_HUB_BAR);
	pch_sbi_and_or(PID_PCH_PSF_TRACE_HUB, 0xa00 + PSF_PCIEN, SBI_CR_READ, ~0, 2);
}

void bootblock_pch_early_init(void)
{
	/* FSP-T's HPET setup, before its microcode load and cache-as-RAM */
	pci_write_config8(SOC_P2SB, 0x60, 0x80);
	write32p(0xfed00108, 0);
	write32p(0xfed0010c, 0);

	pci_write_config32(SOC_P2SB, PCI_BASE_ADDRESS_0, CONFIG_PCR_BASE_ADDRESS);
	pci_or_config8(SOC_P2SB, PCI_COMMAND, PCI_COMMAND_MEMORY);
	bar_init_keep_low(SOC_PMC, SOC_PWRM_BASE);

	/* ACPI base of the SoC PMC, through its PSF shadow */
	soc_pcr_and_or(PID_SOC_PSF_PMC, 0x280 + PSF_PCIEN, ~1, 0);
	soc_pcr_and_or(PID_SOC_PSF_PMC, 0x280 + PSF_BAR(4), 0, ACPI_BASE_ADDRESS);
	soc_pcr_and_or(PID_SOC_PSF_PMC, 0x280 + PSF_PCIEN, ~0, 1);

	bar_init(SOC_DEV_13_3, SOC_DEV_13_3_BAR);
	soc_tco_init();
	pci_write_config16(SOC_SMBUS, 0x20, SOC_SMBUS_IO);

	pch_init();

	/* Upper 128 bytes of CMOS: the RTC is on the chipset */
	pch_sbi_and_or(PID_PCH_RTC, 0x3400, SBI_CR_READ, ~0, 0x4);

	if (pci_read_config32(SOC_TRACE_HUB, PCI_VENDOR_ID) == 0xffffffff)
		return;
	pci_write_config32(SOC_TRACE_HUB, 0x70, SOC_TRACE_HUB_BAR);
	pci_write_config32(SOC_TRACE_HUB, 0x74, 0);
	soc_pcr_and_or(PID_SOC_PSF_TRACE_HUB, 0x100 + PSF_BAR(0), 0, SOC_TRACE_HUB_BAR);
	soc_pcr_and_or(PID_SOC_PSF_TRACE_HUB, 0x100 + PSF_BAR(1), 0, 0);
	soc_pcr_and_or(PID_SOC_PSF_TRACE_HUB, 0x100 + PSF_PCIEN, ~0, 2);
}
