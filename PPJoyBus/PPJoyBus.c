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
/**  PPJoyBus.h Initialisation and IRP routing routines for the           **/
/**             Parallel Port Joystick bus enumerator                     **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "PPJoyBus.h"
#include <wchar.h>
#include <initguid.h>
#include "PPJGUID.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (INIT,DriverEntry)
    #pragma alloc_text (PAGE,PPJoyBus_CreateClose)
    #pragma alloc_text (PAGE,PPJoyBus_PnP)
    #pragma alloc_text (PAGE,PPJoyBus_Power)
    #pragma alloc_text (PAGE,PPJoyBus_InternalIoctl)
    #pragma alloc_text (PAGE,PPJoyBus_Ioctl)
    #pragma alloc_text (PAGE,PPJoyBus_AddDevice)
    #pragma alloc_text (PAGE,PPJoyBus_Unload)
/* The following may later be moved to init code??? */
    #pragma alloc_text (PAGE,PPJoyBus_CreateFromRegistry)
    #pragma alloc_text (PAGE,PPJoyBus_RegQueryFunc)
    #pragma alloc_text (PAGE,PPJoyBus_CrackParamString)
    #pragma alloc_text (PAGE,PPJoyBus_GetHexDigit)
#endif



/**************************************************************************/
/* Declare global variables defined in PPortJoy.h and Debug.h             */
/**************************************************************************/
GLOBALS	Globals;
#ifdef TRAP		/* Only declare this if we are doing a checked build */
ULONG	PPJoyBus_DebugLevel;
#endif
ULONG	PPJoy_Opt_RuntimeDebug;
ULONG	PPJoy_Opt_PortFlags;

PPJOY_OPTION_CALLBACK	PPJoyOptionCallback;

/**************************************************************************/
/* Main entry point for this driver. Setup entry points to other driver   */
/* routines. Do driver-wide initialisation.                               */
/**************************************************************************/
NTSTATUS DriverEntry (IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
 NTSTATUS	ntStatus;
 
 ntStatus= STATUS_SUCCESS;
 PPJoyBus_DebugLevel= PPJOY_DEFAULT_DEBUGLEVEL;

 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_WARN, ("Built " __DATE__ " at " __TIME__) );
 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_FENTRY, ("DriverEntry (DriverObject=0x%p,RegistryPath=0x%p)",DriverObject, RegistryPath) );

 RtlZeroMemory (&Globals,sizeof(Globals));

 /* Setup copy of DriverObject first so we can use it for event log function */
 Globals.DriverObject= DriverObject;

 /* Allocate buffer to store registry path to the parameters registry key */
 Globals.ParamRegistryPath.MaximumLength= RegistryPath->Length+sizeof(UNICODE_NULL)+sizeof(PARAM_KEY_NAME);
 Globals.ParamRegistryPath.Length= RegistryPath->Length;
 Globals.ParamRegistryPath.Buffer= ExAllocatePoolWithTag (PagedPool,Globals.ParamRegistryPath.MaximumLength,PPJOYBUS_POOL_TAG);    

 if (!Globals.ParamRegistryPath.Buffer)
 {
  PPJoyBus_WriteEventLog (PPJ_MSG_ERRORALLOCMEM,&ntStatus,sizeof(ntStatus),L"");
  ntStatus= STATUS_INSUFFICIENT_RESOURCES;
  goto Exit;
 }

 /* Copy driver registry path and append the parameters subkey name */
 RtlCopyUnicodeString (&Globals.ParamRegistryPath,RegistryPath);
 RtlAppendUnicodeToString (&Globals.ParamRegistryPath,PARAM_KEY_NAME);

 ExInitializeFastMutex (&Globals.Mutex);

 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_BABBLE2, ("ParamRegistryPath=%S",Globals.ParamRegistryPath.Buffer) );

 /* Set up pointers to our other entry points in the DeviceObject */
 DriverObject->MajorFunction[IRP_MJ_CREATE]= PPJoyBus_CreateClose;
 DriverObject->MajorFunction[IRP_MJ_CLOSE]=	PPJoyBus_CreateClose;
 DriverObject->MajorFunction[IRP_MJ_POWER]= PPJoyBus_Power;
 DriverObject->MajorFunction[IRP_MJ_PNP]= PPJoyBus_PnP;
 DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]= PPJoyBus_Ioctl;
 DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]= PPJoyBus_InternalIoctl;
 DriverObject->DriverUnload= PPJoyBus_Unload;
 DriverObject->DriverExtension->AddDevice= PPJoyBus_AddDevice;

 PPJoyBus_WriteEventLog (PPJ_MSG_DRIVERSTARTEDVER,&ntStatus,sizeof(ntStatus),LVER_PRODUCTVERSION_STR);

