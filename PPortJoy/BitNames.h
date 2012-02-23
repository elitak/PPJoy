/***************************************************************************
 *   PPJoy Virtual Joystick for Microsoft Windows                          *
 *   Copyright (C) 2011 Deon van der Westhuysen                            *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 ***************************************************************************/


#ifndef __BITNAMES_H__
#define __BITNAMES_H__

/* Define macros to return logic values on parallel port pins (we undo hardware inverted lines) */

/* Data port */
#define	D0_MASK				0x01
#define	D1_MASK				0x02
#define	D2_MASK				0x04
#define	D3_MASK				0x08
#define	D4_MASK				0x10
#define	D5_MASK				0x20
#define	D6_MASK				0x40
#define	D7_MASK				0x80

/* Status port */
#define	S3_MASK				0x08
#define	S4_MASK				0x10
#define	S5_MASK				0x20
#define	S6_MASK				0x40
#define	S7_MASK				0x80

/* Control port */
#define	C0_MASK				0x01
#define	C1_MASK				0x02
#define	C2_MASK				0x04
#define	C3_MASK				0x08

#define	STATUS_INVMASK		0x80
#define	CONTROL_INVMASK		0x0B

/* Data port */
#define	D0_LINE(X)			( ((X)&D0_MASK) )	/* Data 0 */			/* pin 2 */
#define	D1_LINE(X)			( ((X)&D1_MASK) )	/* Data 1 */			/* pin 3 */
#define	D2_LINE(X)			( ((X)&D2_MASK) )	/* Data 2 */			/* pin 4 */
#define	D3_LINE(X)			( ((X)&D3_MASK) )	/* Data 3 */			/* pin 5 */
#define	D4_LINE(X)			( ((X)&D4_MASK) )	/* Data 4 */			/* pin 6 */
#define	D5_LINE(X)			( ((X)&D5_MASK) )	/* Data 5 */			/* pin 7 */
#define	D6_LINE(X)			( ((X)&D6_MASK) )	/* Data 6 */			/* pin 8 */
#define	D7_LINE(X)			( ((X)&D7_MASK) )	/* Data 7 */			/* pin 9 */

/* Status port */
#define	S3_LINE(X)			( ((X)&S3_MASK) )	/* Error */				/* pin 15 */
#define	S4_LINE(X)			( ((X)&S4_MASK) )	/* SelectIn */			/* pin 13 */
#define	S5_LINE(X)			( ((X)&S5_MASK) )	/* PaperOut */			/* pin 12 */
#define	S6_LINE(X)			( ((X)&S6_MASK) )	/* Ack */				/* pin 10 */
#define	S7_LINE(X)			( (~(X)&S7_MASK) )	/* Busy */				/* pin 11 */	/* INVERTED */

/* Control port */
#define	C0_LINE(X)			( (~(X)&C0_MASK) )	/* Strobe */			/* pin 1 */		/* INVERTED */
#define	C1_LINE(X)			( (~(X)&C1_MASK) )	/* Autofeed */			/* pin 14 */	/* INVERTED */
#define	C2_LINE(X)			( ((X)&C2_MASK) )	/* Initialize */		/* pin 16 */
#define	C3_LINE(X)			( (~(X)&C3_MASK) )	/* Select (Out) */		/* pin 17 */	/* INVERTED */


/* Now define it as the pin numbers */
#define	PIN1(X)				C0_LINE(X)
#define	PIN2(X)				D0_LINE(X)
#define	PIN3(X)				D1_LINE(X)
#define	PIN4(X)				D2_LINE(X)
#define	PIN5(X)				D3_LINE(X)
#define	PIN6(X)				D4_LINE(X)
#define	PIN7(X)				D5_LINE(X)
#define	PIN8(X)				D6_LINE(X)
#define	PIN9(X)				D7_LINE(X)
#define	PIN10(X)			S6_LINE(X)
#define	PIN11(X)			S7_LINE(X)
#define	PIN12(X)			S5_LINE(X)
#define	PIN13(X)			S4_LINE(X)
#define	PIN14(X)			C1_LINE(X)
#define	PIN15(X)			S3_LINE(X)
#define	PIN16(X)			C2_LINE(X)
#define	PIN17(X)			C3_LINE(X)

