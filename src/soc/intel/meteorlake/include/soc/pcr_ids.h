/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SOC_METEORLAKE_PCR_H
#define SOC_METEORLAKE_PCR_H

/*
 * Port ids
 */

#define PID_GPIOCOM0	0xD1
#define PID_GPIOCOM1	0xD2
#define PID_GPIOCOM3	0xD3
#define PID_GPIOCOM4	0xD4
#define PID_GPIOCOM5	0xD5

#if CONFIG(SOC_INTEL_ARROWLAKE_S)
/*
 * The chipset's GPIO communities are on sideband ports 0x6e, 0x6d, 0x6c, 0x6b, 0x6a and 0x69 of
 * the chipset's own P2SB, not of the SoC's. The SoC's PSTH and RTC use two of the same numbers
 * (PID_PSTH, PID_RTC below), so a community names a virtual port id and soc_pcr_resolve_pid()
 * turns it into the chipset P2SB and the real port.
 */
#define PID_PCH_GPIOCOM0	0xF0	/* port 0x6e */
#define PID_PCH_GPIOCOM1	0xF1	/* port 0x6d */
#define PID_PCH_GPIOCOM2	0xF2	/* port 0x6c */
#define PID_PCH_GPIOCOM3	0xF3	/* port 0x6b */
#define PID_PCH_GPIOCOM4	0xF4	/* port 0x6a */
#define PID_PCH_GPIOCOM5	0xF5	/* port 0x69 */
#endif

#define PID_PSF8	0x08
#define PID_PSF7	0x07
#define PID_PSF6	0x06
#define PID_CNVI	0x29
#define PID_PSF5	0x8F
#define PID_PSF4	0xBD
#define PID_PSF3	0xBC
#define PID_PSF2	0xBB
#define PID_PSF1	0xBA
#define PID_CSME0	0xC6
#define PID_PSTH	0x6A
#define PID_ITSS	0xCA
#define PID_RTC		0x6C
#define PID_ISCLK	0x64
#define PID_DMI		0x88
#define PID_IOM		0xAA
#define PID_XHCI	0x3A

#endif
