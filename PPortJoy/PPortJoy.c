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
/**  PPortJoy.c Initialisation and unload code for the Parallel Port      **/
/**             Joystick FDO driver.                                      **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "PPortJoy.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text( INIT, DriverEntry )
    #pragma alloc_text( PAGE, PPJoy_AddDevice)
    #pragma alloc_text( PAGE, PPJoy_Unload)
    #pragma alloc_text( PAGE, PPJoy_Ctl_CreateClose)
#endif



/**************************************************************************/
/* Declare global variables defined in PPortJoy.h and Debug.h             */
/**************************************************************************/
PPORTJOY_GLOBAL	Globals;
ULONG			PPJoy_RuntimeDebug;
#ifdef TRAP		/* Only declare this if we are doing a checked build */
ULONG				PPJoy_DebugLevel= PPJOY_DEFAULT_DEBUGLEVEL;
#endif
/* Keep a copy of HID dispatch pointers - before we override them... */
/* Right back at ya, hidclass!                                       */
PDRIVER_DISPATCH	HIDMajorFunctions[IRP_MJ_MAXIMUM_FUNCTION+1];


/**************************************************************************/
/* Main entry point for this driver. Register with HID Class driver and   */
/* do driver-wide initialisation.                                         */
/**************************************************************************/
NTSTATUS DriverEntry (IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
 NTSTATUS						ntStatus;
 HID_MINIDRIVER_REGISTRATION	hidMinidriverRegistration;

 PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_WARN, ("Built " __DATE__ " at " __TIME__) );
 PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_FENTRY, ("DriverEntry (DriverObject=0x%p,RegistryPath=0x%p)",DriverObject,RegistryPath) );

 /* First setup global parameters memory structure  */
 RtlZeroMemory(&Globals, sizeof(Globals));

 /* Setup copy of DriverObject first so we can use it for event log function */
 Globals.DriverObject= DriverObject;

 /* Allocate buffer to store registry path to the parameters registry key */
 Globals.ParamRegistryPath.MaximumLength= RegistryPath->Length+sizeof(UNICODE_NULL)+sizeof(PARAM_KEY_NAME);
 Globals.ParamRegistryPath.Length= RegistryPath->Length;
 Globals.ParamRegistryPath.Buffer= ExAllocatePoolWithTag (PagedPool,Globals.ParamRegistryPath.MaximumLength,PPJOY_POOL_TAG);    

 if (!Globals.ParamRegistryPath.Buffer)
 {
  ntStatus= STATUS_INSUFFICIENT_RESOURCES; 
  PPortJoy_WriteEventLog (PPJ_MSG_ERRORALLOCMEM,&ntStatus,sizeof(ntStatus),L"");
  goto Exit;
 }

 /* Copy driver registry path and append the parameters subkey name */
 RtlCopyUnicodeString (&Globals.ParamRegistryPath,RegistryPath);
 RtlAppendUnicodeToString (&Globals.ParamRegistryPath,PARAM_KEY_NAME);

#ifdef USE_SCAN_THREAD
 ntStatus= PPJoy_StartScanThread();
 if (!NT_SUCCESS (ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_ERROR, ("Error creating async joystick read thread. ntStatus(0x%x)",ntStatus) );
  PPortJoy_WriteEventLog (PPJ_MSG_ERRORTHREAD,&ntStatus,sizeof(ntStatus),L"");
  return ntStatus;
 }