#define	PIN1_MASK			C0_MASK
#define	PIN2_MASK			D0_MASK
#define	PIN3_MASK			D1_MASK
#define	PIN4_MASK			D2_MASK
#define	PIN5_MASK			D3_MASK
#define	PIN6_MASK			D4_MASK
#define	PIN7_MASK			D5_MASK
#define	PIN8_MASK			D6_MASK
#define	PIN9_MASK			D7_MASK
#define	PIN10_MASK			S6_MASK
#define	PIN11_MASK			S7_MASK
#define	PIN12_MASK			S5_MASK
#define	PIN13_MASK			S4_MASK
#define	PIN14_MASK			C1_MASK
#define	PIN15_MASK			S3_MASK
#define	PIN16_MASK			C2_MASK
#define	PIN17_MASK			C3_MASK

/* Constants for DB9.c joystick and Sega Genesis (Linux) interface */
#define	LINDB9_UP(X)		PIN2(X)
#define	LINDB9_DOWN(X)		PIN3(X)
#define	LINDB9_LEFT(X)		PIN4(X)
#define	LINDB9_RIGHT(X)		PIN5(X)
#define	LINDB9_FIRE1(X)		PIN6(X)
#define	LINDB9_FIRE2(X)		PIN7(X)
#define	LINDB9_FIRE3(X)		PIN8(X)
#define	LINDB9_FIRE4(X)		PIN9(X)

#define	LINDB9_FIRE5(X)		PIN10(X)
#define	LINDB9_FIRE6(X)		PIN11(X)
#define	LINDB9_FIRE7(X)		PIN12(X)
#define	LINDB9_FIRE8(X)		PIN13(X)
#define	LINDB9_FIRE9(X)		PIN15(X)

/* Constants for DirectPad Pro Sega Genesis interface */
#define	DPPGEN_UP(X)		PIN1(X)
#define	DPPGEN_DOWN(X)		PIN14(X)
#define	DPPGEN_LEFT(X)		PIN10(X)
#define	DPPGEN_RIGHT(X)		PIN11(X)
#define	DPPGEN_FIRE1(X)		PIN12(X)
#define	DPPGEN_FIRE2(X)		PIN13(X)

/* Constants for NTPad XP Genesis interface */
#define	NTPGEN1_UP(X)		PIN2(X)
#define	NTPGEN1_DOWN(X)		PIN3(X)
#define	NTPGEN1_LEFT(X)		PIN4(X)
#define	NTPGEN1_RIGHT(X)	PIN5(X)
#define	NTPGEN1_FIRE1(X)	PIN6(X)
#define	NTPGEN1_FIRE2(X)	PIN7(X)

#define	NTPGEN2_UP(X)		PIN8(X)
#define	NTPGEN2_DOWN(X)		PIN9(X)
#define	NTPGEN2_LEFT(X)		PIN10(X)
#define	NTPGEN2_RIGHT(X)	PIN11(X)
#define	NTPGEN2_FIRE1(X)	PIN12(X)
#define	NTPGEN2_FIRE2(X)	PIN13(X)

/* Constants for Console Cable Genesis interface */
#define	CCGEN1_UP(X)		PIN11(X)
#define	CCGEN1_DOWN(X)		PIN10(X)
#define	CCGEN1_LEFT(X)		PIN12(X)
#define	CCGEN1_RIGHT(X)		PIN13(X)
#define	CCGEN1_FIRE1(X)		PIN15(X)
#define	CCGEN1_FIRE2(X)		PIN2(X)

#define	CCGEN2_UP(X)		PIN9(X)
#define	CCGEN2_DOWN(X)		PIN8(X)
#define	CCGEN2_LEFT(X)		PIN7(X)
#define	CCGEN2_RIGHT(X)		PIN6(X)
#define	CCGEN2_FIRE1(X)		PIN5(X)
#define	CCGEN2_FIRE2(X)		PIN4(X)

/* Constants for SNESkey Genesis interface */
#define	SNESGEN_UP(X)		PIN11(X)
#define	SNESGEN_DOWN(X)		PIN10(X)
#define	SNESGEN_LEFT(X)		PIN12(X)
#define	SNESGEN_RIGHT(X)	PIN13(X)
#define	SNESGEN_FIRE1(X)	PIN15(X)
#define	SNESGEN_FIRE2(X)	PIN1(X)

/* Constants for IanHarries interface. This is a "weird" interface: */
/* 1 on an input pin indicates that the switch is pressed.          */
#define	IANHARRIES_UP(X)	PIN13(~(X))
#define	IANHARRIES_DOWN(X)	PIN12(~(X))
#define	IANHARRIES_LEFT(X)	PIN11(~(X))
#define	IANHARRIES_RIGHT(X)	PIN10(~(X))
#define	IANHARRIES_FIRE1(X)	PIN15(~(X))

