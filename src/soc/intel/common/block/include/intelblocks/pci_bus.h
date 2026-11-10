/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _INTEL_COMMON_PCI_BUS_H_
#define _INTEL_COMMON_PCI_BUS_H_

#include <device/pci_def.h>
#include <device/pci_type.h>
#include <soc/pci_devs.h>

/*
 * Most Intel SoCs have all their PCI devices on bus 0. An SoC whose chipset is a separate
 * root bus defines these two macros in soc/pci_devs.h to point chipset devices at that bus.
 */

/* Raw PCI address for a devfn, for code that cannot use the devicetree. */
#ifndef SOC_PCI_DEV_FROM_DEVFN
#define SOC_PCI_DEV_FROM_DEVFN(devfn) PCI_DEV(0, PCI_SLOT(devfn), PCI_FUNC(devfn))
#endif

/* Devicetree lookup for a chipset devfn. */
#ifndef SOC_PCH_DEVFN_TO_DEV
#define SOC_PCH_DEVFN_TO_DEV(devfn) pcidev_path_on_root(devfn)
#endif

#endif /* _INTEL_COMMON_PCI_BUS_H_ */
