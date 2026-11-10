/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <gpio.h>
#include <intelblocks/pcr.h>
#include <soc/pcr_ids.h>
#include <soc/pmc.h>

static const struct reset_mapping rst_map[] = {
	{ .logical = PAD_RESET(PWROK), .chipset = 0U << 30 },
	{ .logical = PAD_RESET(DEEP), .chipset = 1U << 30 },
	{ .logical = PAD_RESET(PLTRST), .chipset = 2U << 30 },
	{ .logical = PAD_RESET(GLBRST), .chipset = 3U << 30 },
};

/*
 * The pad numbers in a group follow the order of the pad configuration registers. The ACPI base
 * of each group is the Linux numbering (32 for every group), which pinctrl-meteorlake uses for
 * its Meteor Lake-S tables. No GPIO device is described in ACPI on this board, as in the vendor
 * firmware, so these numbers are only used if a board asks for them.
 */
static const struct pad_group arl_community0_groups[] = {
	INTEL_GPP_BASE(GPP_A00, GPP_A00, GPP_A27, 0),			/* GPP_A */
	INTEL_GPP_BASE(GPP_A00, GPP_VGPIO_LPC00, GPP_VGPIO_LPC17, 32),	/* GPP_VGPIO_LPC */
	INTEL_GPP_BASE(GPP_A00, GPP_C00, GPP_C26, 64),			/* GPP_C */
};

static const struct pad_group arl_community1_groups[] = {
	INTEL_GPP_BASE(GPP_B00, GPP_B00, GPP_B19, 96),			/* GPP_B */
	INTEL_GPP_BASE(GPP_B00, GPP_VGPIO_USB00, GPP_VGPIO_USB01, 128),	/* GPP_VGPIO_USB */
	INTEL_GPP_BASE(GPP_B00, GPP_D00, GPP_D23, 160),			/* GPP_D */
};

static const struct pad_group arl_community3_groups[] = {
	INTEL_GPP_BASE(GPP_JTAG00, GPP_JTAG00, GPP_JTAG15, 192),	/* GPP_JTAG */
	INTEL_GPP_BASE(GPP_JTAG00, GPP_VGPIO_ISCLK00, GPP_VGPIO_ISCLK01, 224), /* GPP_VGPIO_ISCLK */
};

/*
 * The chipset's communities. Each one has a register block for its groups in the order listed
 * here, so the order of the groups is the order of their registers.
 */
static const struct pad_group arl_pch_community0_groups[] = {
	INTEL_GPP(PCH_GPP_D00, PCH_GPP_D00, PCH_GPP_D24),		/* GPP_D */
	INTEL_GPP(PCH_GPP_D00, PCH_GPP_R00, PCH_GPP_R13),		/* GPP_R */
	INTEL_GPP(PCH_GPP_D00, PCH_GPP_J00, PCH_GPP_J17),		/* GPP_J */
	INTEL_GPP(PCH_GPP_D00, PCH_VGPIO00, PCH_VGPIO30),		/* vGPIO */
};

static const struct pad_group arl_pch_community1_groups[] = {
	INTEL_GPP(PCH_GPP_A00, PCH_GPP_A00, PCH_GPP_A14),		/* GPP_A */
	INTEL_GPP(PCH_GPP_A00, PCH_DIR_ESPI00, PCH_DIR_ESPI11),	/* DIR_ESPI */
	INTEL_GPP(PCH_GPP_A00, PCH_GPP_B00, PCH_GPP_B21),		/* GPP_B */
};

static const struct pad_group arl_pch_community2_groups[] = {
	INTEL_GPP(PCH_DSW00, PCH_DSW00, PCH_DSW16),			/* DSW */
};

static const struct pad_group arl_pch_community3_groups[] = {
	INTEL_GPP(PCH_SPI0_00, PCH_SPI0_00, PCH_SPI0_08),		/* SPI0 */
	INTEL_GPP(PCH_SPI0_00, PCH_GPP_C00, PCH_GPP_C23),		/* GPP_C */
	INTEL_GPP(PCH_SPI0_00, PCH_GPP_H00, PCH_GPP_H19),		/* GPP_H */
	INTEL_GPP(PCH_SPI0_00, PCH_VGPIO3_00, PCH_VGPIO3_03),		/* vGPIO_3 */
	INTEL_GPP(PCH_SPI0_00, PCH_VGPIO0_00, PCH_VGPIO0_07),		/* vGPIO_0 */
	INTEL_GPP(PCH_SPI0_00, PCH_VGPIO4_00, PCH_VGPIO4_30),		/* vGPIO_4 */
};