/* Constants for TurboGraFX interface */
#define	TURBOGFX_UP(X)		PIN13(X)
#define	TURBOGFX_DOWN(X)	PIN12(X)
#define	TURBOGFX_LEFT(X)	PIN10(X)
#define	TURBOGFX_RIGHT(X)	PIN11(X)
#define	TURBOGFX_FIRE1(X)	PIN15(X)
#define	TURBOGFX_FIRE2(X)	PIN14(X)
#define	TURBOGFX_FIRE3(X)	PIN16(X)
#define	TURBOGFX_FIRE4(X)	PIN17(X)
#define	TURBOGFX_FIRE5(X)	PIN1(X)

/* Constants for TheMaze interface */
#define	THEMAZE_UP(X)		( PIN13(X) | (!PIN12(X)) )	/* 0 is "true", so "or" is actually "and" */
#define	THEMAZE_DOWN(X)		( PIN12(X) | (!PIN13(X)) )
#define	THEMAZE_LEFT(X)		( PIN11(X) | (!PIN10(X)) )
#define	THEMAZE_RIGHT(X)	( PIN10(X) | (!PIN11(X)) )
#define	THEMAZE_FIRE1(X)	PIN15(X)
#define	THEMAZE_FIRE2(X)	(PIN10(X) | PIN11(X))
#define	THEMAZE_FIRE3(X)	(PIN12(X) | PIN13(X))

/* Constants for Linux v0.8.0.2 interface */
#define	LIN0802_1_UP(X)		PIN15(X)
#define	LIN0802_1_DOWN(X)	PIN13(X)
#define	LIN0802_1_LEFT(X)	PIN12(X)
#define	LIN0802_1_RIGHT(X)	PIN10(X)
#define	LIN0802_1_FIRE1(X)	PIN11(X)

#define	LIN0802_2_UP(X)		PIN5(X)
#define	LIN0802_2_DOWN(X)	PIN6(X)
#define	LIN0802_2_LEFT(X)	PIN7(X)
#define	LIN0802_2_RIGHT(X)	PIN8(X)
#define	LIN0802_2_FIRE1(X)	PIN9(X)

/* Constants for Tormod interface */
#define	TORMOD_UP(X)		PIN10(X)
#define	TORMOD_DOWN(X)		PIN12(X)
#define	TORMOD_LEFT(X)		PIN13(X)
#define	TORMOD_RIGHT(X)		PIN15(X)
#define	TORMOD_FIRE1(X)		PIN11(X)
#define	TORMOD_FIRE2(X)		PIN14(X)
#define	TORMOD_FIRE3(X)		PIN16(X)
#define	TORMOD_FIRE4(X)		PIN17(X)

/* Constants for DirectPad Pro joystick interface */
#define	DPPJOY_UP(X)		PIN1(X)
#define	DPPJOY_DOWN(X)		PIN14(X)
#define	DPPJOY_LEFT(X)		PIN16(X)
#define	DPPJOY_RIGHT(X)		PIN17(X)
#define	DPPJOY_FIRE1(X)		PIN11(X)
#define	DPPJOY_FIRE2(X)		PIN12(X)

/* Constants for LPTJoystick interface */ 
#define	LPTJOY_UP(X)		PIN13(X)
#define	LPTJOY_DOWN(X)		PIN12(X)
#define	LPTJOY_LEFT(X)		PIN11(X)
#define	LPTJOY_RIGHT(X)		PIN10(X)
#define	LPTJOY_FIRE1(X)		PIN15(X)

/* Constants for CHAMPgames interface */
#define	CHAMPG_UP(X)		PIN10(X)
#define	CHAMPG_DOWN(X)		PIN11(X)
#define	CHAMPG_LEFT(X)		PIN12(X)
#define	CHAMPG_RIGHT(X)		PIN13(X)
#define	CHAMPG_FIRE1(X)		PIN15(X)

/* Constants for STFormat interface */
#define	STFORMAT1_UP(X)		PIN2(X)
#define	STFORMAT1_DOWN(X)	PIN3(X)
#define	STFORMAT1_LEFT(X)	PIN4(X)
#define	STFORMAT1_RIGHT(X)	PIN5(X)
#define	STFORMAT1_FIRE1(X)	PIN1(X)

