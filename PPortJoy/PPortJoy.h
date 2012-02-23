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
/**  PPortJoy.h #defines, Type declarations and include files for the     **/
/**             Parallel Port Joystick FDO driver.                        **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#ifndef __PPORTJOY_H__
#define __PPORTJOY_H__

/**************************************************************************/
/* Standard include files for this driver                                 */
/**************************************************************************/

#include "wdm.h"
#include "hidport.h"			// From hidgame's inc directory

#include <branding.h>

#include "PPJIoctl.h"
#include "PPJType.h"

#include "debug.h"

#include "PPJoyMsg.h"

#pragma pack(push,1)

/**************************************************************************/
/* Configuration #defines for the driver                                  */
/**************************************************************************/

/* Define this to enable 5 extra buttons for LinuxDB9c interface */
#define	LINUXDB9_EXTRABUTTONS

/* Define the maximum number of buttons and axes we will support */
#ifndef MAX_REPORT_AXES
#define	MAX_REPORT_AXES				8
#endif
#ifndef	MAX_REPORT_BUTTONS
#define	MAX_REPORT_BUTTONS			32
#endif
#ifndef	MAX_REPORT_HATS
#define	MAX_REPORT_HATS				2
#endif

#define	MAX_ANALOG_RAW				64
#define	MAX_DIGITAL_RAW				128

/* Setting for HID report */
#define PPJOY_REPORT_MAXSIZE		20+7*(MAX_REPORT_AXES)+10+7*(MAX_REPORT_HATS)+13*(MAX_REPORT_BUTTONS)
#define	PPJOY_VERSION_NUMBER		1

/* Define the maximum Joystick Type number. */
#define	PPJOY_MAX_JOYTYPE	IF_GENESISDPP6

/* Define possible bit values for Flags in DeviceExtension */
#define	PPJOY_FLAGS_STARTED	1
#define	PPJOY_FLAGS_REMOVED	2

/* Name for the driver parameters subkey in the registry */
#define PARAM_KEY_NAME			L"\\Parameters"

#define	GENESIS_DELAY			2		/* microseconds */
#define	GENESIS6_DELAY			14		/* microseconds */
#define	SNES_BITDELAY			6		/* microseconds */
#define	SNES_SETUPDELAY			12		/* microseconds */
#define	LPTSWITCH_DELAY			3		/* microseconds */

#define	PSXBIT_DELAY			10		/* microseconds */
#define	PSXSEL_DELAY			30		/* microseconds */
#define	PSXACK1_DELAY			150		/* microseconds */
#define	PSXACKX_DELAY			45		/* microseconds */
#define	PSXTAIL_DELAY			45		/* microseconds */

#define	FMS_MINPULSE			1000	/* microseconds */
#define	FMS_MAXPULSE			2200	/* microseconds */
#define	FMS_MINSYNC				3500	/* microseconds */

/**************************************************************************/
/* Type definitions for the driver                                        */
/**************************************************************************/

/* Collection for everything we place in global memory. */
typedef struct
{
 UNICODE_STRING	ParamRegistryPath;
 KSPIN_LOCK		SpinLock;		/* Lock for excluseive access to port */
 PDRIVER_OBJECT DriverObject;	/* Keep it for IoAllocateErrorLogEntry() */
 int			NumPresentJoys;	/* Number of joysticks currently active */
#ifdef USE_SCAN_THREAD
 PVOID			ThreadObject;	/* Parallel Port scan thread */
 int			MustExit;		/* Flag - time to quit port scan thread */
#endif
} PPORTJOY_GLOBAL;

/* Structure for input data read from the joystick */
typedef struct
{
 ULONG	Analog[MAX_ANALOG_RAW];
 UCHAR	Digital[MAX_DIGITAL_RAW];
} RAWINPUT, *PRAWINPUT;

typedef struct
{
 ULONG	Axes[MAX_REPORT_AXES];
 ULONG	POVHats[MAX_REPORT_HATS];
 UCHAR	Buttons[MAX_REPORT_BUTTONS];
} HIDREPORT, *PHIDREPORT;

typedef	struct
{
 LARGE_INTEGER	MaxPulseTicks;		/* Computed number of ticks - max pulse */
 LARGE_INTEGER	MinPulseTicks;		/* Computed number of ticks - min pulse */
 LARGE_INTEGER	MinSyncTicks;		/* Computed number of ticks - min sync */
 LARGE_INTEGER	LastTickCount;		/* Tick count at last interrupt */
 UCHAR			NextIndex;			/* Next joystick value to read */
} PPMBUDDYBOX, *PPPMBUDDYBOX;

