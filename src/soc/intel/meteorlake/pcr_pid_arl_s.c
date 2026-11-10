/* SPDX-License-Identifier: GPL-2.0-only */

#define __SIMPLE_DEVICE__

#include <commonlib/bsd/helpers.h>
#include <device/pci_type.h>
#include <intelblocks/pcr.h>
#include <soc/pci_devs.h>
#include <soc/pcr_ids.h>

/* Real sideband port of each virtual chipset GPIO port id, in the order of PID_PCH_GPIOCOM0. */
static const uint8_t pch_gpio_ports[] = { 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x69 };

bool soc_pcr_resolve_pid(uint8_t pid, pci_devfn_t *dev, uint8_t *real_pid)
{
	if (pid < PID_PCH_GPIOCOM0 || pid >= PID_PCH_GPIOCOM0 + ARRAY_SIZE(pch_gpio_ports))
		return false;

	/* The sideband interface registers of the chipset P2SB, which is 1f.1 on bus 0x80. */
	*dev = PCI_DEV(PCH_PCI_BUS, PCI_DEV_SLOT_ESPI, 1);
	*real_pid = pch_gpio_ports[pid - PID_PCH_GPIOCOM0];
	return true;
}
