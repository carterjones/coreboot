/* SPDX-License-Identifier: GPL-2.0-or-later */

/*
 * Devices of the chipset, on root bridge PCI1 (bus 0x80-0xdf). Include this inside
 * Device (\_SB.PCI1) after pch_root.asl.
 */

#include <intelblocks/itss.h>
#include <intelblocks/pcr.h>
#include <soc/pcr_ids.h>

/* HDA */
#include "hda.asl"

/* PCIe root ports of the chipset */
#define ARL_ACPI_PCH
#include "pcie.asl"
#undef ARL_ACPI_PCH

/* Serial IO */
#include "serialio.asl"

/* SMBus 80:1f.4 */
#include <soc/intel/common/block/acpi/acpi/smbus.asl>

/* USB XHCI 80:14.0 */
#include "xhci.asl"

/* PMC Shared SRAM 80:14.2 */
#include <soc/intel/common/block/acpi/acpi/sram.asl>

/* CSE/HECI #1 80:16.0 */
#include <soc/intel/common/block/acpi/acpi/heci.asl>
