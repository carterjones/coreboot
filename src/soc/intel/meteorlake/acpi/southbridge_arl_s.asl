/* SPDX-License-Identifier: GPL-2.0-or-later */

/*
 * Devices of the SoC die, on root bridge PCI0 (bus 0x00-0x7f). The chipset's devices are in
 * pch.asl.
 */

#include <intelblocks/itss.h>
#include <intelblocks/pcr.h>
#include <soc/pcr_ids.h>

/* SoC PCR access */
#include <soc/intel/common/acpi/pch_pcr.asl>

/* PCH clock */
#include "camera_clock_ctl.asl"

/* PCIe root ports of the SoC */
#define ARL_ACPI_SOC
#include "pcie.asl"
#undef ARL_ACPI_SOC

/* ESPI 0:1f.0, where the vendor firmware's ACPI puts the LPC bridge and the Super I/O */
#include <soc/intel/common/block/acpi/acpi/lpc.asl>

/* Telemetry 0:0a.0 */
#include "telemetry.asl"

/* IOE Shared SRAM 0:13.3 */
#include "ioe_sram.asl"

/* PCI _OSC */
#include <soc/intel/common/acpi/pci_osc.asl>