Exit:
 PPJOY_EXITPROC (FILE_PPJOYBUS|PPJOY_FEXIT_STATUSOK , "DriverEntry", ntStatus);

 return ntStatus;
} /* DriverEntry */


/**************************************************************************/
/* Routine to handle requests to open and close the bus FDO or the        */
/* joystick PDO.                                                          */
/**************************************************************************/
NTSTATUS PPJoyBus_CreateClose (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 PIO_STACK_LOCATION	IrpStack;
 NTSTATUS			ntStatus;
 int				IsBusDevice;
 PCOMMON_DATA		CommonData;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_FENTRY, ("PPJoyBus_CreateClose(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 /* Get a pointer to common device data */
 CommonData= (PCOMMON_DATA) DeviceObject->DeviceExtension;

 ntStatus= PPJoyBus_EnterRequest(CommonData);
 if (!NT_SUCCESS(ntStatus))
  goto ExitAndComplete;

 /* Return device busy status if our DeviceObject has not been started */
 if (!(CommonData->Flags&PPJFLAGS_STARTED))
 {
  ntStatus= STATUS_DEVICE_NOT_READY;
  goto ExitAndComplete;
 }

 IsBusDevice= CommonData->Flags&PPJFLAGS_ISBUSDEV;
 IrpStack = IoGetCurrentIrpStackLocation(Irp);
 ntStatus= STATUS_SUCCESS;

 switch(IrpStack->MajorFunction)
 {
  case IRP_MJ_CREATE:
		PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_BABBLE, ("PPJoyBus_CreateClose: IRP_MJ_CREATE for %s",IsBusDevice?"Bus":"Joystick") );
		Irp->IoStatus.Information = 0;
		break;

  case IRP_MJ_CLOSE:
		PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_BABBLE, ("PPJoyBus_CreateClose: IRP_MJ_CLOSE for %s",IsBusDevice?"Bus":"Joystick") );
		Irp->IoStatus.Information = 0;
		break;

  default:
		PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_WARN, ("PPJoyBus_CreateClose: Not handled IrpStack->MajorFunction 0x%x for %s",IrpStack->MajorFunction,IsBusDevice?"Bus":"Joystick") );
		ntStatus = STATUS_INVALID_PARAMETER;
	    break;
 }

 /* Save status and complete IRP for Joystick, pass to PDO on for bus */
 if (IsBusDevice)
 {
  ntStatus= PPJoyBus_SendIrpSynchronously (((PBUS_DEVICE_DATA)CommonData)->NextLowerDriver,Irp);
  if (ntStatus==STATUS_NOT_IMPLEMENTED||ntStatus==STATUS_INVALID_DEVICE_REQUEST)
   ntStatus= STATUS_SUCCESS;
 }

