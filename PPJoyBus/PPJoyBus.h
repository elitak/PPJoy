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
/**  PPJoyBus.h #defines, Type declarations and include files for the     **/
/**             Parallel Port Joystick bus enumerator                     **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#ifndef __PPJOYBUS_H__
#define __PPJOYBUS_H__

/**************************************************************************/
/* Standard include files for this driver                                 */
/**************************************************************************/

#include <branding.h>

#include "wdm.h"
#include "PPJIoctl.h"
#include "Debug.h"

#include "BusMsg.h"

#pragma pack(push,1)

/**************************************************************************/
/* Configuration #defines for the driver                                  */
/**************************************************************************/

/* Name for the parameters subkey in the registry */
#define PARAM_KEY_NAME			L"\\Parameters"

/* Define (textual) device name. This is displayed in the Found New       */
/* Hardware Wizard dialogue box until the FDO driver is loaded.           */
#define PPJOYBUS_DEV_NAME		LBUS_DRIVER_CHILD_NAME
#define PPJOYBUS_DEV_NAME_LEN	sizeof(PPJOYBUS_DEV_NAME)

/* Maximum number of parallel ports that we will cater for. (LPT1-LPTx)   */
#define	MAX_PARPORTS			16

/* Maximum time (in seconds) to wait when allocating a parallel port.     */
#define	PORT_ALLOC_TIMEOUT		5

/* Define bit values for the Flags in the Device Extensions               */
#define	PPJFLAGS_ISBUSDEV		1
#define	PPJFLAGS_STARTED		2
#define	PPJFLAGS_REMOVED		4
#define	PPJFLAGS_HWFOUND		8
#define	PPJFLAGS_UNPLUGGED		16

/* Macro to determine whether we need to expose this PDO to the PnP       */
/* PnP manager, based on the flags passed as parameters.                  */
#define	EXPOSE_DEVICE(__x__)	(((__x__)&(PPJFLAGS_UNPLUGGED|PPJFLAGS_HWFOUND))==(PPJFLAGS_HWFOUND))



/**************************************************************************/
/* Type definitions for the driver                                        */
/**************************************************************************/

/* Fields that are common between Bus FDOs and Joystick PDOs */
typedef struct
{
 PDEVICE_OBJECT		Self;				/* Pointer to this DO */
 ULONG				Flags;
 ULONG				ReqCount;
 KEVENT				RemoveEvent;
} COMMON_DATA, *PCOMMON_DATA;

/* Device extension for Bus FDO */
typedef struct
{
 COMMON_DATA;
 PDRIVER_OBJECT		DriverObject;		/* Pointer to our Driver */
 PDEVICE_OBJECT		NextLowerDriver;	/* Pointer to next DO */
 PDEVICE_OBJECT		UnderlyingPDO;		/* Pointer to bottom DO */
 UNICODE_STRING		InterfaceName;
 PVOID				NotificationHandle;
 LIST_ENTRY			JoystickList;
} BUS_DEVICE_DATA, *PBUS_DEVICE_DATA;

/* Device extension for Joystick PDO */
typedef struct
{
 COMMON_DATA;
 LIST_ENTRY			ListEntry;
 JOYSTICK_CONFIG1	Config;
 JOYSTICK_IRQ1		IRQ;
} JOY_DEVICE_DATA, *PJOY_DEVICE_DATA;

/* Collection of information we keep for underlying parallel ports */
typedef struct
{
 PDEVICE_OBJECT	DevObj;
 PFILE_OBJECT	FileObj;
 ULONG			PortAddr;
 USHORT			UseCount;
 UCHAR			LastChipMode;
 UNICODE_STRING	SymLinkName;
} PARPORTINFO, *PPARPORTINFO;

/* Collection for everything we place in global memory. */
typedef struct
{
 UNICODE_STRING	ParamRegistryPath;
 PARPORTINFO	ParPorts[MAX_PARPORTS];	/* LPT1 is at index 0 */
 FAST_MUTEX		Mutex;					/* Global to all lists */
 PDRIVER_OBJECT DriverObject;	/* Keep it for IoAllocateErrorLogEntry() */
} GLOBALS;



/**************************************************************************/
/* Declare global variables                                               */
/**************************************************************************/

extern GLOBALS Globals;

extern ULONG	PPJoy_Opt_RuntimeDebug;
extern ULONG	PPJoy_Opt_PortFlags;

extern PPJOY_OPTION_CALLBACK	PPJoyOptionCallback;

/**************************************************************************/
/* Function prototypes for all the modules                                */
/**************************************************************************/

/* In PPJoyBus.c: */