#define	STFORMAT2_UP(X)		PIN6(X)
#define	STFORMAT2_DOWN(X)	PIN7(X)
#define	STFORMAT2_LEFT(X)	PIN8(X)
#define	STFORMAT2_RIGHT(X)	PIN9(X)
#define	STFORMAT2_FIRE1(X)	PIN11(X)

/* Constants for SNESkey2600 joystick interface */
#define	SNES2600_UP(X)		PIN11(X)
#define	SNES2600_DOWN(X)	PIN10(X)
#define	SNES2600_LEFT(X)	PIN12(X)
#define	SNES2600_RIGHT(X)	PIN13(X)
#define	SNES2600_FIRE1(X)	PIN15(X)

/* Constants for Amiga 4 player joystick interface */
#define	AMIGA4_1_UP(X)		PIN2(X)
#define	AMIGA4_1_DOWN(X)	PIN3(X)
#define	AMIGA4_1_LEFT(X)	PIN4(X)
#define	AMIGA4_1_RIGHT(X)	PIN5(X)
#define	AMIGA4_1_FIRE1(X)	PIN13(X)
#define	AMIGA4_1_FIRE2(X)	PIN12(X)

#define	AMIGA4_2_UP(X)		PIN6(X)
#define	AMIGA4_2_DOWN(X)	PIN7(X)
#define	AMIGA4_2_LEFT(X)	PIN8(X)
#define	AMIGA4_2_RIGHT(X)	PIN9(X)
#define	AMIGA4_2_FIRE1(X)	PIN11(X)
#define	AMIGA4_2_FIRE2(X)	PIN10(X)

/* Constants for PCAE joystick interface */
#define	PCAE1_UP(X)			PIN6(X)
#define	PCAE1_DOWN(X)		PIN7(X)
#define	PCAE1_LEFT(X)		PIN8(X)
#define	PCAE1_RIGHT(X)		PIN9(X)
#define	PCAE1_FIRE1(X)		((~X)&0x20)			/* Bit from joystick port... Pin2 / button 0 */

#define	PCAE2_UP(X)			PIN2(X)
#define	PCAE2_DOWN(X)		PIN3(X)
#define	PCAE2_LEFT(X)		PIN4(X)
#define	PCAE2_RIGHT(X)		PIN5(X)
#define	PCAE2_FIRE1(X)		((~X)&0x10)			/* Bit from joystick port... Pin7 / button 1 */

/* Constants for PSX Peripheral Bus library interface. This interface */
/* is a superset of the DirectPad Pro interface. DPP has controllers 1 to 6 */
//#define	PSXLIB_POWER1		(PIN5_MASK|PIN6_MASK|PIN7_MASK|PIN8_MASK|PIN9_MASK)
//#define	PSXLIB_POWER2		0x04	/* Control port, Forward mode, all pins high */

#define	PSXLIB1_DATA		PIN10_MASK
#define	PSXLIB1_CMD			PIN2_MASK
#define	PSXLIB1_SEL			PIN3_MASK
#define	PSXLIB1_CLK			PIN4_MASK
#define	PSXLIB1_ACK			PIN12_MASK
#define	PSXLIB1_BASE		(PIN5_MASK|PIN6_MASK|PIN7_MASK|PIN8_MASK|PIN9_MASK)

#define	PSXLIB2_DATA		PIN13_MASK
#define	PSXLIB2_CMD			PIN2_MASK
#define	PSXLIB2_SEL			PIN3_MASK
#define	PSXLIB2_CLK			PIN4_MASK
#define	PSXLIB2_ACK			PIN15_MASK
#define	PSXLIB2_BASE		(PIN5_MASK|PIN6_MASK|PIN7_MASK|PIN8_MASK|PIN9_MASK)

#define	PSXLIB3_DATA		PIN10_MASK
#define	PSXLIB3_CMD			PIN2_MASK
#define	PSXLIB3_SEL			PIN5_MASK
#define	PSXLIB3_CLK			PIN4_MASK
#define	PSXLIB3_ACK			PIN12_MASK
#define	PSXLIB3_BASE		(PIN6_MASK|PIN7_MASK|PIN8_MASK|PIN9_MASK)

