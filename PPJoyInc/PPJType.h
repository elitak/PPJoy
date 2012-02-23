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


/***************************************************************************/
/**                                                                       **/
/**  Parallel port joystick driver, (C) Deon van der Westhuysen 2002      **/
/**                                                                       **/
/**  PPJType.h: Define various joystick interfaces we support and a       **/
/**             textual name for each                                     **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/***************************************************************************/

#ifndef	__PPJTYPE_H__
#define	__PPJTYPE_H__

#define	DEVTYPE_JOYSTICK			1
#define	DEVTYPE_GENESIS				2
#define	DEVTYPE_SNESPAD				3
#define	DEVTYPE_PSXPAD				4
#define	DEVTYPE_IOCTL				5
#define	DEVTYPE_RETIRED				6
#define	DEVTYPE_RADIOTX				7

#define	DEVTYPE_JOYSTICK_NAME		"Joystick"
#define	DEVTYPE_GENESIS_NAME		"Genesis pad"
#define	DEVTYPE_SNESPAD_NAME		"SNES or NES pad"
#define	DEVTYPE_PSXPAD_NAME			"Playstation pad"
#define	DEVTYPE_IOCTL_NAME			"Virtual joystick"
#define	DEVTYPE_RETIRED_NAME		"Retired devices"
#define	DEVTYPE_RADIOTX_NAME		"Radio control TX"

/* The numbers assigned here must match up with the array indexes for the */
/* JoyDefs table in Joystick.c  Note that "TheMaze" will be at index 1,   */
/* and not 0, in the array.                                               */
#define	IF_THEMAZE					1
#define	IF_IANHARRIES				2
#define	IF_TURBOGRAFX				3
#define	IF_LINUX0802				4
#define	IF_LINUXDB9C				5
#define	IF_TORMOD					6
#define	IF_DIRECTPRO				7
#define	IF_TURBOGFX45				8
#define IF_LPTJOYSTICK				9
#define IF_CHAMPGAMES				10
#define IF_STFORMAT					11
#define IF_SNESKEY2600				12
#define IF_AMIGA4PLAYER				13
#define IF_PCAE						14
#define	IF_GENESISLIN				15
#define	IF_GENESISDPP				16
#define	IF_GENESISNTP				17
#define	IF_SNESPADLIN				18
#define	IF_SNESPADDPP				19
#define	IF_GENESISCC				20
#define	IF_GENESISSNES				21
#define	IF_PSXPBLIB					22
#define	IF_PSXDPADPRO				23
#define	IF_PSXLINUX					24
#define	IF_PSXNTPADXP				25
#define	IF_PSXMEGATAP				26
#define	IF_IOCTL					27
#define	IF_LINUXGAMECON				28
#define	IF_LPTSWITCH				29
#define	IF_FMSBUDDYBOX				30
#define	IF_POWERPADLIN				31
#define	IF_GENESISDPP6				32

//#define	JOYTYPE_MULTIJOY			

#define	IF_THEMAZE_NAME			"TheMaze"
#define	IF_IANHARRIES_NAME		"IanHerries"
#define	IF_TURBOGRAFX_NAME		"TurboGraFX"
#define	IF_LINUX0802_NAME		"Linux v0802"
#define	IF_LINUXDB9C_NAME		"Linux DB9.c"
#define	IF_TORMOD_NAME			"Tormod"
#define	IF_DIRECTPRO_NAME		"DirectPad Pro"
#define	IF_TURBOGFX45_NAME		"TurboGraFX (swapped buttons)"
#define IF_LPTJOYSTICK_NAME		"LPT JoyStick"
#define IF_CHAMPGAMES_NAME		"CHAMPgames"
#define IF_STFORMAT_NAME		"STFormat"
#define IF_SNESKEY2600_NAME		"SNESkey2600"
#define IF_AMIGA4PLAYER_NAME	"Amiga 4 Player"
#define IF_PCAE_NAME			"PCAE"
#define	IF_GENESISLIN_NAME		"Linux"
#define	IF_GENESISDPP_NAME		"DirectPad Pro"
#define	IF_GENESISNTP_NAME		"NTPad XP"
#define	IF_SNESPADLIN_NAME		"Linux"
#define	IF_SNESPADDPP_NAME		"DirectPad Pro/SNESkey"
#define	IF_GENESISCC_NAME		"ConsoleCable"
#define	IF_GENESISSNES_NAME		"SNESkey"
#define	IF_PSXPBLIB_NAME		"PSXPBLIB"
#define	IF_PSXDPADPRO_NAME		"DirectPad Pro"
#define	IF_PSXLINUX_NAME		"Linux"
#define	IF_PSXNTPADXP_NAME		"NTPad XP"
#define	IF_PSXMEGATAP_NAME		"Megatap"
#define	IF_IOCTL_NAME			"Virtual joystick (IOCTL)"
#define	IF_LINUXGAMECON_NAME	"Linux gamecon"
#define	IF_LPTSWITCH_NAME		"LPT-switch"
#define	IF_FMSBUDDYBOX_NAME		"PPM interface"
#define	IF_POWERPADLIN_NAME		"PowerPad"
#define	IF_GENESISDPP6_NAME		"DirectPad Pro v6"


