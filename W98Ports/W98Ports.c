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
/**  W98Ports.c Interface library for Windows 98. Provides the functions  **/
/**             exported by ParPort.sys on Windows 2000/XP.               **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "W98Ports.h"
#include <stdio.h>


#include <initguid.h>
#include <ntddpar.h>
//DEFINE_GUID(GUID_PARALLEL_DEVICE, 0x97F76EF0, 0xF883, 0x11D0, 0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x34);		// This is TEMP, TEST!!!

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (INIT,DriverEntry)
    #pragma alloc_text (PAGE,W98Ports_CreateClose)
    #pragma alloc_text (PAGE,W98Ports_PnP)
    #pragma alloc_text (PAGE,W98Ports_Power)
    #pragma alloc_text (PAGE,W98Ports_InternalIoctl)
    #pragma alloc_text (PAGE,W98Ports_AddDevice)
    #pragma alloc_text (PAGE,W98Ports_Unload)
#endif


/**************************************************************************/
/* Declare global variables defined in PPortJoy.h and Debug.h             */
/**************************************************************************/
GLOBALS	Globals;
#ifdef TRAP		/* Only declare this if we are doing a checked build */
ULONG	W98Ports_DebugLevel;
#endif


/**************************************************************************/
/* Main entry point for this driver. Setup entry points to other driver   */
/* routines. Do driver-wide initialisation.                               */
/**************************************************************************/
NTSTATUS DriverEntry (IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
 NTSTATUS	ntStatus;
 
 ntStatus= STATUS_SUCCESS;
 W98Ports_DebugLevel= PPJOY_DEFAULT_DEBUGLEVEL;

 PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_WARN, ("Built " __DATE__ " at " __TIME__) );
 PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_FENTRY, ("DriverEntry (DriverObject=0x%x,RegistryPath=0x%x)",DriverObject, RegistryPath) );

 RtlZeroMemory (&Globals,sizeof(Globals));

 /* Allocate buffer to store registry path to the parameters registry key */
 Globals.ParamRegistryPath.MaximumLength= RegistryPath->Length+sizeof(UNICODE_NULL)+sizeof(PARAM_KEY_NAME);
 Globals.ParamRegistryPath.Length= RegistryPath->Length;
 Globals.ParamRegistryPath.Buffer= ExAllocatePoolWithTag (PagedPool,Globals.ParamRegistryPath.MaximumLength,W98PORTS_POOL_TAG);    

 if (!Globals.ParamRegistryPath.Buffer)
 {
  ntStatus= STATUS_INSUFFICIENT_RESOURCES;
  goto Exit;
 }

 /* Copy driver registry path and append the parameters subkey name */
 RtlCopyUnicodeString (&Globals.ParamRegistryPath,RegistryPath);
 RtlAppendUnicodeToString (&Globals.ParamRegistryPath,PARAM_KEY_NAME);

 PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_BABBLE2, ("ParamRegistryPath=%ws",Globals.ParamRegistryPath.Buffer) );

 /* Set up pointers to our other entry points in the DeviceObject */
 DriverObject->MajorFunction[IRP_MJ_CREATE]= W98Ports_CreateClose;
 DriverObject->MajorFunction[IRP_MJ_CLOSE]=	W98Ports_CreateClose;
 DriverObject->MajorFunction[IRP_MJ_POWER]= W98Ports_Power;
 DriverObject->MajorFunction[IRP_MJ_PNP]= W98Ports_PnP;
 DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]= W98Ports_InternalIoctl;
 DriverObject->DriverUnload= W98Ports_Unload;
 DriverObject->DriverExtension->AddDevice= W98Ports_AddDevice;

Exit:
 PPJOY_EXITPROC (FILE_W98PORTS|PPJOY_FEXIT_STATUSOK , "DriverEntry", ntStatus);

 return ntStatus;
} /* DriverEntry */