#define	PSXLIB4_DATA		PIN10_MASK
#define	PSXLIB4_CMD			PIN2_MASK
#define	PSXLIB4_SEL			PIN6_MASK
#define	PSXLIB4_CLK			PIN4_MASK
#define	PSXLIB4_ACK			PIN12_MASK
#define	PSXLIB4_BASE		(PIN5_MASK|PIN7_MASK|PIN8_MASK|PIN9_MASK)

#define	PSXLIB5_DATA		PIN10_MASK
#define	PSXLIB5_CMD			PIN2_MASK
#define	PSXLIB5_SEL			PIN7_MASK
#define	PSXLIB5_CLK			PIN4_MASK
#define	PSXLIB5_ACK			PIN12_MASK
#define	PSXLIB5_BASE		(PIN5_MASK|PIN6_MASK|PIN8_MASK|PIN9_MASK)

#define	PSXLIB6_DATA		PIN10_MASK
#define	PSXLIB6_CMD			PIN2_MASK
#define	PSXLIB6_SEL			PIN8_MASK
#define	PSXLIB6_CLK			PIN4_MASK
#define	PSXLIB6_ACK			PIN12_MASK
#define	PSXLIB6_BASE		(PIN5_MASK|PIN6_MASK|PIN7_MASK|PIN9_MASK)

#define	PSXLIB7_DATA		PIN10_MASK
#define	PSXLIB7_CMD			PIN2_MASK
#define	PSXLIB7_SEL			PIN9_MASK
#define	PSXLIB7_CLK			PIN4_MASK
#define	PSXLIB7_ACK			PIN12_MASK
#define	PSXLIB7_BASE		(PIN5_MASK|PIN6_MASK|PIN7_MASK|PIN8_MASK)

#define	PSXLIB8_DATA		PIN10_MASK
#define	PSXLIB8_CMD			PIN14_MASK
#define	PSXLIB8_SEL			PIN16_MASK
#define	PSXLIB8_CLK			PIN1_MASK
#define	PSXLIB8_ACK			PIN13_MASK
#define	PSXLIB8_BASE		0x00

/* Constants for Linux Playstation interface. First unit is compatible with DPP */
#define	PSXLINUX1_DATA		PIN10_MASK
#define	PSXLINUX1_CMD		PIN2_MASK
#define	PSXLINUX1_SEL		PIN3_MASK
#define	PSXLINUX1_CLK		PIN4_MASK
#define	PSXLINUX1_ACK		0x00	/* Linux interface hase NO ACK input */
#define	PSXLINUX1_BASE		(PIN5_MASK|PIN6_MASK|PIN7_MASK|PIN8_MASK|PIN9_MASK)

#define	PSXLINUX2_DATA		PIN11_MASK
#define	PSXLINUX2_CMD		PIN2_MASK
#define	PSXLINUX2_SEL		PIN3_MASK
#define	PSXLINUX2_CLK		PIN4_MASK
#define	PSXLINUX2_ACK		0x00	/* Linux interface hase NO ACK input */
#define	PSXLINUX2_BASE		(PIN5_MASK|PIN6_MASK|PIN7_MASK|PIN8_MASK|PIN9_MASK)

#define	PSXLINUX3_DATA		PIN12_MASK
#define	PSXLINUX3_CMD		PIN2_MASK
#define	PSXLINUX3_SEL		PIN3_MASK
#define	PSXLINUX3_CLK		PIN4_MASK
#define	PSXLINUX3_ACK		0x00	/* Linux interface hase NO ACK input */
#define	PSXLINUX3_BASE		(PIN5_MASK|PIN6_MASK|PIN7_MASK|PIN8_MASK|PIN9_MASK)

#define	PSXLINUX4_DATA		PIN13_MASK
#define	PSXLINUX4_CMD		PIN2_MASK
#define	PSXLINUX4_SEL		PIN3_MASK
#define	PSXLINUX4_CLK		PIN4_MASK
#define	PSXLINUX4_ACK		0x00	/* Linux interface hase NO ACK input */
#define	PSXLINUX4_BASE		(PIN5_MASK|PIN6_MASK|PIN7_MASK|PIN8_MASK|PIN9_MASK)

#define	PSXLINUX5_DATA		PIN15_MASK
#define	PSXLINUX5_CMD		PIN2_MASK
#define	PSXLINUX5_SEL		PIN3_MASK
#define	PSXLINUX5_CLK		PIN4_MASK
#define	PSXLINUX5_ACK		0x00	/* Linux interface hase NO ACK input */
#define	PSXLINUX5_BASE		(PIN5_MASK|PIN6_MASK|PIN7_MASK|PIN8_MASK|PIN9_MASK)

