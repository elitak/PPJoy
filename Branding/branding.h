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


#ifndef	__BRANDING_H__
#define	__BRANDING_H__

// Version number - decimal values for each of the four components of the final version number
#define	VERSION_MAJOR			0
#define	VERSION_MINOR			84
#define	VERSION_SUB1			6
#define	VERSION_SUB2			000

#define	ENABLE_64BIT_DRIVER

#define	PRODUCT_NAME_SHORT		"PPJoy"
#define	PRODUCT_NAME_LONG		"PPJoy parallel port joystick driver"
#define	COMPANY_NAME			"Deon van der Westhuysen"
#define COPYRIGHT_YEARS			"2002-2004"
#define COPYRIGHT_STRING		"Copyright (C) " COMPANY_NAME " " COPYRIGHT_YEARS

#define	BUS_DRIVER_DESCRIPTION	"Parallel Port Joystick Bus Enumerator"
#define	BUS_DRIVER_BASENAME		"PPJoyBus"
#define	JOY_DRIVER_DESCRIPTION	"Parallel Port Joystick Device Driver"
#define	JOY_DRIVER_BASENAME		"PPortJoy"
#define	JOY_DRIVER_SERVICENAME	"PPortJoystick"
#define	W98_DRIVER_DESCRIPTION	"Parallel Port Win2K function emulator"
#define	W98_DRIVER_BASENAME		"W98Ports"
#define	BUS_DRIVER_ADDSVCREG	""
#define	JOY_DRIVER_ADDSVCREG	""
#define	JOY_DRIVER_LEGACYINF	// Include legacy PPJoy entry in .INF file

#define	CPL_APPLET_FILENAME		"PPortJoy.cpl"
#define	CPL_APPLET_DESCRIPTION	"Configuration utility for joysticks that plug into the machine's  parallel port"
#define	CPL_APPLET_WINDOWTEXT	"This utility allow you to configure and remove joysticks and gamepads connected to parallel ports\n\nPPJoy is (C) Copyright Deon van der Westhuysen 2002-2008."
#define	CPL_APPLET_TITLE		"PPJoy Joystick and Gamepad configuration utility   v"
#define	CPL_APPLET_REGISTRYKEY	"Control Panel\\PPJoy"
#define	CPL_APPLET_CPLNAME		"Parallel Port Joysticks"
#define	CPL_APPLET_CPLDESC		CPL_APPLET_DESCRIPTION
#undef	CPL_SHOWDONATEDIALOG

#define	VIRTUAL_JOYSTICK_NAME	"PPJoy Virtual Joystick"

#define	BUS_DRIVER_DEVICE_ID	"PPJOYBUS"
#define	BUS_DRIVER_CHILD_NAME	"Parallel Port Joystick device"
#define	JOY_DEVINSTANCE_NAME	"PPJoy"
#define	JOY_CONTROL_DEV_NAME	"PPJoyCtl%d:%d"
#define	JOY_IOCTL_DEV_NAME		"PPJoyIOCTL%d"
#define	JOY_VENDOR_ID			0xDEAD
#define	JOY_PRODUCT_ID_BASE		0xBEF0

/*********************************************************************/
/* End of customisation section. Do not modify the definitions below */
/*********************************************************************/

#define	MAKESTR(x)		MAKESTR2(x)
#define	MAKESTR2(x)		#x
#define	MAKELSTR(x)		MAKELSTR2(x)
#define	MAKELSTR2(x)	L##x
#ifdef  _UNICODE
#define	MAKETSTR(x)		MAKELSTR2(x)
#else
#define	MAKETSTR(x)		x
#endif

#define	PNPID_FORMAT			"\\VID_%04X&PID_%04X"

#define	BUS_DRIVER_FILENAME		BUS_DRIVER_BASENAME ".sys"
#define	BUS_DRIVER64_FILENAME	BUS_DRIVER_BASENAME "64.sys"
#define	BUS_DRIVER_INFNAME		BUS_DRIVER_BASENAME ".INF"
#ifndef	BUS_DRIVER_SERVICENAME
#define	BUS_DRIVER_SERVICENAME	BUS_DRIVER_BASENAME
#endif

#define	JOY_DRIVER_FILENAME		JOY_DRIVER_BASENAME ".sys"
#define	JOY_DRIVER64_FILENAME	JOY_DRIVER_BASENAME "64.sys"
#define	JOY_DRIVER_INFNAME		JOY_DRIVER_BASENAME ".INF"
#ifndef	JOY_DRIVER_SERVICENAME
#define	JOY_DRIVER_SERVICENAME	JOY_DRIVER_BASENAME
#endif

#define	W98_DRIVER_FILENAME		W98_DRIVER_BASENAME ".sys"
#define	W98_DRIVER_INFNAME		W98_DRIVER_BASENAME ".INF"
#ifndef	W98_DRIVER_SERVICENAME
#define	W98_DRIVER_SERVICENAME	W98_DRIVER_BASENAME
#endif

#define	LBUS_DRIVER_CHILD_NAME	MAKELSTR(BUS_DRIVER_CHILD_NAME)
#define LJOY_DEVINSTANCE_NAME	MAKELSTR(JOY_DEVINSTANCE_NAME)
#define	LJOY_CONTROL_DEV_NAME	MAKELSTR(JOY_CONTROL_DEV_NAME)
#define	LJOY_IOCTL_DEV_NAME		MAKELSTR(JOY_IOCTL_DEV_NAME)
#define	LCPL_APPLET_FILENAME	MAKELSTR(CPL_APPLET_FILENAME)

#define	JOY_DEVICE_PNP_ID		BUS_DRIVER_DEVICE_ID PNPID_FORMAT
#define	LJOY_DEVICE_PNP_ID		MAKELSTR(BUS_DRIVER_DEVICE_ID) MAKELSTR(PNPID_FORMAT)
#define	TJOY_DEVICE_PNP_ID		MAKETSTR(BUS_DRIVER_DEVICE_ID) MAKETSTR(PNPID_FORMAT)

#undef VER_PRODUCTVERSION
#define VER_PRODUCTVERSION			VERSION_MAJOR,VERSION_MINOR,VERSION_SUB1,VERSION_SUB2
#undef VER_PRODUCTVERSION_STR
#define VER_PRODUCTVERSION_STR		MAKESTR(VERSION_MAJOR) "." MAKESTR(VERSION_MINOR) "." MAKESTR(VERSION_SUB1) "." MAKESTR(VERSION_SUB2)
#undef LVER_PRODUCTVERSION_STR
#define LVER_PRODUCTVERSION_STR		MAKELSTR(MAKESTR(VERSION_MAJOR)) L"." MAKELSTR(MAKESTR(VERSION_MINOR)) L"." MAKELSTR(MAKESTR(VERSION_SUB1)) L"." MAKELSTR(MAKESTR(VERSION_SUB2))

#define TVER_PRODUCTVERSION_STR		MAKETSTR(MAKESTR(VERSION_MAJOR)) MAKETSTR(".") MAKETSTR(MAKESTR(VERSION_MINOR)) MAKETSTR(".") MAKETSTR(MAKESTR(VERSION_SUB1)) MAKETSTR(".") MAKETSTR(MAKESTR(VERSION_SUB2))

#endif