/**************************************************************************/
/* Routine to handle requests to open and close the bus FDO or the        */
/* joystick PDO.                                                          */
/**************************************************************************/
NTSTATUS W98Ports_CreateClose (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 PIO_STACK_LOCATION	IrpStack;
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_FENTRY, ("W98Ports_CreateClose(DeviceObject=0x%x,Irp=0x%x)",DeviceObject,Irp) );

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
	
 ntStatus= W98Ports_EnterRequest(DeviceObject);
 if (!NT_SUCCESS(ntStatus))
  goto ExitAndComplete;

 /* Return device busy status if our DeviceObject has not been started */
 if (!(DeviceExtension->Flags&W98PFLAGS_STARTED))
 {
  PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_WARN, ("W98Ports_CreateClose: Someone is trying to access us when we are not started") );
  ntStatus= STATUS_DEVICE_NOT_READY;
  goto ExitAndComplete;
 }

 IrpStack = IoGetCurrentIrpStackLocation(Irp);
 ntStatus= STATUS_SUCCESS;

 switch(IrpStack->MajorFunction)
 {
  case IRP_MJ_CREATE:
		PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_BABBLE, ("W98Ports_CreateClose: IRP_MJ_CREATE") );
		Irp->IoStatus.Information = 0;
		break;

  case IRP_MJ_CLOSE:
		PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_BABBLE, ("W98Ports_CreateClose: IRP_MJ_CLOSE") );
		Irp->IoStatus.Information = 0;
		break;

  default:
		PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_WARN, ("W98Ports_CreateClose: Not handled IrpStack->MajorFunction 0x%x",IrpStack->MajorFunction) );
		ntStatus = STATUS_INVALID_PARAMETER;
	    break;
 }

 ntStatus= W98Ports_SendIrpSynchronously (DeviceExtension->NextLowerDriver,Irp);
 if (ntStatus==STATUS_NOT_IMPLEMENTED||ntStatus==STATUS_INVALID_DEVICE_REQUEST)
  ntStatus= STATUS_SUCCESS;

ExitAndComplete:
 W98Ports_LeaveRequest(DeviceObject);
 /* Set status in IRP and complete the request */
 Irp->IoStatus.Status= ntStatus;
 IoCompleteRequest (Irp,IO_NO_INCREMENT);

 PPJOY_EXITPROC (FILE_W98PORTS|PPJOY_FEXIT_STATUSOK, "W98Ports_CreateClose", ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine to route Plug and Play requests for the Bus FDO and the        */
/* Joystick PDO.                                                          */
/**************************************************************************/
NTSTATUS W98Ports_PnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;
 int				MustLeaveRequest;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_FENTRY, ("W98Ports_Pnp(DeviceObject=0x%x,Irp=0x%x)",DeviceObject,Irp) );

 MustLeaveRequest= IoGetCurrentIrpStackLocation(Irp)->MinorFunction!=IRP_MN_REMOVE_DEVICE;

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

 ntStatus= W98Ports_EnterRequest(DeviceObject);
 if (!NT_SUCCESS(ntStatus))
 {
  /* Set status in IRP and complete the request */
  MustLeaveRequest= 1;
  Irp->IoStatus.Status= ntStatus;
  IoCompleteRequest (Irp,IO_NO_INCREMENT);
  goto Exit;
 }

 ntStatus= W98Ports_DoPnP (DeviceObject,Irp);