#endif

 /* Set up pointers to our other entry points in the DeviceObject */
 DriverObject->DriverUnload= PPJoy_Unload;
 DriverObject->DriverExtension->AddDevice= PPJoy_AddDevice;

 DriverObject->MajorFunction[IRP_MJ_PNP]= PPJoy_PnP;
 DriverObject->MajorFunction[IRP_MJ_POWER]= PPJoy_Power;
 DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]= PPJoy_InternalIoctl;

 /* Setup HID registration structure */
 RtlZeroMemory(&hidMinidriverRegistration, sizeof(hidMinidriverRegistration));

 hidMinidriverRegistration.DeviceExtensionSize= sizeof(DEVICE_EXTENSION);
 hidMinidriverRegistration.Revision= HID_REVISION;
 hidMinidriverRegistration.DriverObject= DriverObject;
 hidMinidriverRegistration.RegistryPath= RegistryPath;
 //hidMinidriverRegistration.DevicesArePolled= FALSE;		// Setting False makes biiiiig mess.
 hidMinidriverRegistration.DevicesArePolled= TRUE;

 PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_BABBLE2, ("DeviceExtensionSize= %d",hidMinidriverRegistration.DeviceExtensionSize) );

 /* Register  with HID.SYS module */
 ntStatus= HidRegisterMinidriver(&hidMinidriverRegistration);
 PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_BABBLE2, ("Registered with HID.SYS, returnCode=%x",ntStatus) );
 if (!NT_SUCCESS (ntStatus))
 {
  PPortJoy_WriteEventLog (PPJ_MSG_ERRORHIDREG,&ntStatus,sizeof(ntStatus),L"");
  goto Exit;
 }
 
 /* Make a copy of the dispatch table, after hid.sys modified it */
 RtlCopyMemory (HIDMajorFunctions,DriverObject->MajorFunction,sizeof(HIDMajorFunctions));

 /* Now we modify it back to our routines :-) */
 DriverObject->MajorFunction[IRP_MJ_CREATE]= PPJoy_Ctl_CreateClose;
 DriverObject->MajorFunction[IRP_MJ_CLOSE]= PPJoy_Ctl_CreateClose;
 DriverObject->MajorFunction[IRP_MJ_PNP]= PPJoy_Ctl_PnP;
 DriverObject->MajorFunction[IRP_MJ_POWER]= PPJoy_Ctl_Power;
 DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]= PPJoy_Ctl_InternalIoctl;
 DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]= PPJoy_Ctl_Ioctl;

 /* Note, there is no "un-Initialise" routine for the SpinLock */
 KeInitializeSpinLock (&Globals.SpinLock);
 
 PPortJoy_WriteEventLog (PPJ_MSG_DRIVERSTARTEDVER,&ntStatus,sizeof(ntStatus),LVER_PRODUCTVERSION_STR);

Exit:
 PPJOY_EXITPROC (FILE_PPORTJOY|PPJOY_FEXIT_STATUSOK, "DriverEntry",ntStatus);

 return ntStatus;
}

/**************************************************************************/
/* Routine to handle Open and Close IRPs. We simply complete them with no */
/* error. Don't think it gets called for HID Minidrivers.                 */
/**************************************************************************/
NTSTATUS PPJoy_Ctl_CreateClose (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;
 int				Count;


 PAGED_CODE ();

 /* If it is not for one of our control devices - pass on to HID.sys */
 if (GET_NEXT_DEVICE_OBJECT(DeviceObject))
 {
  PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_HIDHACK, ("Not a control device - passing request on to HID.sys (PPJoy_Ctl_CreateClose)") );
  return HIDMajorFunctions[IoGetCurrentIrpStackLocation(Irp)->MajorFunction](DeviceObject,Irp);
 }

 PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_FENTRY, ("PPJoy_Ctl_CreateClose(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 DeviceExtension= GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

 ntStatus= PPJoy_EnterRequest (DeviceExtension);
 if (!NT_SUCCESS(ntStatus))
  goto Exit;


 switch(IoGetCurrentIrpStackLocation(Irp)->MajorFunction)
 {
  case IRP_MJ_CREATE:
		DeviceExtension->OpenCount++;
		PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_BABBLE, ("PPJoy_CreateClose:IRP_MJ_CREATE - open count %d",DeviceExtension->OpenCount) );
		Irp->IoStatus.Information= 0;
		break;

  case IRP_MJ_CLOSE:
		if (DeviceExtension->OpenCount)
		 DeviceExtension->OpenCount--;
		PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_BABBLE, ("PPJoy_CreateClose:IRP_MJ_CLOSE - open count %d",DeviceExtension->OpenCount) );
		if ((!(DeviceExtension->OpenCount))&&(DeviceExtension->Config.JoyType==IF_IOCTL))
		{
		 /* Last open handle the Virtual stick closed. Reset device to idle state */
		 PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_BABBLE2, ("PPJoy_CreateClose: reseting virtual joystick position") );
         RtlZeroMemory (&(DeviceExtension->RawInput),sizeof(DeviceExtension->RawInput));
         for (Count=0;Count<MAX_ANALOG_RAW;Count++)
          DeviceExtension->RawInput.Analog[Count]= (PPJOY_AXIS_MIN+PPJOY_AXIS_MAX)/2;	/* Centre */
		}
		Irp->IoStatus.Information= 0;
		break;

  default:
		PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_WARN, ("PPJoy_CreateClose:Not handled IrpStack->MajorFunction 0x%x",IoGetCurrentIrpStackLocation(Irp)->MajorFunction) );
		ntStatus= STATUS_INVALID_PARAMETER;
		break;
 }