//#define	JOYTYPE_MULTIJOY_NAME		"MULTIJOY"

#define	IF_THEMAZE_DEV			DEVTYPE_JOYSTICK
#define	IF_IANHARRIES_DEV		DEVTYPE_JOYSTICK
#define	IF_TURBOGRAFX_DEV		DEVTYPE_JOYSTICK
#define	IF_LINUX0802_DEV		DEVTYPE_JOYSTICK
#define	IF_LINUXDB9C_DEV		DEVTYPE_JOYSTICK
#define	IF_TORMOD_DEV			DEVTYPE_JOYSTICK
#define	IF_DIRECTPRO_DEV		DEVTYPE_JOYSTICK
#define	IF_TURBOGFX45_DEV		DEVTYPE_RETIRED
#define IF_LPTJOYSTICK_DEV		DEVTYPE_JOYSTICK
#define IF_CHAMPGAMES_DEV		DEVTYPE_JOYSTICK
#define IF_STFORMAT_DEV			DEVTYPE_JOYSTICK
#define IF_SNESKEY2600_DEV		DEVTYPE_JOYSTICK
#define IF_AMIGA4PLAYER_DEV		DEVTYPE_JOYSTICK
#define IF_PCAE_DEV				DEVTYPE_JOYSTICK
#define	IF_GENESISLIN_DEV		DEVTYPE_GENESIS
#define	IF_GENESISDPP_DEV		DEVTYPE_GENESIS
#define	IF_GENESISNTP_DEV		DEVTYPE_GENESIS
#define	IF_SNESPADLIN_DEV		DEVTYPE_SNESPAD
#define	IF_SNESPADDPP_DEV		DEVTYPE_SNESPAD
#define	IF_GENESISCC_DEV		DEVTYPE_GENESIS
#define	IF_GENESISSNES_DEV		DEVTYPE_GENESIS
#define	IF_PSXPBLIB_DEV			DEVTYPE_PSXPAD
#define	IF_PSXDPADPRO_DEV		DEVTYPE_PSXPAD
#define	IF_PSXLINUX_DEV			DEVTYPE_PSXPAD
#define	IF_PSXNTPADXP_DEV		DEVTYPE_PSXPAD
#define	IF_PSXMEGATAP_DEV		DEVTYPE_PSXPAD
#define	IF_IOCTL_DEV			DEVTYPE_IOCTL
#define	IF_LINUXGAMECON_DEV		DEVTYPE_JOYSTICK
#define	IF_LPTSWITCH_DEV		DEVTYPE_JOYSTICK
#define	IF_FMSBUDDYBOX_DEV		DEVTYPE_RADIOTX
#define	IF_POWERPADLIN_DEV		DEVTYPE_SNESPAD
#define	IF_GENESISDPP6_DEV		DEVTYPE_GENESIS

#define	SUBTYPE_DEFAULT			0
#define	SUBTYPE_GENESYS3		0
#define	SUBTYPE_GENESYS6		1
#define	SUBTYPE_SNES_SNES		0
#define	SUBTYPE_SNES_NES		1

#define	SUBTYPE_DEFAULT_NAME	"default"
#define	SUBTYPE_GENESYS3_NAME	"A,B,C and Start"
#define	SUBTYPE_GENESYS6_NAME	"A,B,C,X,Y,Z, Start and Mode"
#define	SUBTYPE_SNES_SNES_NAME	"SNES or Virtual Gameboy"
#define	SUBTYPE_SNES_NES_NAME	"NES"

#endif
