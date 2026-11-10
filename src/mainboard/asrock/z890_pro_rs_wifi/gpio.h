/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef MAINBOARD_GPIO_H
#define MAINBOARD_GPIO_H

#include <gpio.h>
#include <stddef.h>

extern const struct pad_config early_gpio_table[];
extern const size_t early_gpio_table_size;
extern const struct pad_config pch_gpio_table[];
extern const size_t pch_gpio_table_size;

#endif /* MAINBOARD_GPIO_H */
