/* SPDX-License-Identifier: GPL-2.0-or-later */

/*
 * Root bridge of the chipset, bus 0x80-0xdf in the same segment as the SoC's PCI0. The windows
 * are the ones in soc/iomap.h. The 64-bit window comes from A4GB and A4GS, which chip.c writes
 * into this scope.
 */

#include <commonlib/bsd/helpers.h>
#include <soc/iomap.h>

Name (_HID, EisaId ("PNP0A08") /* PCI Express Bus */)
Name (_CID, EisaId ("PNP0A03") /* PCI Bus */)
Name (_SEG, 0)  // _SEG: PCI Segment
Name (_UID, 1)
Name (_BBN, 0x80)

External (A4GS, IntObj)
External (A4GB, IntObj)

Method (_CRS, 0, Serialized)
{
	Name (PCRS, ResourceTemplate ()
	{
		/* Bus Numbers */
		WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode,
				0x0000, 0x0080, 0x00df, 0x0000, 0x0060)

		/* IO window, 0x2000-0x8bff */
		DWordIO (ResourceProducer, MinFixed, MaxFixed, PosDecode,
				EntireRange,
				0x0000, PCH_DOMAIN_IO_BASE, PCH_DOMAIN_IO_LIMIT, 0x0000,
				0x6c00)

		/* 32-bit memory window, 0xb8000000-0xbdffffff */
		DWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed,
				NonCacheable, ReadWrite,
				0x00000000, PCH_DOMAIN_MEM32_BASE, PCH_DOMAIN_MEM32_LIMIT,
				0x00000000,
				0x06000000)

		/* 64-bit memory window */
		QWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed,
				NonCacheable, ReadWrite,
				0x00000000, 0x10000, 0x1ffff, 0x00000000,
				0x10000,,, PM02)
	})

	/* Patch the 64-bit window */
	If (A4GS == 0) {
		CreateQwordField (PCRS, PM02._LEN, MSEN)
		MSEN = 0
	} Else {
		CreateQwordField (PCRS, PM02._MIN, MMIN)
		CreateQwordField (PCRS, PM02._MAX, MMAX)
		CreateQwordField (PCRS, PM02._LEN, MLEN)
		MLEN = A4GS
		MMIN = A4GB
		MMAX = MMIN + MLEN - 1
	}

	Return (PCRS)
}

/*
 * Interrupt routing. The vendor firmware leaves the interrupt line registers at 0xff and its
 * ACPI maps the standard PIRQ A to D to GSI 16 to 19 (the SMBus, interrupt pin C, uses GSI 18),
 * so every chipset device slot gets that map.
 */
Method (_PRT, 0, NotSerialized)
{
	If (PICM) {
		Return (PRTA)
	}
	Return (PRTP)
}

#define PRT_SLOT(adr) \
	Package () { adr, 0, 0, 16 }, \
	Package () { adr, 1, 0, 17 }, \
	Package () { adr, 2, 0, 18 }, \
	Package () { adr, 3, 0, 19 }

#define PRT_SLOT_PIC(adr) \
	Package () { adr, 0, 0, 11 }, \
	Package () { adr, 1, 0, 10 }, \
	Package () { adr, 2, 0, 11 }, \
	Package () { adr, 3, 0, 11 }

Name (PRTA, Package () {
	PRT_SLOT (0x0014ffff),
	PRT_SLOT (0x0015ffff),
	PRT_SLOT (0x0016ffff),
	PRT_SLOT (0x0017ffff),
	PRT_SLOT (0x0019ffff),
	PRT_SLOT (0x001cffff),
	PRT_SLOT (0x001effff),
	PRT_SLOT (0x001fffff)
})

Name (PRTP, Package () {
	PRT_SLOT_PIC (0x0014ffff),
	PRT_SLOT_PIC (0x0015ffff),
	PRT_SLOT_PIC (0x0016ffff),
	PRT_SLOT_PIC (0x0017ffff),
	PRT_SLOT_PIC (0x0019ffff),
	PRT_SLOT_PIC (0x001cffff),
	PRT_SLOT_PIC (0x001effff),
	PRT_SLOT_PIC (0x001fffff)
})

/* PCI _OSC */
Method (_OSC, 4) {
	/* Check for proper GUID */
	If (Arg0 == ToUUID ("33DB4D5B-1FF7-401C-9657-7441C03DD766")) {
		/* Let OS control everything */
		Return (Arg3)
	} Else {
		/* Unrecognized UUID */
		CreateDWordField (Arg3, 0, CDW1)
		CDW1 |= 4
		Return (Arg3)
	}
}
