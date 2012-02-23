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


#ifndef __PPJIOCTL_H__
#define __PPJIOCTL_H__

/* Ensure that our structures are as tightly packed as possible - ensure same packing for 32/64 bit code */
#pragma pack(push,1)

/* Define to use byte-size values for joystick axes, else dword size */
#ifdef UCHAR_AXES
#error UCHAR_AXES no longer supported!!
#endif

#define	PPJOY_AXIS_MIN				1
#define	PPJOY_AXIS_MAX				32767

#define	PPJOY_AXIS_CENTRE			((PPJOY_AXIS_MAX+PPJOY_AXIS_MIN)/2)
#define	PPJOY_AXIS_SWING			(PPJOY_AXIS_MAX-PPJOY_AXIS_MIN)
#define	PPJOY_AXIS_HALFSWING		((PPJOY_AXIS_MAX-PPJOY_AXIS_MIN)/2)
#define	PPJOY_AXIS_HITRIP			((3*PPJOY_AXIS_MAX+PPJOY_AXIS_MIN)/4)
#define	PPJOY_AXIS_LOTRIP			((PPJOY_AXIS_MAX+3*PPJOY_AXIS_MIN)/4)

#define	PPJOY_AXIS_REV(AXIS)		(PPJOY_AXIS_MIN+PPJOY_AXIS_MAX-(AXIS))

#if 0
#define	PPJOY_HAT_0DEG				((0*PPJOY_AXIS_SWING)/360+PPJOY_AXIS_MIN)
#define	PPJOY_HAT_45DEG				((45*PPJOY_AXIS_SWING)/360+PPJOY_AXIS_MIN)
#define	PPJOY_HAT_90DEG				((90*PPJOY_AXIS_SWING)/360+PPJOY_AXIS_MIN)
#define	PPJOY_HAT_135DEG			((135*PPJOY_AXIS_SWING)/360+PPJOY_AXIS_MIN)
#define	PPJOY_HAT_180DEG			((180*PPJOY_AXIS_SWING)/360+PPJOY_AXIS_MIN)
#define	PPJOY_HAT_225DEG			((225*PPJOY_AXIS_SWING)/360+PPJOY_AXIS_MIN)
#define	PPJOY_HAT_270DEG			((270*PPJOY_AXIS_SWING)/360+PPJOY_AXIS_MIN)
#define	PPJOY_HAT_315DEG			((315*PPJOY_AXIS_SWING)/360+PPJOY_AXIS_MIN)
#endif

#define	PPJOY_HAT_MIN				0
#define	PPJOY_HAT_MAX				35999
#define	PPJOY_HAT_SWING				(PPJOY_HAT_MAX-PPJOY_HAT_MIN)
#define	PPJOY_HAT_NUL				(PPJOY_HAT_MAX+1)

#define	PPJOY_HAT_0DEG				(0)
#define	PPJOY_HAT_45DEG				(4500)
#define	PPJOY_HAT_90DEG				(9000)
#define	PPJOY_HAT_135DEG			(13500)
#define	PPJOY_HAT_180DEG			(18000)
#define	PPJOY_HAT_225DEG			(22500)
#define	PPJOY_HAT_270DEG			(27000)
#define	PPJOY_HAT_315DEG			(31500)

#define	PPJOY_AXISTOHAT(AXIS)		(((((AXIS)-PPJOY_AXIS_MIN)*PPJOY_HAT_SWING)/PPJOY_AXIS_SWING)+PPJOY_HAT_MIN)

#define FILE_DEVICE_PPJOYBUS			FILE_DEVICE_BUS_EXTENDER
#define FILE_DEVICE_PPORTJOY			FILE_DEVICE_UNKNOWN