Exit:
 /* Call leave request if the flag was set (only set for REMOVE_DEVICE  */
 /* (because it called from the Remove request routine.)                */
 if (MustLeaveRequest)
  W98Ports_LeaveRequest(DeviceObject);
 PPJOY_EXITPROC (FILE_W98PORTS|PPJOY_FEXIT_STATUSOK, "W98Ports_PnP",ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine to route Power requests for the Bus FDO and the Joystick PDO.  */
/**************************************************************************/
NTSTATUS W98Ports_Power (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_FENTRY, ("W98Ports_Power(DeviceObject=0x%x,Irp=0x%x)",DeviceObject,Irp) );

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

 ntStatus= W98Ports_EnterRequest(DeviceObject);
 if (!NT_SUCCESS(ntStatus))
 {
  /* Must always call PoStartNextPowerIrp()!!! */
  PoStartNextPowerIrp (Irp);
  /* Set status in IRP and complete the request */
  Irp->IoStatus.Status= ntStatus;
  IoCompleteRequest (Irp,IO_NO_INCREMENT);
  goto Exit;
 }

 ntStatus= W98Ports_DoPower (DeviceObject,Irp);
 
Exit:
 W98Ports_LeaveRequest(DeviceObject);
 PPJOY_EXITPROC (FILE_W98PORTS|PPJOY_FEXIT_STATUSOK, "W98Ports_Power",ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine to route Internal IOCTL requests for the Bus FDO and the       */
/* Joystick PDO.                                                          */
/**************************************************************************/
NTSTATUS W98Ports_InternalIoctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_FENTRY, ("W98Ports_InternalIoctl(DeviceObject=0x%x,Irp=0x%x)",DeviceObject,Irp) );

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

 ntStatus= W98Ports_EnterRequest(DeviceObject);
 if (!NT_SUCCESS(ntStatus))
  goto ExitAndComplete;

 if (!(DeviceExtension->Flags&W98PFLAGS_STARTED))
 {
  ntStatus= STATUS_DEVICE_NOT_READY;
  goto ExitAndComplete;
 }

 ntStatus= W98Ports_DoInternalIoctl (DeviceObject,Irp);
 goto ExitNoComplete;	/* Already completed or passed on to lower driver */

ExitAndComplete:
 /* Set status in IRP and complete the request */
 Irp->IoStatus.Status= ntStatus;
 IoCompleteRequest (Irp,IO_NO_INCREMENT);

ExitNoComplete:
 W98Ports_LeaveRequest(DeviceObject);
 PPJOY_EXITPROC (FILE_W98PORTS|PPJOY_FEXIT_STATUSOK, "W98Ports_InternalIoctl", ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* This routine called when the driver unloads from memory. It should     */
/* any resources allocated by the driver that is not yet freed.           */
/**************************************************************************/
VOID W98Ports_Unload (IN PDRIVER_OBJECT DriverObject)
{
 int	Count;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_FENTRY, ("W98Ports_Unload Enter") );

 /* If we allocated buffer for the registry path, then free is */
 if (Globals.ParamRegistryPath.Buffer)
  ExFreePool(Globals.ParamRegistryPath.Buffer);   

 PPJOY_EXITPROC (FILE_W98PORTS|PPJOY_FEXIT_STATUSOK, "W98Ports_Unload:",STATUS_SUCCESS );
 return;
}

#if 0
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
NTSTATUS PPJoyBus_RegQueryFunc (IN PWSTR ValueName, IN ULONG ValueType, IN PVOID ValueData,
								IN ULONG ValueLength, IN PVOID Context, IN PVOID EntryContext)
{
 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_FENTRY, ("PPJoyBus_RegQueryFunc(ValueName=%ws, ValueType=%d, ValueData= 0x%X, ValueLength= %d, Context= 0x%X, EntryContext= 0x%X)",ValueName,ValueType,ValueData,ValueLength,Context,EntryContext) );

 
 /* Routine MUST return success */
 return STATUS_SUCCESS;
}

#endif

