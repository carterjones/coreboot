/* SPDX-License-Identifier: GPL-2.0-only */

#include <acpi/acpi.h>

DefinitionBlock(
	"dsdt.aml",
	"DSDT",
	ACPI_DSDT_REV_2,
	OEM_ID,
	ACPI_TABLE_CREATOR,
	COREBOOT_OEM_REVISION
)
{
	#include <acpi/dsdt_top.asl>
	#include <soc/intel/common/block/acpi/acpi/platform.asl>
	#include <soc/intel/common/block/acpi/acpi/globalnvs.asl>
	#include <cpu/intel/common/acpi/cpu.asl>

	Device (\_SB.PCI0) {
		#include <soc/intel/meteorlake/acpi/northbridge_arl_s.asl>
		#include <soc/intel/meteorlake/acpi/southbridge_arl_s.asl>
		#include <soc/intel/meteorlake/acpi/tcss.asl>
	}

	Device (\_SB.PCI1) {
		#include <soc/intel/meteorlake/acpi/pch_root_arl_s.asl>
		#include <soc/intel/meteorlake/acpi/pch_arl_s.asl>
	}

	Scope (\_SB.PCI0.LPCB)
	{
		#include "acpi/superio.asl"
	}

	#include <southbridge/intel/common/acpi/sleepstates.asl>
}
