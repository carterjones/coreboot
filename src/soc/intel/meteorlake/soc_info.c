/* SPDX-License-Identifier: GPL-2.0-only */

#include <console/console.h>
#include <device/device.h>
#include <soc/pci_devs.h>
#include <device/pci_ids.h>
#include <soc/soc_info.h>
#include <intelblocks/tcss.h>

uint8_t get_max_usb20_port(void)
{
	return CONFIG_SOC_INTEL_USB2_DEV_MAX;
}

uint8_t get_max_usb30_port(void)
{
	return CONFIG_SOC_INTEL_USB3_DEV_MAX;
}

uint8_t get_max_tcss_port(void)
{
	return MAX_TYPE_C_PORTS;
}

uint8_t get_max_tbt_pcie_port(void)
{
	return CONFIG_MAX_TBT_ROOT_PORTS;
}

uint8_t get_max_pcie_port(void)
{
	return CONFIG_MAX_ROOT_PORTS;
}

uint8_t get_max_pcie_clock(void)
{
	return CONFIG_MAX_PCIE_CLOCK_SRC;
}

uint8_t get_max_uart_port(void)
{
	return CONFIG_SOC_INTEL_UART_DEV_MAX;
}

uint8_t get_max_i2c_port(void)
{
	return CONFIG_SOC_INTEL_I2C_DEV_MAX;
}

uint8_t get_max_gspi_port(void)
{
	return CONFIG_SOC_INTEL_COMMON_BLOCK_GSPI_MAX;
}

#if CONFIG(SOC_INTEL_ARROWLAKE_S)
#if !defined(__SIMPLE_DEVICE__)
/*
 * pcidev_path_on_root() only searches the first domain. Chipset devices are in their own
 * domain, so look them up behind that domain's root bus. The P2SB and PMC stay on the SoC.
 */
DEVTREE_CONST struct device *soc_pch_pcidev(pci_devfn_t devfn, const char *func)
{
	const struct device_path domain_path = {
		.type = DEVICE_PATH_DOMAIN,
		.domain.domain_id = PCH_DOMAIN_ID,
	};
	DEVTREE_CONST struct device *domain = find_dev_path(dev_root.downstream, &domain_path);
	DEVTREE_CONST struct device *dev = NULL;

	if (!SOC_DEVFN_ON_PCH(devfn))
		return pcidev_path_on_root_debug(devfn, func);

	if (domain)
		dev = pcidev_path_behind(domain->downstream, devfn);
	if (!dev)
		printk(BIOS_ERR, "BUG: %s requests hidden %02x:%02x.%u\n", func,
		       PCH_PCI_BUS, devfn >> 3, devfn & 7);
	return dev;
}
#endif

/*
 * is_devfn_enabled() looks devices up here. Without it every chipset device, the CSE at 80:16.0
 * among them, reads as disabled. Unlike soc_pch_pcidev() this is quiet when a device is not in
 * the devicetree.
 */
DEVTREE_CONST struct device *soc_pcidev_path(pci_devfn_t devfn)
{
	const struct device_path domain_path = {
		.type = DEVICE_PATH_DOMAIN,
		.domain.domain_id = PCH_DOMAIN_ID,
	};
	DEVTREE_CONST struct device *domain;

	if (!SOC_DEVFN_ON_PCH(devfn))
		return pcidev_path_on_root(devfn);

	domain = find_dev_path(dev_root.downstream, &domain_path);
	return domain ? pcidev_path_behind(domain->downstream, devfn) : NULL;
}
#endif