static const struct pad_group arl_pch_community4_groups[] = {
	INTEL_GPP(PCH_GPP_S00, PCH_GPP_S00, PCH_GPP_S07),		/* GPP_S */
	INTEL_GPP(PCH_GPP_S00, PCH_GPP_E00, PCH_GPP_E22),		/* GPP_E */
	INTEL_GPP(PCH_GPP_S00, PCH_GPP_K00, PCH_GPP_K13),		/* GPP_K */
	INTEL_GPP(PCH_GPP_S00, PCH_GPP_F00, PCH_GPP_F23),		/* GPP_F */
};

static const struct pad_group arl_pch_community5_groups[] = {
	INTEL_GPP(PCH_GPP_I00, PCH_GPP_I00, PCH_GPP_I20),		/* GPP_I */
	INTEL_GPP(PCH_GPP_I00, PCH_JTAG_CPU00, PCH_JTAG_CPU15),	/* JTAG_CPU */
};

static const struct pad_community arl_communities[] = {
	[COMM_0] = { /* GPP_A, GPP_VGPIO_LPC, GPP_C */
		.port = PID_GPIOCOM0,
		.cpu_port = PID_GPIOCOM0,
		.first_pad = GPIO_COM0_START,
		.last_pad = GPIO_COM0_END,
		.num_gpi_regs = NUM_GPIO_COM0_GPI_REGS,
		.pad_cfg_base = PAD_CFG_BASE,
		.pad_cfg_lock_offset = PAD_CFG_LOCK_OFFSET,
		.host_own_reg_0 = HOSTSW_OWN_REG_0,
		.gpi_int_sts_reg_0 = GPI_INT_STS_0,
		.gpi_int_en_reg_0 = GPI_INT_EN_0,
		.gpi_gpe_sts_reg_0 = GPI_GPE_STS_0,
		.gpi_gpe_en_reg_0 = GPI_GPE_EN_0,
		.gpi_smi_sts_reg_0 = GPI_SMI_STS_0,
		.gpi_smi_en_reg_0 = GPI_SMI_EN_0,
		.gpi_nmi_sts_reg_0 = GPI_NMI_STS_0,
		.gpi_nmi_en_reg_0 = GPI_NMI_EN_0,
		.max_pads_per_group = GPIO_MAX_NUM_PER_GROUP,
		.name = "GPP_AC",
		.acpi_path = "\\_SB.PCI0.GPIO",
		.reset_map = rst_map,
		.num_reset_vals = ARRAY_SIZE(rst_map),
		.groups = arl_community0_groups,
		.num_groups = ARRAY_SIZE(arl_community0_groups),
	},
	[COMM_1] = { /* GPP_B, GPP_VGPIO_USB, GPP_D */
		.port = PID_GPIOCOM1,
		.cpu_port = PID_GPIOCOM1,
		.first_pad = GPIO_COM1_START,
		.last_pad = GPIO_COM1_END,
		.num_gpi_regs = NUM_GPIO_COM1_GPI_REGS,
		.pad_cfg_base = PAD_CFG_BASE,
		.pad_cfg_lock_offset = PAD_CFG_LOCK_OFFSET,
		.host_own_reg_0 = HOSTSW_OWN_REG_0,
		.gpi_int_sts_reg_0 = GPI_INT_STS_0,
		.gpi_int_en_reg_0 = GPI_INT_EN_0,
		.gpi_gpe_sts_reg_0 = GPI_GPE_STS_0,
		.gpi_gpe_en_reg_0 = GPI_GPE_EN_0,
		.gpi_smi_sts_reg_0 = GPI_SMI_STS_0,
		.gpi_smi_en_reg_0 = GPI_SMI_EN_0,
		.gpi_nmi_sts_reg_0 = GPI_NMI_STS_0,
		.gpi_nmi_en_reg_0 = GPI_NMI_EN_0,
		.max_pads_per_group = GPIO_MAX_NUM_PER_GROUP,
		.name = "GPP_BD",
		.acpi_path = "\\_SB.PCI0.GPIO",
		.reset_map = rst_map,
		.num_reset_vals = ARRAY_SIZE(rst_map),
		.groups = arl_community1_groups,
		.num_groups = ARRAY_SIZE(arl_community1_groups),
	},
	[COMM_3] = { /* GPP_JTAG, GPP_VGPIO_ISCLK */
		.port = PID_GPIOCOM3,
		.cpu_port = PID_GPIOCOM3,
		.first_pad = GPIO_COM3_START,
		.last_pad = GPIO_COM3_END,
		.num_gpi_regs = NUM_GPIO_COM3_GPI_REGS,
		.pad_cfg_base = PAD_CFG_BASE,
		.pad_cfg_lock_offset = PAD_CFG_LOCK_OFFSET,
		.host_own_reg_0 = HOSTSW_OWN_REG_0,
		.gpi_int_sts_reg_0 = GPI_INT_STS_0,
		.gpi_int_en_reg_0 = GPI_INT_EN_0,
		.gpi_gpe_sts_reg_0 = GPI_GPE_STS_0,
		.gpi_gpe_en_reg_0 = GPI_GPE_EN_0,
		.gpi_smi_sts_reg_0 = GPI_SMI_STS_0,
		.gpi_smi_en_reg_0 = GPI_SMI_EN_0,
		.gpi_nmi_sts_reg_0 = GPI_NMI_STS_0,
		.gpi_nmi_en_reg_0 = GPI_NMI_EN_0,
		.max_pads_per_group = GPIO_MAX_NUM_PER_GROUP,
		.name = "GPP_JTAG",
		.acpi_path = "\\_SB.PCI0.GPIO",
		.reset_map = rst_map,
		.num_reset_vals = ARRAY_SIZE(rst_map),
		.groups = arl_community3_groups,
		.num_groups = ARRAY_SIZE(arl_community3_groups),
	},
	[COMM_PCH_0] = { /* chipset port 0x6e: GPP_D, GPP_R, GPP_J, vGPIO */
		.port = PID_PCH_GPIOCOM0,
		.cpu_port = PID_PCH_GPIOCOM0,
		.first_pad = GPIO_PCH_COM0_START,
		.last_pad = GPIO_PCH_COM0_END,
		.num_gpi_regs = NUM_GPIO_PCH_COM0_GPI_REGS,
		.pad_cfg_base = PAD_CFG_BASE,
		.pad_cfg_lock_offset = PCH_PAD_CFG_LOCK_OFFSET,
		.host_own_reg_0 = PCH_HOSTSW_OWN_REG_0,
		.gpi_int_sts_reg_0 = PCH_GPI_INT_STS_0,
		.gpi_int_en_reg_0 = PCH_GPI_INT_EN_0,
		.gpi_gpe_sts_reg_0 = PCH_GPI_GPE_STS_0,
		.gpi_gpe_en_reg_0 = PCH_GPI_GPE_EN_0,
		.gpi_smi_sts_reg_0 = PCH_GPI_SMI_STS_0,
		.gpi_smi_en_reg_0 = PCH_GPI_SMI_EN_0,
		.gpi_nmi_sts_reg_0 = PCH_GPI_NMI_STS_0,
		.gpi_nmi_en_reg_0 = PCH_GPI_NMI_EN_0,
		.max_pads_per_group = GPIO_MAX_NUM_PER_GROUP,
		.name = "PCH_DRJV",
		.acpi_path = "\\_SB.PCI1.GPIO",
		.reset_map = rst_map,
		.num_reset_vals = ARRAY_SIZE(rst_map),
		.groups = arl_pch_community0_groups,
		.num_groups = ARRAY_SIZE(arl_pch_community0_groups),
	},
	[COMM_PCH_1] = { /* chipset port 0x6d: GPP_A, DIR_ESPI, GPP_B */
		.port = PID_PCH_GPIOCOM1,
		.cpu_port = PID_PCH_GPIOCOM1,
		.first_pad = GPIO_PCH_COM1_START,
		.last_pad = GPIO_PCH_COM1_END,
		.num_gpi_regs = NUM_GPIO_PCH_COM1_GPI_REGS,
		.pad_cfg_base = PAD_CFG_BASE,
		.pad_cfg_lock_offset = PCH_PAD_CFG_LOCK_OFFSET,
		.host_own_reg_0 = PCH_HOSTSW_OWN_REG_0,
		.gpi_int_sts_reg_0 = PCH_GPI_INT_STS_0,
		.gpi_int_en_reg_0 = PCH_GPI_INT_EN_0,
		.gpi_gpe_sts_reg_0 = PCH_GPI_GPE_STS_0,
		.gpi_gpe_en_reg_0 = PCH_GPI_GPE_EN_0,
		.gpi_smi_sts_reg_0 = PCH_GPI_SMI_STS_0,
		.gpi_smi_en_reg_0 = PCH_GPI_SMI_EN_0,
		.gpi_nmi_sts_reg_0 = PCH_GPI_NMI_STS_0,
		.gpi_nmi_en_reg_0 = PCH_GPI_NMI_EN_0,
		.max_pads_per_group = GPIO_MAX_NUM_PER_GROUP,
		.name = "PCH_ADB",
		.acpi_path = "\\_SB.PCI1.GPIO",
		.reset_map = rst_map,
		.num_reset_vals = ARRAY_SIZE(rst_map),
		.groups = arl_pch_community1_groups,
		.num_groups = ARRAY_SIZE(arl_pch_community1_groups),
	},
	[COMM_PCH_2] = { /* chipset port 0x6c: DSW */
		.port = PID_PCH_GPIOCOM2,
		.cpu_port = PID_PCH_GPIOCOM2,
		.first_pad = GPIO_PCH_COM2_START,
		.last_pad = GPIO_PCH_COM2_END,
		.num_gpi_regs = NUM_GPIO_PCH_COM2_GPI_REGS,
		.pad_cfg_base = PAD_CFG_BASE,
		.pad_cfg_lock_offset = PCH_PAD_CFG_LOCK_OFFSET,
		.host_own_reg_0 = PCH_HOSTSW_OWN_REG_0,
		.gpi_int_sts_reg_0 = PCH_GPI_INT_STS_0,
		.gpi_int_en_reg_0 = PCH_GPI_INT_EN_0,
		.gpi_gpe_sts_reg_0 = PCH_GPI_GPE_STS_0,
		.gpi_gpe_en_reg_0 = PCH_GPI_GPE_EN_0,
		.gpi_smi_sts_reg_0 = PCH_GPI_SMI_STS_0,
		.gpi_smi_en_reg_0 = PCH_GPI_SMI_EN_0,
		.gpi_nmi_sts_reg_0 = PCH_GPI_NMI_STS_0,
		.gpi_nmi_en_reg_0 = PCH_GPI_NMI_EN_0,
		.max_pads_per_group = GPIO_MAX_NUM_PER_GROUP,
		.name = "PCH_DSW",
		.acpi_path = "\\_SB.PCI1.GPIO",
		.reset_map = rst_map,
		.num_reset_vals = ARRAY_SIZE(rst_map),
		.groups = arl_pch_community2_groups,
		.num_groups = ARRAY_SIZE(arl_pch_community2_groups),
	},
	[COMM_PCH_3] = { /* chipset port 0x6b: SPI0, GPP_C, GPP_H, vGPIO_3, vGPIO_0, vGPIO_4 */
		.port = PID_PCH_GPIOCOM3,
		.cpu_port = PID_PCH_GPIOCOM3,
		.first_pad = GPIO_PCH_COM3_START,
		.last_pad = GPIO_PCH_COM3_END,
		.num_gpi_regs = NUM_GPIO_PCH_COM3_GPI_REGS,
		.pad_cfg_base = PAD_CFG_BASE,
		.pad_cfg_lock_offset = PCH_PAD_CFG_LOCK_OFFSET,
		.host_own_reg_0 = PCH_HOSTSW_OWN_REG_0,
		.gpi_int_sts_reg_0 = PCH_GPI_INT_STS_0,
		.gpi_int_en_reg_0 = PCH_GPI_INT_EN_0,
		.gpi_gpe_sts_reg_0 = PCH_GPI_GPE_STS_0,
		.gpi_gpe_en_reg_0 = PCH_GPI_GPE_EN_0,
		.gpi_smi_sts_reg_0 = PCH_GPI_SMI_STS_0,
		.gpi_smi_en_reg_0 = PCH_GPI_SMI_EN_0,
		.gpi_nmi_sts_reg_0 = PCH_GPI_NMI_STS_0,
		.gpi_nmi_en_reg_0 = PCH_GPI_NMI_EN_0,
		.max_pads_per_group = GPIO_MAX_NUM_PER_GROUP,
		.name = "PCH_SCH",
		.acpi_path = "\\_SB.PCI1.GPIO",
		.reset_map = rst_map,
		.num_reset_vals = ARRAY_SIZE(rst_map),
		.groups = arl_pch_community3_groups,
		.num_groups = ARRAY_SIZE(arl_pch_community3_groups),
	},
	[COMM_PCH_4] = { /* chipset port 0x6a: GPP_S, GPP_E, GPP_K, GPP_F */
		.port = PID_PCH_GPIOCOM4,
		.cpu_port = PID_PCH_GPIOCOM4,
		.first_pad = GPIO_PCH_COM4_START,
		.last_pad = GPIO_PCH_COM4_END,
		.num_gpi_regs = NUM_GPIO_PCH_COM4_GPI_REGS,
		.pad_cfg_base = PAD_CFG_BASE,
		.pad_cfg_lock_offset = PCH_PAD_CFG_LOCK_OFFSET,
		.host_own_reg_0 = PCH_HOSTSW_OWN_REG_0,
		.gpi_int_sts_reg_0 = PCH_GPI_INT_STS_0,
		.gpi_int_en_reg_0 = PCH_GPI_INT_EN_0,
		.gpi_gpe_sts_reg_0 = PCH_GPI_GPE_STS_0,
		.gpi_gpe_en_reg_0 = PCH_GPI_GPE_EN_0,
		.gpi_smi_sts_reg_0 = PCH_GPI_SMI_STS_0,
		.gpi_smi_en_reg_0 = PCH_GPI_SMI_EN_0,
		.gpi_nmi_sts_reg_0 = PCH_GPI_NMI_STS_0,
		.gpi_nmi_en_reg_0 = PCH_GPI_NMI_EN_0,
		.max_pads_per_group = GPIO_MAX_NUM_PER_GROUP,
		.name = "PCH_SEKF",
		.acpi_path = "\\_SB.PCI1.GPIO",
		.reset_map = rst_map,
		.num_reset_vals = ARRAY_SIZE(rst_map),
		.groups = arl_pch_community4_groups,
		.num_groups = ARRAY_SIZE(arl_pch_community4_groups),
	},
	[COMM_PCH_5] = { /* chipset port 0x69: GPP_I, JTAG_CPU */
		.port = PID_PCH_GPIOCOM5,
		.cpu_port = PID_PCH_GPIOCOM5,
		.first_pad = GPIO_PCH_COM5_START,
		.last_pad = GPIO_PCH_COM5_END,
		.num_gpi_regs = NUM_GPIO_PCH_COM5_GPI_REGS,
		.pad_cfg_base = PAD_CFG_BASE,
		.pad_cfg_lock_offset = PCH_PAD_CFG_LOCK_OFFSET,
		.host_own_reg_0 = PCH_HOSTSW_OWN_REG_0,
		.gpi_int_sts_reg_0 = PCH_GPI_INT_STS_0,
		.gpi_int_en_reg_0 = PCH_GPI_INT_EN_0,
		.gpi_gpe_sts_reg_0 = PCH_GPI_GPE_STS_0,
		.gpi_gpe_en_reg_0 = PCH_GPI_GPE_EN_0,
		.gpi_smi_sts_reg_0 = PCH_GPI_SMI_STS_0,
		.gpi_smi_en_reg_0 = PCH_GPI_SMI_EN_0,
		.gpi_nmi_sts_reg_0 = PCH_GPI_NMI_STS_0,
		.gpi_nmi_en_reg_0 = PCH_GPI_NMI_EN_0,
		.max_pads_per_group = GPIO_MAX_NUM_PER_GROUP,
		.name = "PCH_IJTAG",
		.acpi_path = "\\_SB.PCI1.GPIO",
		.reset_map = rst_map,
		.num_reset_vals = ARRAY_SIZE(rst_map),
		.groups = arl_pch_community5_groups,
		.num_groups = ARRAY_SIZE(arl_pch_community5_groups),
	}
};

