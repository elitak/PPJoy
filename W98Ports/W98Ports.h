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
/**  W98Ports.h #defines, Type declarations and include files for the     **/
/**             Parallel Port support routines driver                     **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#ifndef __W98PORTS_H__
#define __W98PORTS_H__

/**************************************************************************/
/* Standard include files for this driver                                 */
/**************************************************************************/

#include "wdm.h"

#include <branding.h>

#include "Debug.h"

#include <parallel.h>


/**************************************************************************/
/* Configuration #defines for the driver                                  */
/**************************************************************************/

/* Name for the parameters subkey in the registry */
#define PARAM_KEY_NAME			L"\\Parameters"

/* Maximum number of parallel ports that we will cater for. (LPT1-LPTx)   */
#define	MAX_PARPORTS			3

/* Define bit values for the Flags in the Device Extensions               */
#define	W98PFLAGS_STARTED		1
#define	W98PFLAGS_REMOVED		2
#define	W98PFLAGS_ALLOCATED		4



/**************************************************************************/
/* Type definitions for the driver                                        */
/**************************************************************************/

/* Device extension for Bus FDO */
typedef struct
{
 ULONG				Flags;
 ULONG				ReqCount;
 KEVENT				RemoveEvent;
 PDRIVER_OBJECT		DriverObject;		/* Pointer to our Driver */
 PDEVICE_OBJECT		NextLowerDriver;	/* Pointer to next DO */
 UNICODE_STRING		InterfaceName;

 int				LPTNumber;
 WCHAR				LPTName[16];
 USHORT				BaseAddress;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

/* Collection for everything we place in global memory. */
typedef struct
{
 UNICODE_STRING	ParamRegistryPath;
} GLOBALS;



/**************************************************************************/
/* Declare global variables                                               */
/**************************************************************************/

extern GLOBALS Globals;



/**************************************************************************/
/* Function prototypes for all the modules                                */
/**************************************************************************/

/* In PPJoyBus.c: */

NTSTATUS DriverEntry (IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
NTSTATUS W98Ports_CreateClose (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS W98Ports_PnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS W98Ports_Power (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS W98Ports_InternalIoctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS W98Ports_AddDevice (IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject);
VOID W98Ports_Unload (IN PDRIVER_OBJECT DriverObject);

// void W98Ports_FindParPorts (IN PDRIVER_OBJECT DriverObject);

/* In IRPUtilc */
NTSTATUS W98Ports_SendIrpSynchronously (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS W98Ports_CompletionRoutine (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);
NTSTATUS W98Ports_EnterRequest (IN PDEVICE_OBJECT DeviceObject);
NTSTATUS W98Ports_LeaveRequest (IN PDEVICE_OBJECT DeviceObject);

/* In Power.c */

NTSTATUS W98Ports_DoPower (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

PCHAR PowerMinorFunctionString (UCHAR MinorFunction);
PCHAR DbgSystemPowerString (IN SYSTEM_POWER_STATE Type);
PCHAR DbgDevicePowerString (IN DEVICE_POWER_STATE Type);

/* In PnP.c */

NTSTATUS W98Ports_DoPnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS W98Ports_StartDevice  (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS W98Ports_StopDevice  (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS W98Ports_RemoveDevice  (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

PCHAR PnPMinorFunctionString (UCHAR MinorFunction);
PCHAR DbgDeviceRelationString (IN DEVICE_RELATION_TYPE Type);
PCHAR DbgDeviceIDString (BUS_QUERY_ID_TYPE Type);

/* In Ioctl.c */

NTSTATUS W98Ports_DoInternalIoctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS W98Ports_GetParallelPortInfo (IN PDEVICE_OBJECT DeviceObject, IN PPARALLEL_PORT_INFORMATION PortInfo);
NTSTATUS W98Ports_GetParallelPnpInfo (IN PDEVICE_OBJECT DeviceObject, IN PPARALLEL_PNP_INFORMATION PnpInfo);
NTSTATUS W98Ports_ParallelPortAllocate (IN PDEVICE_OBJECT DeviceObject);
NTSTATUS W98Ports_ParallelPortFree (IN PDEVICE_OBJECT DeviceObject);
NTSTATUS W98Ports_ParallelSetChipMode (IN PDEVICE_OBJECT DeviceObject, IN PPARALLEL_CHIP_MODE ChipMode);
NTSTATUS W98Ports_ParallelClearChipMode (IN PDEVICE_OBJECT DeviceObject, IN PPARALLEL_CHIP_MODE ChipMode);

#endif
