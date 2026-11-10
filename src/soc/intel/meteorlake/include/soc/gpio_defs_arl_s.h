/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _SOC_METEORLAKE_GPIO_DEFS_ARL_S_H_
#define _SOC_METEORLAKE_GPIO_DEFS_ARL_S_H_

#include <soc/gpio_soc_defs_arl_s.h>

#define GPIO_NUM_PAD_CFG_REGS	4 /* DW0, DW1, DW2, DW3 */

/* The status and enable registers are laid out per pad group, one register for each. */
#define NUM_GPIO_COM0_GPI_REGS	3 /* GPP_A, GPP_VGPIO_LPC, GPP_C */
#define NUM_GPIO_COM1_GPI_REGS	3 /* GPP_B, GPP_VGPIO_USB, GPP_D */
#define NUM_GPIO_COM3_GPI_REGS	2 /* GPP_JTAG, GPP_VGPIO_ISCLK */

/* The chipset's communities, in the order of the ports 0x6e, 0x6d, 0x6c, 0x6b, 0x6a and 0x69. */
#define NUM_GPIO_PCH_COM0_GPI_REGS	4 /* GPP_D, GPP_R, GPP_J, vGPIO */
#define NUM_GPIO_PCH_COM1_GPI_REGS	3 /* GPP_A, DIR_ESPI, GPP_B */
#define NUM_GPIO_PCH_COM2_GPI_REGS	1 /* DSW */
#define NUM_GPIO_PCH_COM3_GPI_REGS	6 /* SPI0, GPP_C, GPP_H, vGPIO_3, vGPIO_0, vGPIO_4 */
#define NUM_GPIO_PCH_COM4_GPI_REGS	4 /* GPP_S, GPP_E, GPP_K, GPP_F */
#define NUM_GPIO_PCH_COM5_GPI_REGS	2 /* GPP_I, JTAG_CPU */

#define NUM_GPI_STATUS_REGS	\
		((NUM_GPIO_COM0_GPI_REGS) +\
		(NUM_GPIO_COM1_GPI_REGS) +\
		(NUM_GPIO_COM3_GPI_REGS) +\
		(NUM_GPIO_PCH_COM0_GPI_REGS) +\
		(NUM_GPIO_PCH_COM1_GPI_REGS) +\
		(NUM_GPIO_PCH_COM2_GPI_REGS) +\
		(NUM_GPIO_PCH_COM3_GPI_REGS) +\
		(NUM_GPIO_PCH_COM4_GPI_REGS) +\
		(NUM_GPIO_PCH_COM5_GPI_REGS))

/* Register defines. */
#define GPIO_MISCCFG				0x10
#define  GPE_DW_SHIFT				8
#define  GPE_DW_MASK				0xfff00
#define PAD_CFG_LOCK_OFFSET			0x0f0
#define HOSTSW_OWN_REG_0			0x110
#define GPI_INT_STS_0				0x200
#define GPI_INT_EN_0				0x210
#define GPI_GPE_STS_0				0x220
#define GPI_GPE_EN_0				0x230
#define GPI_SMI_STS_0				0x240
#define GPI_SMI_EN_0				0x250
#define GPI_NMI_STS_0				0x260
#define GPI_NMI_EN_0				0x270
#define PAD_CFG_BASE				0x600

/* The chipset's community registers sit at other offsets, again one register per group. */
#define PCH_PAD_CFG_LOCK_OFFSET			0x110
#define PCH_HOSTSW_OWN_REG_0			0x150
#define PCH_GPI_INT_STS_0			0x200
#define PCH_GPI_INT_EN_0			0x220
#define PCH_GPI_GPE_STS_0			0x240
#define PCH_GPI_GPE_EN_0			0x260
#define PCH_GPI_SMI_STS_0			0x280
#define PCH_GPI_SMI_EN_0			0x2a0
#define PCH_GPI_NMI_STS_0			0x2c0
#define PCH_GPI_NMI_EN_0			0x2e0

#endif