/**************************************************************************/
/* Routine to create Bus FDO objects. Should only be called once.         */
/**************************************************************************/
NTSTATUS W98Ports_AddDevice (IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject)
{
 NTSTATUS					ntStatus;
 PDEVICE_OBJECT				DeviceObject;
 PDEVICE_EXTENSION			DeviceExtension;
 RTL_QUERY_REGISTRY_TABLE	QueryTable[2];
 HANDLE						RegKeyHandle;

 UNICODE_STRING				PortName;
 WCHAR						PortNameBuffer[32];
 int						LPTNumber;

 PAGED_CODE ();

 ntStatus= STATUS_SUCCESS;

 PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_FENTRY, ("W98Ports_AddDevice(DriverObject=0x%x,FunctionalDeviceObject=0x%x)",DriverObject,PhysicalDeviceObject) );

 /* Initialise PortName unicode string */
 PortName.Buffer= PortNameBuffer;
 PortName.MaximumLength= sizeof(PortNameBuffer)/sizeof(WCHAR);
 PortName.Length= 0;

 ntStatus= IoOpenDeviceRegistryKey (PhysicalDeviceObject,PLUGPLAY_REGKEY_DRIVER,GENERIC_READ,&RegKeyHandle);
 if (!NT_SUCCESS (ntStatus)) 
 {
  PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_ERROR, ("W98Ports_AddDevice: IoOpenDeviceRegistryKey failed status= 0x%x)",ntStatus) );
  goto ExitNoDelete;
 }

 /* Create table to query registry. 2nd entry is NULL to teminate the list */
 RtlZeroMemory (QueryTable,sizeof(QueryTable));
 QueryTable->Flags= RTL_QUERY_REGISTRY_DIRECT;
 QueryTable->Name= L"PortName";
 QueryTable->DefaultType= REG_NONE;
 QueryTable->EntryContext= (PVOID) &PortName;

 /* Then execute query. Callback function does all the work. */
 ntStatus= RtlQueryRegistryValues (RTL_REGISTRY_HANDLE,(PWSTR)RegKeyHandle,QueryTable,NULL,NULL);
 ZwClose (RegKeyHandle);
 if (!NT_SUCCESS (ntStatus)) 
 {
  PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_ERROR, ("W98Ports_AddDevice: RtlQueryRegistryValues failed status= 0x%x)",ntStatus) );
  goto ExitNoDelete;
 }

 LPTNumber= 0;
 PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_BABBLE, ("W98Ports_AddDevice: Our port name is %ws",PortNameBuffer) );
 if ((PortNameBuffer[0]==L'L')&&(PortNameBuffer[1]==L'P')&&(PortNameBuffer[2]==L'T'))
 {
  if ((PortNameBuffer[3]>=L'1')&&(PortNameBuffer[3]<=(L'0'+MAX_PARPORTS)))
   LPTNumber= PortNameBuffer[3]- L'0';
 }

 if (!LPTNumber) 
 {
  PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_ERROR, ("W98Ports_AddDevice: Invalid LPT Port name, exiting") );
  goto ExitNoDelete;
 }

 /* Create our FDO device */
 ntStatus= IoCreateDevice (DriverObject,sizeof(DEVICE_EXTENSION),NULL,FILE_DEVICE_BUS_EXTENDER,0,TRUE,&DeviceObject);
 /* Check to see if we could create the device. If not, exit. */
 if (!NT_SUCCESS (ntStatus)) 
 {
  PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_ERROR, ("W98Ports_AddDevice: IoCreateDevice failed status= 0x%x)",ntStatus) );
  goto ExitNoDelete;
 }

 /* Setup our DeviceExtension */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
 RtlZeroMemory (DeviceExtension,sizeof(DEVICE_EXTENSION));

 /* Tell IOManager et al that we are pagable. */
 DeviceObject->Flags|= DO_POWER_PAGABLE;

 /* Attempt to register an interface for this DeviceObject. It will be */
 /* called by the configuration utility to add and remove joysticks.   */
 ntStatus= IoRegisterDeviceInterface (PhysicalDeviceObject,(LPGUID) &GUID_PARALLEL_DEVICE,NULL,&DeviceExtension->InterfaceName);
 /* If we could not register the interface then we exit and delete the DO */
 if (!NT_SUCCESS (ntStatus))
 {
  PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_ERROR, ("W98Ports_AddDevice: IoRegisterDeviceInterface failed status= 0x%x)",ntStatus) );
  goto ExitDeleteDevice;
 }

 PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_BABBLE2, ("W98Ports_AddDevice: IoRegisterDeviceInterface interface name= %ws)",DeviceExtension->InterfaceName.Buffer) );

 /* Now attach ourselves to the device stack. The return value is the next */
 /* lower down Device Object. This is where all the IRPs should be routed. */
 DeviceExtension->NextLowerDriver= IoAttachDeviceToDeviceStack (DeviceObject,PhysicalDeviceObject);
 /* Test if our attach was successful, exit if not. */
 if(!DeviceExtension->NextLowerDriver)
 {
  PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_ERROR, ("W98Ports_AddDevice: IoAttachDeviceToDeviceStack failed status= 0x%x)",ntStatus) );
  ntStatus= STATUS_NO_SUCH_DEVICE;

  goto ExitDeleteDevice;
 }

 /* Save important pointers in our DeviceExtenstion for later use. */
 DeviceExtension->DriverObject= DriverObject;

 /* Initialise per DeviceObject objects */
 KeInitializeEvent (&DeviceExtension->RemoveEvent,SynchronizationEvent,FALSE);


 /* Fill in the value we read from the registry before. */
 DeviceExtension->LPTNumber= LPTNumber;
 swprintf (DeviceExtension->LPTName,L"LPT%d",DeviceExtension->LPTNumber);