ExitAndComplete:
 PPJoyBus_LeaveRequest(CommonData);
 /* Set status in IRP and complete the request */
 Irp->IoStatus.Status= ntStatus;
 IoCompleteRequest (Irp,IO_NO_INCREMENT);

 PPJOY_EXITPROC (FILE_PPJOYBUS|PPJOY_FEXIT_STATUSOK, "PPJoyBus_CreateClose", ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine to route Plug and Play requests for the Bus FDO and the        */
/* Joystick PDO.                                                          */
/**************************************************************************/
NTSTATUS PPJoyBus_PnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS		ntStatus;
 PCOMMON_DATA	CommonData;
 int			MustLeaveRequest;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_FENTRY, ("PPJoyBus_Pnp(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 MustLeaveRequest= IoGetCurrentIrpStackLocation(Irp)->MinorFunction!=IRP_MN_REMOVE_DEVICE;
 CommonData= DeviceObject->DeviceExtension;

 ntStatus= PPJoyBus_EnterRequest(CommonData);
 if (!NT_SUCCESS(ntStatus))
 {
  /* Set status in IRP and complete the request */
  MustLeaveRequest= 1;
  Irp->IoStatus.Status= ntStatus;
  IoCompleteRequest (Irp,IO_NO_INCREMENT);
  goto Exit;
 }

 if (CommonData->Flags&PPJFLAGS_ISBUSDEV)
  ntStatus= PPJoyBus_PnP_Bus (DeviceObject,Irp);
 else
  ntStatus= PPJoyBus_PnP_Joy (DeviceObject,Irp);


Exit:
 /* Call leave request if the flag was set (only set for REMOVE_DEVICE  */
 /* (because it called from the Remove request routine.)                */
 if (MustLeaveRequest)
  PPJoyBus_LeaveRequest(CommonData);
 PPJOY_EXITPROC (FILE_PPJOYBUS|PPJOY_FEXIT_STATUSOK, "PPJoyBus_PnP",ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine to route Power requests for the Bus FDO and the Joystick PDO.  */
/**************************************************************************/
NTSTATUS PPJoyBus_Power (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS		ntStatus;
 PCOMMON_DATA	CommonData;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_FENTRY, ("PPJoyBus_Power(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 CommonData= DeviceObject->DeviceExtension;

 ntStatus= PPJoyBus_EnterRequest(CommonData);
 if (!NT_SUCCESS(ntStatus))
 {
  /* Must always call PoStartNextPowerIrp()!!! */
  PoStartNextPowerIrp (Irp);
  /* Set status in IRP and complete the request */
  Irp->IoStatus.Status= ntStatus;
  IoCompleteRequest (Irp,IO_NO_INCREMENT);
  goto Exit;
 }

 if (CommonData->Flags&PPJFLAGS_ISBUSDEV)
  ntStatus= PPJoyBus_Power_Bus (DeviceObject,Irp);
 else
  ntStatus= PPJoyBus_Power_Joy (DeviceObject,Irp);
 
Exit:
 PPJoyBus_LeaveRequest(CommonData);
 PPJOY_EXITPROC (FILE_PPJOYBUS|PPJOY_FEXIT_STATUSOK, "PPJoyBus_Power",ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine to route Internal IOCTL requests for the Bus FDO and the       */
/* Joystick PDO.                                                          */
/**************************************************************************/
NTSTATUS PPJoyBus_InternalIoctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PCOMMON_DATA			CommonData;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_FENTRY, ("PPJoyBus_InternalIoctl(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 CommonData= DeviceObject->DeviceExtension;

 ntStatus= PPJoyBus_EnterRequest(CommonData);
 if (!NT_SUCCESS(ntStatus))
  goto ExitAndComplete;

 if (!(CommonData->Flags&PPJFLAGS_STARTED))
 {
  ntStatus= STATUS_DEVICE_NOT_READY;
  goto ExitAndComplete;
 }

 if (CommonData->Flags&PPJFLAGS_ISBUSDEV)
  ntStatus= PPJoyBus_InternalIoctl_Bus (DeviceObject,Irp);
 else
  ntStatus= PPJoyBus_InternalIoctl_Joy (DeviceObject,Irp);
 goto ExitNoComplete;	/* Already completed or passed on to lower driver */

ExitAndComplete:
 /* Set status in IRP and complete the request */
 Irp->IoStatus.Status= ntStatus;
 IoCompleteRequest (Irp,IO_NO_INCREMENT);

ExitNoComplete:
 PPJoyBus_LeaveRequest(CommonData);
 PPJOY_EXITPROC (FILE_PPJOYBUS|PPJOY_FEXIT_STATUSOK, "PPJoyBus_InternalIoctl", ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine to route IOCTL requests for the Bus FDO and the Joystick PDO   */
/**************************************************************************/
NTSTATUS PPJoyBus_Ioctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PCOMMON_DATA			CommonData;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_FENTRY, ("PPJoyBus_Ioctl(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 CommonData= DeviceObject->DeviceExtension;

 ntStatus= PPJoyBus_EnterRequest(CommonData);
 if (!NT_SUCCESS(ntStatus))
  goto ExitAndComplete;

 if (!(CommonData->Flags&PPJFLAGS_STARTED))
 {
  ntStatus= STATUS_DEVICE_NOT_READY;
  goto ExitAndComplete;
 }

 if (CommonData->Flags&PPJFLAGS_ISBUSDEV)
  ntStatus= PPJoyBus_Ioctl_Bus (DeviceObject,Irp);
 else
  ntStatus= PPJoyBus_Ioctl_Joy (DeviceObject,Irp);
 goto ExitNoComplete;	/* Already completed or passed on to lower driver */

ExitAndComplete:
 /* Set status in IRP and complete the request */
 Irp->IoStatus.Status= ntStatus;
 IoCompleteRequest (Irp,IO_NO_INCREMENT);

ExitNoComplete:
 PPJoyBus_LeaveRequest(CommonData);
 PPJOY_EXITPROC (FILE_PPJOYBUS|PPJOY_FEXIT_STATUSOK, "PPJoyBus_Ioctl", ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* This routine called when the driver unloads from memory. It should     */
/* any resources allocated by the driver that is not yet freed.           */
/**************************************************************************/
VOID PPJoyBus_Unload (IN PDRIVER_OBJECT DriverObject)
{
 int	Count;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_FENTRY, ("PPJoyBus_Unload Enter") );

 /* If we allocated buffer for the registry path, then free is */
 if (Globals.ParamRegistryPath.Buffer)
  ExFreePool(Globals.ParamRegistryPath.Buffer);   

 /* Release the FileObj reference on all the parallel ports we have opened */
 /* in the PnP interface notification routine. Also free the buffer for    */
 /* the device SymbolicLinkName, if allocated.                             */
 for (Count=0;Count<MAX_PARPORTS;Count++)
 {
  if (Globals.ParPorts[Count].FileObj)
   ObDereferenceObject (Globals.ParPorts[Count].FileObj);

  if (Globals.ParPorts[Count].SymLinkName.Buffer)
   ExFreePool(Globals.ParPorts[Count].SymLinkName.Buffer);   
 }

 PPJOY_EXITPROC (FILE_PPJOYBUS|PPJOY_FEXIT_STATUSOK, "PPJoyBus_Unload:",STATUS_SUCCESS );
 return;
}

/**************************************************************************/
/* Routine to create Bus FDO objects. Should only be called once.         */
/**************************************************************************/
NTSTATUS PPJoyBus_AddDevice (IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject)
{
 NTSTATUS			ntStatus;
 PDEVICE_OBJECT		DeviceObject;
 PBUS_DEVICE_DATA	BusDeviceData;

 PAGED_CODE ();

 ntStatus= STATUS_SUCCESS;

 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_FENTRY, ("PPJoyBus_AddDevice(DriverObject=0x%p,FunctionalDeviceObject=0x%p)",DriverObject,PhysicalDeviceObject) );

 /* Create our FDO device */
 ntStatus= IoCreateDevice (DriverObject,sizeof(BUS_DEVICE_DATA),NULL,FILE_DEVICE_BUS_EXTENDER,FILE_DEVICE_SECURE_OPEN,TRUE,&DeviceObject);
 /* Check to see if we could create the device. If not, exit. */
 if (!NT_SUCCESS (ntStatus)) 
 {
  PPJoyBus_WriteEventLog (PPJ_MSG_ERRORCREATINGBUS,&ntStatus,sizeof(ntStatus),L"");
  PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_ERROR, ("PPJoyBus_AddDevice: IoCreateDevice failed status= 0x%x)",ntStatus) );
  goto ExitNoDelete;
 }

 /* Setup our DeviceExtension */
 BusDeviceData= (PBUS_DEVICE_DATA) DeviceObject->DeviceExtension;
 RtlZeroMemory (BusDeviceData,sizeof(BUS_DEVICE_DATA));
 BusDeviceData->Flags|= PPJFLAGS_ISBUSDEV;

 /* Tell IOManager et al that we are pagable. */
 DeviceObject->Flags|= DO_POWER_PAGABLE;

 /* Attempt to register an interface for this DeviceObject. It will be */
 /* called by the configuration utility to add and remove joysticks.   */
 ntStatus= IoRegisterDeviceInterface (PhysicalDeviceObject,(LPGUID) &GUID_PPJOY_BUS,NULL,&BusDeviceData->InterfaceName);
 /* If we could not register the interface then we exit and delete the DO */
 if (!NT_SUCCESS (ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_ERROR, ("PPJoyBus_AddDevice: IoRegisterDeviceInterface failed status= 0x%x)",ntStatus) );
  PPJoyBus_WriteEventLog (PPJ_MSG_ERRORBUSIF,&ntStatus,sizeof(ntStatus),L"");
  goto ExitDeleteDevice;
 }

 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_BABBLE2, ("PPJoyBus_AddDevice: IoRegisterDeviceInterface interface name= %S)",BusDeviceData->InterfaceName.Buffer) );
 
 /* Now attach ourselves to the device stack. The return value is the next */
 /* lower down Device Object. This is where all the IRPs should be routed. */
 BusDeviceData->NextLowerDriver= IoAttachDeviceToDeviceStack (DeviceObject,PhysicalDeviceObject);
 /* Test if our attach was successful, exit if not. */
 if(!BusDeviceData->NextLowerDriver)
 {
  PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_ERROR, ("PPJoyBus_AddDevice: IoAttachDeviceToDeviceStack failed status= 0x%x)",ntStatus) );
  ntStatus= STATUS_NO_SUCH_DEVICE;

  PPJoyBus_WriteEventLog (PPJ_MSG_ERRORBUSATTACH,&ntStatus,sizeof(ntStatus),L"");
  RtlFreeUnicodeString (&BusDeviceData->InterfaceName);
  goto ExitDeleteDevice;
 }

 /* Save important pointers in our DeviceExtenstion for later use. */
 BusDeviceData->DriverObject= DriverObject;
 BusDeviceData->Self= DeviceObject;
 BusDeviceData->UnderlyingPDO= PhysicalDeviceObject;

 /* Initialise per DeviceObject objects */
 InitializeListHead (&BusDeviceData->JoystickList);
 KeInitializeEvent (&BusDeviceData->RemoveEvent,SynchronizationEvent,FALSE);

 /* Read configuration options from the registry */
 PPJoyBus_ReadOptionsFromReg (DeviceObject);

 /* Read persistent joystick device data from the registry and create PDOs */
 /* This call can possibly move to the DriverEntry routine???              */
 PPJoyBus_CreateFromRegistry (DeviceObject);

 /* Done initializing, clear flag. Should be the final step in AddDevice.  */
 DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
 goto ExitNoDelete;

ExitDeleteDevice:
 IoDeleteDevice (DeviceObject);

ExitNoDelete:
 PPJOY_EXITPROC (FILE_PPJOYBUS|PPJOY_FEXIT_STATUSOK, "PPJoyBus_AddDevice",ntStatus);

 return ntStatus;
}

/**************************************************************************/
/* Read all configured joysticks from the registry and add PDOs for them  */
/**************************************************************************/
VOID PPJoyBus_CreateFromRegistry (IN PDEVICE_OBJECT DeviceObject)
{
 NTSTATUS					ntStatus;
 RTL_QUERY_REGISTRY_TABLE	QueryTable[2];
 
 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_FENTRY, ("PPJoyBus_CreateFromRegistry(DeviceObject=0x%p)",DeviceObject) );

 /* Create table to query registry. 2nd entry is NULL to teminate the list */
 RtlZeroMemory (QueryTable,sizeof(QueryTable));
 QueryTable->Flags= RTL_QUERY_REGISTRY_REQUIRED;
 QueryTable->QueryRoutine= PPJoyBus_RegQueryFunc;

 /* Then execute query. Callback function does all the work. */
 ntStatus= RtlQueryRegistryValues (RTL_REGISTRY_ABSOLUTE,Globals.ParamRegistryPath.Buffer,QueryTable,(PVOID)DeviceObject,NULL);

 PPJOY_EXITPROC (FILE_PPJOYBUS|PPJOY_FEXIT_STATUSOK, "PPJoyBus_CreateFromRegistry:",ntStatus );
 return;
}

/**************************************************************************/
/* Callback function for CreateFromRegistry. Does all the hard work.      */
/**************************************************************************/
NTSTATUS PPJoyBus_RegQueryFunc (IN PWSTR ValueName, IN ULONG ValueType, IN PVOID ValueData,
								IN ULONG ValueLength, IN PVOID Context, IN PVOID EntryContext)
{
 NTSTATUS	ntStatus;
 UCHAR		LPTNumber;
 UCHAR		JoyType;
 UCHAR		UnitNumber;

 PAGED_CODE ();


 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_FENTRY, ("PPJoyBus_RegQueryFunc(ValueName=%S, ValueType=%d, ValueData= 0x%p, ValueLength= %d, Context= 0x%p, EntryContext= 0x%p)",ValueName,ValueType,ValueData,ValueLength,Context,EntryContext) );

 /* Check for and exclude values for PPJoyBus options */
 if (!_wcsicmp(ValueName,PPJOY_OPTNAME_RUNTIMEDEBUG))
  goto ExitOK;
 if (!_wcsicmp(ValueName,PPJOY_OPTNAME_PORTFLAGS))
  goto ExitOK;

 /* Check to see if ValueName conforms to our format */
 if (ValueType!=REG_BINARY)
 {
  PPJOY_DBGPRINT (FILE_PPJOYBUS | PPJOY_WARN, ("Invalid registry data type, ignoring entry %S",ValueName) );
  goto Exit;
 }

 if (ValueLength!=sizeof(JOYSTICK_CONFIG1))
 {
  PPJOY_DBGPRINT (FILE_PPJOYBUS | PPJOY_WARN, ("Invalid registry data size, ignoring entry %S",ValueName) );
  goto Exit;
 }

 if ( ((PJOYSTICK_CONFIG1)ValueData)->Size!=sizeof(JOYSTICK_CONFIG1))
 {
  PPJOY_DBGPRINT (FILE_PPJOYBUS | PPJOY_WARN, ("Invalid registry data - size is wrong, ignoring entry %S",ValueName) );
  goto Exit;
 }

 if (!PPJoyBus_CrackParamString(ValueName,&LPTNumber,&JoyType,&UnitNumber))
 {
  PPJOY_DBGPRINT (FILE_PPJOYBUS | PPJOY_WARN, ("Invalid registry data - cannot parse name to LPT+Type+Unit, ignoring entry %S",ValueName) );
  goto Exit;
 }

 if (((PJOYSTICK_CONFIG1)ValueData)->LPTNumber!=LPTNumber)
 {
  PPJOY_DBGPRINT (FILE_PPJOYBUS | PPJOY_WARN, ("Invalid registry data - LPTNumber mismatch, ignoring entry %S",ValueName) );
  goto Exit;
 }

 if (((PJOYSTICK_CONFIG1)ValueData)->JoyType!=JoyType)
 {
  PPJOY_DBGPRINT (FILE_PPJOYBUS | PPJOY_WARN, ("Invalid registry data - JoyType mismatch, ignoring entry %S",ValueName) );
  goto Exit;
 }

 if (((PJOYSTICK_CONFIG1)ValueData)->UnitNumber!=UnitNumber)
 {
  PPJOY_DBGPRINT (FILE_PPJOYBUS | PPJOY_WARN, ("Invalid registry data - UnitNumber mismatch, ignoring entry %S",ValueName) );
  goto Exit;
 }

 /* Entry looks OK. Call AddJoystick routine to create and setup the PDO.  */
 PPJoyBus_AddJoystick ((PDEVICE_OBJECT)Context,(PJOYSTICK_CONFIG1)ValueData,0);
 goto ExitOK;

Exit:
  PPJoyBus_WriteEventLog (PPJ_MSG_ERRORJOYREGISTRY,&ntStatus,sizeof(ntStatus),ValueName);

ExitOK:
 /* Routine MUST return success */
 ntStatus= STATUS_SUCCESS;

 PPJOY_EXITPROC (FILE_PPJOYBUS | PPJOY_FEXIT_STATUSOK, "PPJoyBus_RegQueryFunc:", ntStatus );
 return ntStatus;
}


/**************************************************************************/
/* Routine to break string into Joystick parameters. We expect the string */
/* to be in the format pXXtXXuXX where XX is a hex value. Anything else   */
/* is an error.                                                           */
/**************************************************************************/
ULONG PPJoyBus_CrackParamString (IN PWSTR ParamString, UCHAR *LPTNumber, UCHAR *JoyType, UCHAR *UnitNumber)
{
 UCHAR	Digit1;
 UCHAR	Digit2;

 PAGED_CODE ();

 if (*(ParamString++)!=L'p')
  return 0;

 if (!PPJoyBus_GetHexDigit(*(ParamString++),&Digit1))
  return 0;
 if (!PPJoyBus_GetHexDigit(*(ParamString++),&Digit2))
  return 0;

 *LPTNumber= Digit1<<4|Digit2;


 if (*(ParamString++)!=L't')
  return 0;

 if (!PPJoyBus_GetHexDigit(*(ParamString++),&Digit1))
  return 0;
 if (!PPJoyBus_GetHexDigit(*(ParamString++),&Digit2))
  return 0;

 *JoyType= Digit1<<4|Digit2;


 if (*(ParamString++)!=L'u')
  return 0;

 if (!PPJoyBus_GetHexDigit(*(ParamString++),&Digit1))
  return 0;
 if (!PPJoyBus_GetHexDigit(*(ParamString++),&Digit2))
  return 0;

 *UnitNumber= (Digit1<<4|Digit2) - 1;

 return 1;
}

/**************************************************************************/
/* Routine to convert hex digit into an integer. Function return 1 on     */
/* success and 0 in case of error.                                        */
/**************************************************************************/
ULONG PPJoyBus_GetHexDigit (IN WCHAR Char, OUT UCHAR *Value)
{
 PAGED_CODE ();

 if ((Char>='0')&&(Char<='9'))
 {
  *Value= Char-'0';
  return 1;
 }

 if ((Char>='a')&&(Char<='f'))
 {
  *Value= Char-'a'+10;
  return 1;
 }

 if ((Char>='A')&&(Char<='F'))
 {
  *Value= Char-'A'+10;
  return 1;
 }

 return 0;
}

VOID PPJoyBus_ReadOptionsFromReg (IN PDEVICE_OBJECT DeviceObject)
{
 NTSTATUS					ntStatus;
 RTL_QUERY_REGISTRY_TABLE	QueryTable[3];
 ULONG						DefaultOptionValue= 0;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_PPJOYBUS|PPJOY_FENTRY, ("PPJoyBus_ReadOptionsFromReg(DeviceObject=0x%p)",DeviceObject) );

 /* Create table to query registry. 3rd entry is NULL to teminate the list */
 RtlZeroMemory (QueryTable,sizeof(QueryTable));

 QueryTable[0].QueryRoutine= NULL;
 QueryTable[0].Flags= RTL_QUERY_REGISTRY_DIRECT;
 QueryTable[0].Name= PPJOY_OPTNAME_RUNTIMEDEBUG;
 QueryTable[0].EntryContext= &PPJoy_Opt_RuntimeDebug;
 QueryTable[0].DefaultType=  REG_DWORD;
 QueryTable[0].DefaultData= &DefaultOptionValue;
 QueryTable[0].DefaultLength= sizeof (DefaultOptionValue);

 QueryTable[1].QueryRoutine= NULL;
 QueryTable[1].Flags= RTL_QUERY_REGISTRY_DIRECT;
 QueryTable[1].Name= PPJOY_OPTNAME_PORTFLAGS;
 QueryTable[1].EntryContext= &PPJoy_Opt_PortFlags;
 QueryTable[1].DefaultType=  REG_DWORD;
 QueryTable[1].DefaultData= &DefaultOptionValue;
 QueryTable[1].DefaultLength= sizeof (DefaultOptionValue);

 /* Then execute query. Callback function does all the work. */
 ntStatus= RtlQueryRegistryValues (RTL_REGISTRY_ABSOLUTE,Globals.ParamRegistryPath.Buffer,QueryTable,(PVOID)DeviceObject,NULL);

 if (PPJoyOptionCallback)
 {
  PPJoyOptionCallback (PPJOY_OPTION_RUNTIMEDEBUG,PPJoy_Opt_RuntimeDebug);
  PPJoyOptionCallback (PPJOY_OPTION_PORTFLAGS,PPJoy_Opt_PortFlags);
 }

 PPJOY_EXITPROC (FILE_PPJOYBUS|PPJOY_FEXIT_STATUSOK, "PPJoyBus_ReadOptionsFromReg:",ntStatus );
 return;
}