#define PPJOYBUS_IOCTL(_index_)	\
	CTL_CODE (FILE_DEVICE_PPJOYBUS, _index_, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define PPJOYBUS_INTERNAL_IOCTL(_index_) \
    CTL_CODE (FILE_DEVICE_PPJOYBUS, _index_, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define PPORTJOY_IOCTL(_index_)	\
	CTL_CODE (FILE_DEVICE_PPORTJOY, _index_, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define PPORTJOY_INTERNAL_IOCTL(_index_) \
    CTL_CODE (FILE_DEVICE_PPORTJOY, _index_, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PPJOYBUS_ADD_JOY			PPJOYBUS_IOCTL (0x3)
#define IOCTL_PPJOYBUS_DEL_JOY			PPJOYBUS_IOCTL (0x4)
#define IOCTL_PPJOYBUS_ENUM_JOY			PPJOYBUS_IOCTL (0x5)

#define IOCTL_PPJOYBUS_GET_OPTIONS		PPORTJOY_IOCTL (0x6)
#define IOCTL_PPJOYBUS_SET_OPTIONS		PPORTJOY_IOCTL (0x7)

#define IOCTL_PPJOYBUS_CHIPMODE			PPJOYBUS_INTERNAL_IOCTL (0x101)
#define	IOCTL_PPJOYBUS_GETCONFIG1		PPJOYBUS_INTERNAL_IOCTL (0x103)
#define IOCTL_PPJOYBUS_ALLOCIRQ1		PPJOYBUS_INTERNAL_IOCTL (0x104)

#define IOCTL_PPJOYBUS_REGOPTIONS		PPJOYBUS_INTERNAL_IOCTL (0x105)

#define IOCTL_PPORTJOY_SET_STATE		PPORTJOY_IOCTL (0x0)
#define IOCTL_PPORTJOY_GET_STATE		PPORTJOY_IOCTL (0x1)
#define IOCTL_PPORTJOY_SET_MAPPING		PPORTJOY_IOCTL (0x2)
#define IOCTL_PPORTJOY_GET_MAPPING		PPORTJOY_IOCTL (0x3)
#define IOCTL_PPORTJOY_DEL_MAPPING		PPORTJOY_IOCTL (0x4)

#define IOCTL_PPORTJOY_SET_FORCE		PPORTJOY_IOCTL (0x5)

#define IOCTL_PPORTJOY_SET_TIMING		PPORTJOY_IOCTL (0x6)
#define IOCTL_PPORTJOY_GET_TIMING		PPORTJOY_IOCTL (0x7)
#define IOCTL_PPORTJOY_DEL_TIMING		PPORTJOY_IOCTL (0x8)
#define IOCTL_PPORTJOY_ACT_TIMING		PPORTJOY_IOCTL (0x9)

#define IOCTL_PPORTJOY_GET_OPTIONS		PPORTJOY_IOCTL (0xA)
#define IOCTL_PPORTJOY_SET_OPTIONS		PPORTJOY_IOCTL (0xB)

typedef struct
{
 ULONG			Size;				/* Number of bytes in this structure */
 ULONG			PortAddress;		/* Base address for LPT port specified in LPTNumber */
 UCHAR			JoyType;			/* Index into joystick type table. 0 is no joystick */
 UCHAR			JoySubType;			/* Sub-model of joystick type, 0 if not applicable */
 UCHAR			UnitNumber;			/* Index of joystick on interface. First unit is 0 */
 UCHAR			LPTNumber;			/* Number of LPT port, 0 is virtual interface */
 USHORT			VendorID;			/* PnP vendor ID for this device */
 USHORT			ProductID;			/* PnP product ID for this device */
} JOYSTICK_CONFIG1, *PJOYSTICK_CONFIG1;

/* Structure to pass information about a joystick to add to the system. Callers should */
/* fill all the fields of JOYSTICK_CONFIG1 except the PortAddress field. */
typedef struct
{
 JOYSTICK_CONFIG1	JoyData;		/* Data for joystick to add */
 ULONG				Persistent;		/* 1= automatically add joystick after reboot */
} JOYSTICK_ADD_DATA, *PJOYSTICK_ADD_DATA;

/* Structure to specify a joystick to delete. Callers should fill in the Size, JoyType, */
/* UnitNumber and LPTNumber fields. Set all other fields to 0 */
typedef struct
{
 JOYSTICK_CONFIG1	JoyData;		/* Data for joystick to delete */
} JOYSTICK_DEL_DATA, *PJOYSTICK_DEL_DATA;

/* Structure to enumerate all the joysticks joysticks currently defined */
typedef struct
{
 ULONG				Count;			/* Number of joystick currently defined */	/* No, num returned below */
 ULONG				Size;			/* Size needed to enumerate all joysticks */
 JOYSTICK_CONFIG1	Joysticks[1];	/* Array of joystick records */		
} JOYSTICK_ENUM_DATA, *PJOYSTICK_ENUM_DATA;

#define	JOYSTICK_STATE_V1	0x53544143

typedef struct
{
 ULONG	Version;
 UCHAR	Data[1];
} JOYSTICK_SET_STATE, *PJOYSTICK_SET_STATE;

#define	JOYSTICK_FORCE_V1	0x21474f44

typedef struct
{
 ULONG	Version;
 UCHAR	Data[1];
} JOYSTICK_SET_FORCE, *PJOYSTICK_SET_FORCE;

typedef struct
{
 UCHAR		NumAxes;
 UCHAR		NumButtons;
 UCHAR		NumHats;
 UCHAR		NumMaps;
 UCHAR		Data[1];
} JOYSTICK_MAP, *PJOYSTICK_MAP;


#define	UNUSED_MAPPING			0xFF
#define	DIGITAL_VALUE(X)		((X)&0x7F)
#define	ANALOG_VALUE(X)			(((X)&0x3F)|0x80)


#define	MAP_SCOPE_DEFAULT	0
#define	MAP_SCOPE_DRIVER	1
#define	MAP_SCOPE_DEVICE	2

#define	JOYSTICK_MAP_V1	0x454E4F47

typedef struct
{
 ULONG		Version;
 UCHAR		MapScope;
 UCHAR		JoyType;
 UCHAR		Pad1;
 UCHAR		Pad2;
 ULONG		MapSize;
 /* Followed by
 JOYSTICK_MAP	MapData;
 */
} JOYSTICKMAP_HEADER, *PJOYSTICKMAP_HEADER;

#ifdef _NTDDK_
typedef struct
{
 PKSERVICE_ROUTINE	Callback;	
 PVOID				Context;
} JOYSTICK_IRQ1, *PJOYSTICK_IRQ1;
#endif

#define	TIMING_SCOPE_DEFAULT	0
#define	TIMING_SCOPE_DRIVER	1
#define	TIMING_SCOPE_DEVICE	2

#define	TIMING_V1	0x54494D45

typedef struct
{
 ULONG		Version;
 UCHAR		TimingScope;
 UCHAR		JoyType;
 UCHAR		Pad1;
 UCHAR		Pad2;
 ULONG		TimingSize;
 /* Followed by
 TIMING_xxx	TimingData;
 */
} TIMING_HEADER, *PTIMING_HEADER;

typedef struct
{
 ULONG		BitDelay;
 ULONG		Bit6Delay;
} TIMING_GENESIS, *PTIMING_GENESIS;

typedef struct
{
 ULONG		Ack1Delay;
 ULONG		AckXDelay;
 ULONG		BitDelay;
 ULONG		SelDelay;
 ULONG		TailDelay;
} TIMING_PSX, *PTIMING_PSX;

typedef struct
{
 ULONG		BitDelay;
 ULONG		SetupDelay;
} TIMING_SNES, *PTIMING_SNES;

typedef struct
{
 ULONG		RowDelay;
} TIMING_LPTSWITCH, *PTIMING_LPTSWITCH;

typedef struct
{
 ULONG		MaxPulseWidth;
 ULONG		MinPulseWidth;
 ULONG		MinSyncWidth;
} TIMING_FMSPPM, *PTIMING_FMSPPM;

typedef union
{
 TIMING_GENESIS		genesis;
 TIMING_SNES		snes;
 TIMING_PSX			psx;
 TIMING_LPTSWITCH	lptswitch;
 TIMING_FMSPPM		fmsppm;
} JOYTIMING, *PJOYTIMING;

#define	PPJOY_OPTION_UNUSEDOPTION	0
#define	PPJOY_OPTION_RUNTIMEDEBUG	1
#define	PPJOY_OPTION_PORTFLAGS		2

#define	PPJOY_OPTNAME_RUNTIMEDEBUG	L"RuntimeDebug"
#define	PPJOY_OPTNAME_PORTFLAGS		L"PortFlags"

/* Constants for the various runtime debugging options */
#define	RTDEBUG_FMS					0x00000001
#define	RTDEBUG_PSXERR				0x00000002

/* Constants for the port flags */
#define	PORTFLAG_NOALLOC			0x00000001

typedef struct
{
 ULONG		Option;
 ULONG		Value;
} PPJOY_OPTIONS, *PPPJOY_OPTIONS;

typedef void (*PPJOY_OPTION_CALLBACK)(ULONG Option, ULONG Value);

#pragma pack(pop)

#endif