//// Get contention handler for this port. Currently does not work. #$%#$%#$%
//// {
////  PFN Function= LCODE__VCOMM_Get_Contention_Handler ("LPT1");
////  PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_BABBLE2, ("Contention handler for LPT1 is 0x%X",Function) );
//// }

 /* Done initializing, clear flag. Should be the final step in AddDevice.  */
 DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
 goto ExitNoDelete;

ExitDeleteDevice:
 IoDeleteDevice (DeviceObject);

ExitNoDelete:
 PPJOY_EXITPROC (FILE_W98PORTS|PPJOY_FEXIT_STATUSOK, "W98Ports_AddDevice",ntStatus);

 return ntStatus;
}


#if 0
void W98Ports_FindParPorts (IN PDRIVER_OBJECT DriverObject)
{
 NTSTATUS			ntStatus;
 int				Count;
 USHORT				*BaseAddr;
 PDEVICE_OBJECT		PortDeviceObject;
 PPORT_DEVICE_DATA	PortDeviceData;


 for (Count=0;Count<MAX_PARPORTS;Count++)
 {
  BaseAddr= 0x408+Count*2;
  PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_BABBLE2, ("Port LPT%d base address 0x%X",Count+1,*BaseAddr) );

  if (*BaseAddr)
  {
   /* Adding the port the our list */
   
   /* Create the PDO for the new parallel port. */
   ntStatus= IoCreateDevice (DriverObject,sizeof(PORT_DEVICE_DATA),NULL,FILE_DEVICE_BUS_EXTENDER,
						     FILE_AUTOGENERATED_DEVICE_NAME,FALSE,&PortDeviceObject);
   /* If we fail to add the PDO then make sure list entry is blank, continue with next port */
   if (!NT_SUCCESS (ntStatus))
   {
    PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_ERROR, ("W98Ports_FindPorts: IoCreateDevice failed status= 0x%x)",ntStatus) );
	continue;
   }
 
   Globals.ParPorts[Count].PDO= PortDeviceObject;

   /* Initialise DeviceExtension for the new Joystick device */
   PortDeviceData= (PPORT_DEVICE_DATA) PortDeviceObject->DeviceExtension;
   RtlZeroMemory (PortDeviceData,sizeof(PORT_DEVICE_DATA));

   PortDeviceData->LPTNumber= Count+1;
   /* Add a pointer to the containing DeviceObject to the DeviceExtension */
   PortDeviceData->Self= PortDeviceObject;
   /* Tell NT the code for this driver is pagable */
   PortDeviceObject->Flags|= DO_POWER_PAGABLE;

   /* After initialisation is completed, clear the flag to say we are done. */
   PortDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

   /* Attempt to register an interface for this DeviceObject. */
   ntStatus= IoRegisterDeviceInterface (PortDeviceObject,(LPGUID) &GUID_PARALLEL_DEVICE,NULL,&PortDeviceData->InterfaceName);
   /* Continue with for loop if we fail to register device interface  */
   if (!NT_SUCCESS (ntStatus))
   {
    PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_ERROR, ("W98Ports_FindPorts: IoRegisterDeviceInterface failed status= 0x%x)",ntStatus) );
    continue;
   }

   PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_BABBLE2, ("W98Ports_FindPorts: IoRegisterDeviceInterface interface name= %ws)",PortDeviceData->InterfaceName.Buffer) );
 
   /* Now try to enable the interface... */
   ntStatus= IoSetDeviceInterfaceState (&PortDeviceData->InterfaceName,TRUE);
   /* Continue with for loop if we fail to enable the device interface  */
   if (!NT_SUCCESS (ntStatus))
   {
    PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_ERROR, ("W98Ports_FindPorts: IoSetDeviceInterfaceState failed status= 0x%x)",ntStatus) );
    continue;
   }

   PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_BABBLE2, ("W98Ports_FindPorts: Interface %ws enabled",PortDeviceData->InterfaceName.Buffer) );

  }
 }
}
#endif