const struct pad_community *soc_gpio_get_community(size_t *num_communities)
{
	*num_communities = ARRAY_SIZE(arl_communities);
	return arl_communities;
}

/*
 * The PMC group numbers are the ones in the vendor controller module's group records. They cover
 * the SoC's groups only: the chipset die numbers its groups separately, so it is skipped below.
 */
const struct pmc_to_gpio_route *soc_pmc_gpio_routes(size_t *num)
{
	static const struct pmc_to_gpio_route routes[] = {
		{ PMC_GPP_A, GPP_A },
		{ PMC_GPP_C, GPP_C },
		{ PMC_GPP_B, GPP_B },
		{ PMC_GPP_VGPIO_USB, GPP_VGPIO_USB },
		{ PMC_GPP_VGPIO_ISCLK, GPP_VGPIO_ISCLK },
		{ PMC_GPP_VGPIO_LPC, GPP_VGPIO_LPC },
		{ PMC_GPP_D, GPP_D },
		{ PMC_GPP_JTAG, GPP_JTAG },
	};
	*num = ARRAY_SIZE(routes);
	return routes;
}

bool soc_gpio_community_has_pmc_gpe(const struct pad_community *comm)
{
	return comm->port < PID_PCH_GPIOCOM0 || comm->port > PID_PCH_GPIOCOM5;
}

__weak const struct gpio_lock_config *mainboard_gpio_lock_config(size_t *num)
{
	*num = 0;
	return NULL;
}

const struct gpio_lock_config *soc_gpio_lock_config(size_t *num)
{
	return mainboard_gpio_lock_config(num);
}