typedef union
{
 PPMBUDDYBOX	PPMBuddyBox;

} JOYSTICK_PARAM, *PJOYSTICK_PARAM;

/* Device extension for the miniport driver */
typedef struct
{
 ULONG				Flags;
 ULONG				ReqCount;
 KEVENT				RemoveEvent;
 JOYSTICK_CONFIG1	Config;			/* Joystick config data */
 RAWINPUT			RawInput;		/* Current joystick data to report */
 PDEVICE_OBJECT		CtlDevObject;	/* Control device for this joystick */
 PJOYSTICK_MAP		Mapping;		/* HID types and conversion rules */
 PJOYTIMING			Timing;			/* Timing parameters to read interface */
 JOYSTICK_PARAM		Param;			/* (Dymanic) Parameters for stick */
 UCHAR				ActiveMap;		/* Which joystick mapping to use */
 ULONG				OpenCount;		/* Current number of opens on device */
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

/* Device extension structure for our control device */
typedef struct
{
 /* We include the HID device extension (header) here so that our control */
 /* device's device extension would start with the same fields as the     */
 /* real device extension for joystick objects. We determine the type of  */
 /* device by looking if the MiniDeviceExtension field in the device      */
 /* extension is NULL - if it is then it is a control device and this     */
 /* device extenstion is used.                                            */
 HID_DEVICE_EXTENSION	hde;
 PDEVICE_OBJECT			JoyDeviceObject;
} CTLDEV_EXTENSION, *PCTLDEV_EXTENSION;


/**************************************************************************/
/* Declare global variables                                               */
/**************************************************************************/

extern PPORTJOY_GLOBAL	Globals;
extern PDRIVER_DISPATCH	HIDMajorFunctions[IRP_MJ_MAXIMUM_FUNCTION+1];


/**************************************************************************/
/* Helper macros                                                          */
/**************************************************************************/

#define GET_MINIDRIVER_DEVICE_EXTENSION(DO)  \
    ((PDEVICE_EXTENSION) (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->MiniDeviceExtension))

#define GET_NEXT_DEVICE_OBJECT(DO)			 \
    (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->NextDeviceObject)

#define GET_PHYSICAL_DEVICE_OBJECT(DO)		 \
    (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->PhysicalDeviceObject)


/**************************************************************************/
/* Function prototypes for all the modules                                */
/**************************************************************************/

/* In PPortJoy.c: */

NTSTATUS DriverEntry (IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
NTSTATUS PPJoy_Ctl_CreateClose (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_AddDevice (IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT FunctionalDeviceObject);
VOID PPJoy_Unload (IN PDRIVER_OBJECT DriverObject);

/* In PnP.c */

NTSTATUS PPJoy_PnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_Ctl_PnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_PnpStart (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_EnterRequest	(PDEVICE_EXTENSION DeviceExtension);
NTSTATUS PPJoy_LeaveRequest	(PDEVICE_EXTENSION DeviceExtension);
NTSTATUS PPJoy_InitPortAndInterface (IN PDEVICE_OBJECT DeviceObject);
NTSTATUS PPJoy_PnpUpdateRegistry (IN PDEVICE_OBJECT DeviceObject);
void PPJoy_UpdateOptions (ULONG Option, ULONG Value);

/* In Power.c */

NTSTATUS PPJoy_Power (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_Ctl_Power (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

PCHAR PowerMinorFunctionString (UCHAR MinorFunction);
PCHAR DbgSystemPowerString (IN SYSTEM_POWER_STATE Type);
PCHAR DbgDevicePowerString (IN DEVICE_POWER_STATE Type);
    
/* In Ioctl.c */

NTSTATUS PPJoy_InternalIoctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_Ctl_InternalIoctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_Ctl_Ioctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_GetDeviceDescriptor (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_GetReportDescriptor (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_GetAttributes (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_ReadReport (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_GenerateReport (IN PDEVICE_OBJECT DeviceObject, OUT UCHAR rgGameReport[PPJOY_REPORT_MAXSIZE], OUT PUSHORT pCbReport);
NTSTATUS PPJoy_SetJoystickState (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_GetJoystickState (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_SetJoystickMap (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_GetJoystickMap (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_DelJoystickMap (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS PPJoy_SetJoyTiming (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_GetJoyTiming (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_DelJoyTiming (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_ActJoyTiming (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS PPJoy_GetPPJoyOptions (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoy_SetPPJoyOptions (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

/* In Joystick.c */

void InitIanHarries (PUCHAR BasePort, UCHAR JoyType);
void InitTurboGraFX (PUCHAR BasePort, UCHAR JoyType);
void InitTheMaze (PUCHAR BasePort, UCHAR JoyType);
void InitLinux0802 (PUCHAR BasePort, UCHAR JoyType);
void InitLinuxDB9c (PUCHAR BasePort, UCHAR JoyType);
void InitTormod (PUCHAR BasePort, UCHAR JoyType);
void InitDirectPro (PUCHAR BasePort, UCHAR JoyType);

void InitLPTJoystick (PUCHAR BasePort, UCHAR JoyType);
void InitCHAMPGames  (PUCHAR BasePort, UCHAR JoyType);
void InitSTFormat (PUCHAR BasePort, UCHAR JoyType);
void InitSNESKey2600 (PUCHAR BasePort, UCHAR JoyType);
void InitAmiga4Play (PUCHAR BasePort, UCHAR JoyType);

void InitPCAE (PUCHAR BasePort, UCHAR JoyType);
void InitGenesisLin (PUCHAR BasePort, UCHAR JoyType);
void InitGenesisDPP (PUCHAR BasePort, UCHAR JoyType);
void InitGenesisNTP (PUCHAR BasePort, UCHAR JoyType);
void InitSNESPad (PUCHAR BasePort, UCHAR JoyType);

void InitGenesisCC (PUCHAR BasePort, UCHAR JoyType);
void InitGenesisSNES (PUCHAR BasePort, UCHAR JoyType);
void InitGenesisDPP6 (PUCHAR BasePort, UCHAR JoyType);

void InitPSXStandard (PUCHAR BasePort, UCHAR JoyType);
void InitNullFunc (PUCHAR BasePort, UCHAR JoyType);

void InitLinuxGamecon (PUCHAR BasePort, UCHAR JoyType);
void InitLPTSwitch (PUCHAR BasePort, UCHAR JoyType);

void InitFMSBuddyBox (PUCHAR BasePort, UCHAR JoyType);

void InitPowerPadLin (PUCHAR BasePort, UCHAR JoyType);

void ReadIanHarries (PDEVICE_EXTENSION DeviceExtension);
void ReadTurboGraFX (PDEVICE_EXTENSION DeviceExtension);
void ReadTheMaze (PDEVICE_EXTENSION DeviceExtension);
void ReadLinux0802 (PDEVICE_EXTENSION DeviceExtension);
void ReadLinuxDB9c (PDEVICE_EXTENSION DeviceExtension);
void ReadTormod (PDEVICE_EXTENSION DeviceExtension);
void ReadDirectPro (PDEVICE_EXTENSION DeviceExtension);

void ReadLPTJoystick (PDEVICE_EXTENSION DeviceExtension);
void ReadCHAMPGames (PDEVICE_EXTENSION DeviceExtension);
void ReadSTFormat (PDEVICE_EXTENSION DeviceExtension);
void ReadSNESKey2600 (PDEVICE_EXTENSION DeviceExtension);
void ReadAmiga4Play (PDEVICE_EXTENSION DeviceExtension);

void ReadPCAE (PDEVICE_EXTENSION DeviceExtension);
void ReadGenesisLin (PDEVICE_EXTENSION DeviceExtension);
void ReadGenesisDPP (PDEVICE_EXTENSION DeviceExtension);
void ReadGenesisNTP (PDEVICE_EXTENSION DeviceExtension);
void ReadSNESPad (PDEVICE_EXTENSION DeviceExtension);

void ReadGenesisCC (PDEVICE_EXTENSION DeviceExtension);
void ReadGenesisSNES (PDEVICE_EXTENSION DeviceExtension);
void ReadGenesisDPP6 (PDEVICE_EXTENSION DeviceExtension);

void ReadPSXPBLIB (PDEVICE_EXTENSION DeviceExtension);
void ReadPSXMegaTap (PDEVICE_EXTENSION DeviceExtension);
void ReadPSXDirectPad (PDEVICE_EXTENSION DeviceExtension);
void ReadPSXLinux (PDEVICE_EXTENSION DeviceExtension);
void ReadPSXNTPadXP (PDEVICE_EXTENSION DeviceExtension);

void ReadNullFunc (PDEVICE_EXTENSION DeviceExtension);

void ReadLinuxGamecon (PDEVICE_EXTENSION DeviceExtension);
void ReadLPTSwitch (IN PDEVICE_EXTENSION DeviceExtension);

BOOLEAN ISR_FMSBuddyBox (PKINTERRUPT IntObj, IN PDEVICE_EXTENSION DeviceExtension);

void ReadPowerPadLin (IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS PPJoy_UpdateJoystickData (IN PDEVICE_EXTENSION DeviceExtension);

/* In IRPUtilc */
NTSTATUS PPortJoy_SendIrpSynchronously (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN UCHAR CopyStack);
NTSTATUS PPortJoy_CompletionRoutine (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);
NTSTATUS PPortJoy_SendInternalIoctl (IN PDEVICE_OBJECT DeviceObject, IN ULONG IoControlCode,
									 IN PVOID InputBuffer OPTIONAL, IN ULONG InputBufferLength,
									 OUT PVOID OutputBuffer OPTIONAL, IN ULONG OutputBufferLength,
									 IN LONG TimeoutSecs);

/* In EventLog.c */
void PPortJoy_WriteEventLog (NTSTATUS ErrorMessageCode, PVOID DumpData, USHORT DumpSize, PCWSTR Message);

/* In Config.c */
HANDLE PPJoy_OpenDeviceRegKey (IN PDEVICE_OBJECT DeviceObject);
HANDLE PPJoy_OpenDriverRegKey (void);
NTSTATUS PPJoy_ReadRegULONG (HANDLE RegKey, WCHAR *ValueName, PULONG Result, ULONG DefaultValue);
NTSTATUS PPJoy_WriteRegULONG (HANDLE RegKey, WCHAR *ValueName, ULONG Value);

NTSTATUS PPJoy_ReadDriverConfig (void);
NTSTATUS PPJoy_WriteDriverConfig (void);

/* In Mapping.c */

ULONG PPJOY_ComputeMappingSize (PJOYSTICK_MAP Mapping);

NTSTATUS PPJoy_DeleteMappingFromReg (UCHAR JoyType, HANDLE RegKey);
NTSTATUS PPJoy_DeleteDeviceMapping (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType);
NTSTATUS PPJoy_DeleteDriverMapping (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType);

NTSTATUS PPJoy_WriteMappingToReg (PJOYSTICK_MAP Mapping, UCHAR JoyType, HANDLE RegKey);
NTSTATUS PPJoy_WriteDeviceMapping (IN PDEVICE_OBJECT DeviceObject, PJOYSTICK_MAP Mapping, UCHAR JoyType);
NTSTATUS PPJoy_WriteDriverMapping (IN PDEVICE_OBJECT DeviceObject, PJOYSTICK_MAP Mapping, UCHAR JoyType);

PJOYSTICK_MAP PPJoy_ReadMappingFromReg (UCHAR JoyType, HANDLE RegKey);
PJOYSTICK_MAP PPJoy_ReadDeviceMapping (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType);
PJOYSTICK_MAP PPJoy_ReadDriverMapping (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType);
PJOYSTICK_MAP PPJoy_BuildDefaultMapping (UCHAR JoyType);

NTSTATUS PPJoy_LoadJoystickMapping (IN PDEVICE_OBJECT DeviceObject);
void PPJoy_MapRawToReport (PHIDREPORT Report, PRAWINPUT RawInput, PJOYSTICK_MAP Mapping, UCHAR MapIndex);

/* In Timing.c */

ULONG PPJOY_ComputeTimingSize (UCHAR JoyType);
void PPJoy_InitTimingValues (IN PDEVICE_OBJECT DeviceObject);

NTSTATUS PPJoy_DeleteTimingFromReg (UCHAR JoyType, HANDLE RegKey);
NTSTATUS PPJoy_DeleteDeviceTiming (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType);
NTSTATUS PPJoy_DeleteDriverTiming (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType);

NTSTATUS PPJoy_WriteTimingToReg (PJOYTIMING Timing, UCHAR JoyType, HANDLE RegKey);
NTSTATUS PPJoy_WriteDeviceTiming (IN PDEVICE_OBJECT DeviceObject, PJOYTIMING Timing, UCHAR JoyType);
NTSTATUS PPJoy_WriteDriverTiming (IN PDEVICE_OBJECT DeviceObject, PJOYTIMING Timing, UCHAR JoyType);

PJOYTIMING PPJoy_ReadTimingFromReg (UCHAR JoyType, HANDLE RegKey);
PJOYTIMING PPJoy_ReadDeviceTiming (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType);
PJOYTIMING PPJoy_ReadDriverTiming (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType);
PJOYTIMING PPJoy_BuildDefaultTiming (UCHAR JoyType);

NTSTATUS PPJoy_LoadJoystickTiming (IN PDEVICE_OBJECT DeviceObject);

#ifdef USE_SCAN_THREAD
/* In AsyncIO.c */
VOID PPJoy_ScanThread (IN PVOID StartContext);
NTSTATUS PPJoy_StartScanThread (void);
#endif

#pragma pack(pop)

#endif