Exit:
 /* Set the return status for operation in the IRP */
 Irp->IoStatus.Status= ntStatus;
 PPJoy_LeaveRequest	(DeviceExtension);

 /* Complete the IRP */
 IoCompleteRequest (Irp,IO_NO_INCREMENT);

 PPJOY_EXITPROC (FILE_PPORTJOY | PPJOY_FEXIT_STATUSOK, "PPJoy_Ctl_CreateClose", ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* This routine is called (by the HID Class driver) each time a new       */
/* Parallel Port Joystick PDO is found by the PnP Manager.                */
/**************************************************************************/
NTSTATUS PPJoy_AddDevice (IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT FunctionalDeviceObject)
{
 NTSTATUS			ntStatus;
 PDEVICE_OBJECT		DeviceObject;
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_FENTRY, ("PPJoy_AddDevice(DriverObject=0x%p,FunctionalDeviceObject=0x%p)",DriverObject,FunctionalDeviceObject) );

 ntStatus= STATUS_SUCCESS;
 DeviceObject= FunctionalDeviceObject;
 DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

 /* This is where we initialise the DeviceExtension for each joystick */
 RtlZeroMemory (DeviceExtension,sizeof(DEVICE_EXTENSION));
 KeInitializeEvent (&DeviceExtension->RemoveEvent,SynchronizationEvent,FALSE);
 DeviceExtension->ActiveMap= 0;

 Globals.NumPresentJoys++;
 PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_BABBLE, ("PPJoy_AddDevice: Current number of joystick objects are now %d",Globals.NumPresentJoys) );

 /* Tell Io Manager we are done initialising */
 DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

 PPJOY_EXITPROC (FILE_PPORTJOY|PPJOY_FEXIT_STATUSOK, "PPJoy_AddDevice", ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Unload routine. Here we should clean up any driver-global resources    */
/* before we are unloaded. Currently we have nothing to clear up.         */
/**************************************************************************/
VOID PPJoy_Unload (IN PDRIVER_OBJECT DriverObject)
{
 PAGED_CODE();
 
 PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_FENTRY, ("PPJoy_Unload Enter") );

 ////////////////////////////////////////////////////////////////////////////////
 //// Should probably add code here to make sure all our devices are deleted! ///
 //// also problably delete all sym links we could have created               ///
 ////////////////////////////////////////////////////////////////////////////////

 PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_WARN, ("Unloading PPortJoy...") );

#ifdef USE_SCAN_THREAD
 Globals.MustExit= 1;

 if (Globals.ThreadObject)
 {
  // wait for the thread to die
  KeWaitForSingleObject(Globals.ThreadObject,Executive,KernelMode,FALSE,NULL);
        
  // allow the system to release the thread object
  ObDereferenceObject(Globals.ThreadObject);
  Globals.ThreadObject= NULL;
 }
#endif

 /* If we allocated buffer for the registry path, then free is */
 if (Globals.ParamRegistryPath.Buffer)
  ExFreePool(Globals.ParamRegistryPath.Buffer);   

 PPJOY_EXITPROC (FILE_PPORTJOY | PPJOY_FEXIT_STATUSOK, "PPJoy_Unload:", STATUS_SUCCESS );
 return;
}