/* Constants for NTPadXP Playstation interface. First unit is compatible with DPP */
#define	PSXNTPAD1_DATA		PIN10_MASK
#define	PSXNTPAD1_CMD		PIN2_MASK
#define	PSXNTPAD1_SEL		PIN3_MASK
#define	PSXNTPAD1_CLK		PIN4_MASK
#define	PSXNTPAD1_ACK		PIN12_MASK
#define	PSXNTPAD1_BASE		(PIN5_MASK|PIN6_MASK|PIN7_MASK|PIN8_MASK|PIN9_MASK)

#define	PSXNTPAD2_DATA		PIN10_MASK
#define	PSXNTPAD2_CMD		PIN2_MASK
#define	PSXNTPAD2_SEL		PIN3_MASK
#define	PSXNTPAD2_CLK		PIN5_MASK
#define	PSXNTPAD2_ACK		PIN12_MASK
#define	PSXNTPAD2_BASE		(PIN6_MASK|PIN7_MASK|PIN8_MASK|PIN9_MASK)

#define	PSXNTPAD3_DATA		PIN10_MASK
#define	PSXNTPAD3_CMD		PIN2_MASK
#define	PSXNTPAD3_SEL		PIN3_MASK
#define	PSXNTPAD3_CLK		PIN6_MASK
#define	PSXNTPAD3_ACK		PIN12_MASK
#define	PSXNTPAD3_BASE		(PIN5_MASK|PIN7_MASK|PIN8_MASK|PIN9_MASK)

#define	PSXNTPAD4_DATA		PIN10_MASK
#define	PSXNTPAD4_CMD		PIN2_MASK
#define	PSXNTPAD4_SEL		PIN3_MASK
#define	PSXNTPAD4_CLK		PIN7_MASK
#define	PSXNTPAD4_ACK		PIN12_MASK
#define	PSXNTPAD4_BASE		(PIN5_MASK|PIN6_MASK|PIN8_MASK|PIN9_MASK)

#define	PSXNTPAD5_DATA		PIN10_MASK
#define	PSXNTPAD5_CMD		PIN2_MASK
#define	PSXNTPAD5_SEL		PIN3_MASK
#define	PSXNTPAD5_CLK		PIN8_MASK
#define	PSXNTPAD5_ACK		PIN12_MASK
#define	PSXNTPAD5_BASE		(PIN5_MASK|PIN6_MASK|PIN7_MASK|PIN9_MASK)

/* Not in original diagram; just following logical sequence. */
#define	PSXNTPAD6_DATA		PIN10_MASK
#define	PSXNTPAD6_CMD		PIN2_MASK
#define	PSXNTPAD6_SEL		PIN3_MASK
#define	PSXNTPAD6_CLK		PIN9_MASK
#define	PSXNTPAD6_ACK		PIN12_MASK
#define	PSXNTPAD6_BASE		(PIN5_MASK|PIN6_MASK|PIN7_MASK|PIN8_MASK)

/* Bit masks for Linux gamecon.c joystick interface */
#define	GAMECONJOY0_DATA	PIN10_MASK
#define	GAMECONJOY1_DATA	PIN11_MASK
#define	GAMECONJOY2_DATA	PIN12_MASK
#define	GAMECONJOY3_DATA	PIN13_MASK
#define	GAMECONJOY4_DATA	PIN15_MASK

#define	GAMECONJOY_UP		PIN2_MASK
#define	GAMECONJOY_DOWN		PIN3_MASK
#define	GAMECONJOY_LEFT		PIN4_MASK
#define	GAMECONJOY_RIGHT	PIN5_MASK
#define	GAMECONJOY_FIRE1	PIN6_MASK
#define	GAMECONJOY_FIRE2	PIN7_MASK

/* Constants for DirectPad Pro v6  Sega Genesis interface */
#define	DPP6GEN_UP(X)		PIN10(~(X))		// Pin 10 levels are inverted
#define	DPP6GEN_DOWN(X)		PIN15(X)
#define	DPP6GEN_LEFT(X)		PIN10(~(X))		// Pin 10 levels are inverted
#define	DPP6GEN_RIGHT(X)	PIN11(X)
#define	DPP6GEN_FIRE1(X)	PIN12(X)
#define	DPP6GEN_FIRE2(X)	PIN13(X)

#endif