NTSTATUS DriverEntry (IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
NTSTATUS PPJoyBus_CreateClose (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoyBus_PnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoyBus_Power (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoyBus_InternalIoctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoyBus_Ioctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoyBus_AddDevice (IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject);
VOID PPJoyBus_Unload (IN PDRIVER_OBJECT DriverObject);

VOID PPJoyBus_CreateFromRegistry (IN PDEVICE_OBJECT DeviceObject);
NTSTATUS PPJoyBus_RegQueryFunc (IN PWSTR ValueName, IN ULONG ValueType, IN PVOID ValueData,
								IN ULONG ValueLength, IN PVOID Context, IN PVOID EntryContext);
ULONG PPJoyBus_CrackParamString (IN PWSTR ParamString, UCHAR *LPTNumber, UCHAR *JoyType, UCHAR *UnitNumber);
ULONG PPJoyBus_GetHexDigit (IN WCHAR Char, OUT UCHAR *Value);

VOID PPJoyBus_ReadOptionsFromReg (IN PDEVICE_OBJECT DeviceObject);

/* In BusIoctl.c */

NTSTATUS PPJoyBus_InternalIoctl_Bus (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoyBus_Ioctl_Bus (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS PPJoyBus_AddJoystick (IN PDEVICE_OBJECT BusDeviceObject, IN PJOYSTICK_CONFIG1 Config, IN ULONG AddToReg);
NTSTATUS PPJoyBus_DelJoystick (IN PDEVICE_OBJECT BusDeviceObject, IN PJOYSTICK_CONFIG1 Config);
NTSTATUS PPJoyBus_EnumJoystick (IN PDEVICE_OBJECT BusDeviceObject, IN PJOYSTICK_ENUM_DATA EnumData, IN ULONG BufSize, OUT PULONG BytesUsed);

NTSTATUS PPJoyBus_GetOptions (IN PDEVICE_OBJECT BusDeviceObject, IN PPPJOY_OPTIONS Option, OUT PULONG BytesOut);
NTSTATUS PPJoyBus_SetOptions (IN PDEVICE_OBJECT BusDeviceObject, IN PPPJOY_OPTIONS Option, OUT PULONG BytesOut);

/* In JoyIoctl.c */

NTSTATUS PPJoyBus_InternalIoctl_Joy (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoyBus_Ioctl_Joy (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

/* In IRPUtilc */
NTSTATUS PPJoyBus_SendIrpSynchronously (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoyBus_CompletionRoutine (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);
NTSTATUS PPJoyBus_EnterRequest (PCOMMON_DATA CommonData);
NTSTATUS PPJoyBus_LeaveRequest (PCOMMON_DATA CommonData);


/* In ParPort.c */
NTSTATUS PPJoyBus_SendParPortIoctl (IN PDEVICE_OBJECT DeviceObject, IN ULONG IoControlCode,
									IN PVOID InputBuffer OPTIONAL, IN ULONG InputBufferLength,
									OUT PVOID OutputBuffer OPTIONAL, IN ULONG OutputBufferLength,
									IN LONG TimeoutSecs);

ULONG PPJoyBus_GetLptNumber (IN PWCHAR LptName);
ULONG PPJoyBus_PnPArrival (IN PDEVICE_OBJECT BusDevObject, IN PUNICODE_STRING SymLinkName);
ULONG PPJoyBus_PnPRemoval (IN PDEVICE_OBJECT BusDevObject, IN PUNICODE_STRING SymLinkName);
NTSTATUS PPJoyBus_PnPNotification (IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION Notification, IN PDEVICE_OBJECT BusDevObject);

NTSTATUS PPJoyBus_RegisterNotifications (IN PBUS_DEVICE_DATA BusDeviceData);
void PPJoyBus_UnregisterNotifications (IN PBUS_DEVICE_DATA BusDeviceData);

NTSTATUS PPJoyBus_ParSetChipMode (ULONG PortIndex, UCHAR NewChipMode);
NTSTATUS PPJoyBus_ParPortAlloc (ULONG PortIndex);
NTSTATUS PPJoyBus_ParPortFree (ULONG PortIndex);

NTSTATUS PPJoyBus_ParAllocIRQ (ULONG PortIndex, PJOYSTICK_IRQ1 IRQ);
NTSTATUS PPJoyBus_ParFreeIRQ (ULONG PortIndex, PJOYSTICK_IRQ1 IRQ);

NTSTATUS PPJoyBus_ManualSetChipMode (ULONG BaseAddress, IN UCHAR ChipModeFlags);


/* In Power.c */

NTSTATUS PPJoyBus_Power_Bus (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PPJoyBus_Power_Joy (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

PCHAR PowerMinorFunctionString (UCHAR MinorFunction);
PCHAR DbgSystemPowerString (IN SYSTEM_POWER_STATE Type);
PCHAR DbgDevicePowerString (IN DEVICE_POWER_STATE Type);

/* In BusPnP.c */

NTSTATUS PPJoyBus_PnP_Bus (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS PPJoyBus_BusStartDevice (IN PBUS_DEVICE_DATA BusDeviceData, IN PIRP Irp);
NTSTATUS PPJoyBus_BusStopDevice (IN PBUS_DEVICE_DATA BusDeviceData, IN PIRP Irp);
NTSTATUS PPJoyBus_BusRemoveDevice (IN PBUS_DEVICE_DATA BusDeviceData, IN PIRP Irp);
NTSTATUS PPJoyBus_BusQueryRelations (IN PBUS_DEVICE_DATA BusDeviceData, IN PIRP Irp);

PCHAR PnPMinorFunctionString (UCHAR MinorFunction);
PCHAR DbgDeviceRelationString (IN DEVICE_RELATION_TYPE Type);
PCHAR DbgDeviceIDString (BUS_QUERY_ID_TYPE Type);

/* In JoyPnP.c */

NTSTATUS PPJoyBus_PnP_Joy (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS PPJoyBus_JoyQueryIDs (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp);
NTSTATUS PPJoyBus_JoyQueryDevText (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp);
NTSTATUS PPJoyBus_JoyDevRelations (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp);
NTSTATUS PPJoyBus_JoyQueryCaps (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp);
NTSTATUS PPJoyBus_JoyQueryResourceRequirements (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp);

NTSTATUS PPJoyBus_JoyStartDevice (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp);
NTSTATUS PPJoyBus_JoyStopDevice (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp);

/* In EventLog.c */
void PPJoyBus_WriteEventLog (NTSTATUS ErrorMessageCode, PVOID DumpData, USHORT DumpSize, PCWSTR Message);

#pragma pack(pop)

#endif
