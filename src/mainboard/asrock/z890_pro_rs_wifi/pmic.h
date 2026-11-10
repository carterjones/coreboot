/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef MAINBOARD_PMIC_H
#define MAINBOARD_PMIC_H

/* Print the DDR5 PMIC registers of each DIMM, labelled with when. Romstage only. */
void z890_dump_pmics(const char *when);

#endif
