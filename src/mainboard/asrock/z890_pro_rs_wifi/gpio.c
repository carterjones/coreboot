/* SPDX-License-Identifier: GPL-2.0-only */

#include <gpio.h>

#include "gpio.h"

/*
 * Pad entries built the way the vendor's records are. They differ from PAD_CFG_GPI and
 * PAD_CFG_GPO in leaving the I/O standby state at zero, as the vendor does, in switching the
 * interrupt trigger off explicitly, and in taking the host ownership from the record.
 */
#define VENDOR_GPI(pad, pull, rst, own)						\
	_PAD_CFG_STRUCT(pad,							\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_TRIG(OFF) |		\
		PAD_BUF(TX_DISABLE),						\
		PAD_PULL(pull) | PAD_CFG_OWN_GPIO(own))

#define VENDOR_GPI_SCI(pad, pull, rst, trig, inv, own)				\
	_PAD_CFG_STRUCT(pad,							\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_BUF(TX_DISABLE) |		\
		PAD_IRQ_CFG(SCI, trig, inv),					\
		PAD_PULL(pull) | PAD_CFG_OWN_GPIO(own))

#define VENDOR_GPO(pad, val, rst, pull, own)					\
	_PAD_CFG_STRUCT(pad,							\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_TRIG(OFF) |		\
		PAD_BUF(RX_DISABLE) | !!(val),					\
		PAD_PULL(pull) | PAD_CFG_OWN_GPIO(own))

/*
 * The SoC pads that the vendor firmware sets and the Intel FSP does not: GPP_A15 and A16 from the
 * decoded records and GPP_A17, which the vendor blob names without a configuration, from the
 * pad's registers on the running board. Every other SoC pad is at its reset value or is
 * configured by the FSP for the IP block that uses it (GPP_B8, B9, B14, D13 and D14).
 */
const struct pad_config early_gpio_table[] = {
	VENDOR_GPI(GPP_A15, NONE, PLTRST, DRIVER),
	VENDOR_GPI(GPP_A16, NONE, PLTRST, DRIVER),
	VENDOR_GPO(GPP_A17, 0, PLTRST, NONE, ACPI),
};

const size_t early_gpio_table_size = ARRAY_SIZE(early_gpio_table);

/*
 * The chipset pads that the vendor's configuration table sets to a GPIO function, from the
 * decoded records. The pads that the table lists with a native function (I2C, UART, SPI and
 * others) are configured by the FSP for the IP block that uses them and are not repeated here.
 * Pads whose record only names a reset class keep their hardware state, as do the pads the
 * table does not list.
 *
 * The vendor's lock fields are not applied here; the lock of the TPM reset pad is in
 * smihandler.c.
 */
const struct pad_config pch_gpio_table[] = {
	VENDOR_GPI_SCI(PCH_GPP_F17, UP_20K, PLTRST, EDGE_SINGLE, INVERT, ACPI),	/* input, inverted, edge, SCI, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_F15, NONE, PLTRST, ACPI),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_H05, NONE, PLTRST, ACPI),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_H06, NONE, PLTRST, ACPI),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_H07, NONE, PLTRST, ACPI),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_H08, NONE, PLTRST, ACPI),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_C16, NONE, PLTRST, ACPI),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_C17, NONE, PLTRST, ACPI),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_C18, NONE, PLTRST, ACPI),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_C19, NONE, PLTRST, ACPI),	/* input, reset on PLTRST */
	VENDOR_GPO(PCH_GPP_E02, 1, PLTRST, NONE, DRIVER),	/* output high, reset on PLTRST */
	VENDOR_GPO(PCH_GPP_F22, 1, PLTRST, NONE, ACPI),	/* output high, reset on PLTRST */
	VENDOR_GPO(PCH_GPP_F23, 1, PLTRST, NONE, ACPI),	/* output high, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_E00, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_E01, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_E04, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_E05, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_E06, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_E18, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_E19, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_E20, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_E21, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_A13, UP_20K, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_B01, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_B02, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_B04, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_B05, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_B06, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_B07, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_B08, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_B09, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_B10, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_B15, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_B16, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_B17, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_C08, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_C09, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_C12, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_C13, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_C14, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_C15, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_C20, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_C21, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_C22, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_J11, NONE, PWROK, DRIVER),	/* input, reset on PWROK */
	VENDOR_GPI(PCH_GPP_R06, NONE, PWROK, DRIVER),	/* input, reset on PWROK */
	VENDOR_GPI(PCH_GPP_B11, NONE, PLTRST, DRIVER),	/* input, reset on PLTRST */
	VENDOR_GPO(PCH_GPP_F14, 0, PLTRST, NONE, DRIVER),	/* output low, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_F18, NONE, PLTRST, ACPI),	/* input, reset on PLTRST */
	VENDOR_GPO(PCH_GPP_J15, 1, PLTRST, NONE, ACPI),	/* output high, reset on PLTRST */
	VENDOR_GPO(PCH_GPP_F16, 1, PLTRST, NONE, ACPI),	/* output high, reset on PLTRST */
	VENDOR_GPI(PCH_GPP_E13, NONE, PWROK, DRIVER),	/* input, reset on PWROK */
	VENDOR_GPI(PCH_GPP_F02, NONE, DEEP, DRIVER),	/* input, reset on DEEP */
	VENDOR_GPI(PCH_GPP_F08, NONE, DEEP, DRIVER),	/* input, reset on DEEP */
	VENDOR_GPI(PCH_GPP_E17, NONE, PWROK, DRIVER),	/* input, reset on PWROK */
	VENDOR_GPI(PCH_GPP_K00, NONE, DEEP, DRIVER),	/* input, reset on DEEP */
};

const size_t pch_gpio_table_size = ARRAY_SIZE(pch_gpio_table);
