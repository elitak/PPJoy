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


/**                                                                       **/
/**  Parallel port joystick driver, (C) Deon van der Westhuysen 2002      **/
/**                                                                       **/
/**  PnP.c      Routines to handle interaction with the PnP manager       **/
/**             Starts and stops the FDOs                                 **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "PPortJoy.h"
#include "PPJIoctl.h"
#include "JoyDefs.h"
#include <wchar.h>

#include <windef.h>
#define	DIJ_RINGZERO
#include <dinputd.h>

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE,PPJoy_PnP)
    #pragma alloc_text (PAGE,PPJoy_Ctl_PnP)
    #pragma alloc_text (PAGE,PPJoy_PnpStart)
    #pragma alloc_text (PAGE,PPJoy_PnpUpdateRegistry)
    #pragma alloc_text (PAGE,PPJoy_InitPortAndInterface)
#endif

/* Flag - have we registered for option updates yet? */
int	RegisteredForOptionUpdates;


/**************************************************************************/
/* Main entry point for PlugAndPlay IRPs. Either handle or route them.    */
/**************************************************************************/
NTSTATUS PPJoy_PnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;
 PIO_STACK_LOCATION	IrpStack;
 WCHAR				SymNameBuffer[128];
 UNICODE_STRING		SymLinkName;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_PNP|PPJOY_FENTRY, ("PPJoy_PnP(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
 IrpStack = IoGetCurrentIrpStackLocation (Irp);

 ntStatus= PPJoy_EnterRequest (DeviceExtension);
 if (!NT_SUCCESS(ntStatus))
 {
  Irp->IoStatus.Information= 0;
  Irp->IoStatus.Status= ntStatus;
  IoCompleteRequest (Irp,IO_NO_INCREMENT);
  goto Exit;
 }

 switch(IrpStack->MinorFunction)
 {
  case IRP_MN_START_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoy_Pnp: IRP_MN_START_DEVICE") );

		/* The Start routing will call lower level drivers so we complete */
		/* IRP when the Start routine returns.                            */
		ntStatus= PPJoy_PnpStart (DeviceObject,Irp);
		Irp->IoStatus.Information= 0;
		Irp->IoStatus.Status= ntStatus;
		IoCompleteRequest (Irp,IO_NO_INCREMENT);
		break;

  case IRP_MN_STOP_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoy_Pnp: IRP_MN_STOP_DEVICE") );

		if (DeviceExtension->Mapping)
		 ExFreePool(DeviceExtension->Mapping);
		DeviceExtension->Mapping= NULL;
  
		if (DeviceExtension->Timing)
		 ExFreePool(DeviceExtension->Timing);
		DeviceExtension->Timing= NULL;

		/* Set the DeviceExtension flag to say that our device is stopped */
		 DeviceExtension->Flags&= ~PPJOY_FLAGS_STARTED;

		/* Then pass call to lower driver */
		IoSkipCurrentIrpStackLocation (Irp);
		ntStatus= IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject),Irp);
		break;

  case IRP_MN_SURPRISE_REMOVAL:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoy_Pnp: IRP_MN_SURPRISE_REMOVAL") );

		/* Simply pass call to lower driver */
		IoSkipCurrentIrpStackLocation (Irp);
		ntStatus= IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject),Irp);
		break;

  case IRP_MN_REMOVE_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoy_Pnp: IRP_MN_REMOVE_DEVICE") );

		if (DeviceExtension->Mapping)
		 ExFreePool(DeviceExtension->Mapping);
		DeviceExtension->Mapping= NULL;

		if (DeviceExtension->Timing)
		 ExFreePool(DeviceExtension->Timing);
		DeviceExtension->Timing= NULL;

		if (Globals.NumPresentJoys)			/* Sanity check! This value should always be >0 here */
		{
		 if (!(--Globals.NumPresentJoys))	/* Now decrement count for joystick we are about to remove */
		 {
		  /* Send a NULL address to un-register for update messages */
		  PPJOY_OPTION_CALLBACK	UpdateCallbackProc;

		  UpdateCallbackProc= NULL;

		  ntStatus= PPortJoy_SendInternalIoctl (GET_NEXT_DEVICE_OBJECT(DeviceObject),
											   IOCTL_PPJOYBUS_REGOPTIONS,
											   &UpdateCallbackProc,sizeof(UpdateCallbackProc),
											   NULL,0,-1);

		  if(!NT_SUCCESS(ntStatus))
		  {
		   PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("Error de-registering for option updates from PPJoyBus.sys 0x%X",ntStatus) );
		   PPortJoy_WriteEventLog (PPJ_MSG_ERRORREGUPDATES,&ntStatus,sizeof(ntStatus),L"");
		  }

		  RegisteredForOptionUpdates= FALSE;
		 }
		}
		else
		{
		 PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("PPJoy_PnP: NumPresentJoys=0 before IRP_MN_REMOVE_DEVICE") );
		}

		/* Set the DeviceExtension flag to say that our device is removed */
		DeviceExtension->Flags|= PPJOY_FLAGS_REMOVED;

		/* Set status success in this IRP before passing down. Else Driver */
		/* Verifier moans about it. (At least for the bus enumerator...)   */
		Irp->IoStatus.Status= STATUS_SUCCESS;

		/* Pass call to lower driver */
		IoSkipCurrentIrpStackLocation (Irp);
		ntStatus= IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject),Irp);
		
		/* Leave request to decrease requests counter. Event should be */
		/* signaled when last request leaves. Then we will return a    */
		/* successful status.                                          */
		PPJoy_LeaveRequest (DeviceExtension);

        KeWaitForSingleObject (&DeviceExtension->RemoveEvent,Executive,KernelMode,FALSE,NULL);
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE2, ("KeWaitForSingleObject on RemoveEvent completed.") );

		/* If there is a control device allocated for us, delete it */
		if (DeviceExtension->CtlDevObject)
		{
		 /* Set associate dev ext (pointed to our joystick dev ext) to   */
		 /* to NULL in control device - our dev ext will be invalid soon */
		 ((PCTLDEV_EXTENSION)(DeviceExtension->CtlDevObject->DeviceExtension))->hde.MiniDeviceExtension= NULL;

		 if (DeviceExtension->Config.LPTNumber==0)
		 {
		  /* Delete IOCTL symlink. */
		  swprintf (SymNameBuffer,L"\\DosDevices\\" LJOY_IOCTL_DEV_NAME,DeviceExtension->Config.UnitNumber+1);
		  RtlInitUnicodeString(&SymLinkName,SymNameBuffer);
		  ntStatus= IoDeleteSymbolicLink(&SymLinkName);
		  PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE2, ("Removing symbolic link %S - ntStatus 0x%X.",SymNameBuffer,ntStatus) );
		 }

		 /* Delete Config symlink. */
		 swprintf (SymNameBuffer,L"\\DosDevices\\" LJOY_CONTROL_DEV_NAME,DeviceExtension->Config.LPTNumber,DeviceExtension->Config.UnitNumber+1);
		 RtlInitUnicodeString(&SymLinkName,SymNameBuffer);
		 ntStatus= IoDeleteSymbolicLink(&SymLinkName);
		 PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE2, ("Removing symbolic link %S - ntStatus 0x%X.",SymNameBuffer,ntStatus) );

		 IoDeleteDevice (DeviceExtension->CtlDevObject);
		 PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE2, ("Deleting control device 0x%X.",DeviceExtension->CtlDevObject) );
		 DeviceExtension->CtlDevObject= NULL;
		}

		/* Delete joystick mapping if any */
		if (DeviceExtension->Mapping)
		{
		 ExFreePool(DeviceExtension->Mapping);
		 DeviceExtension->Mapping= NULL;
		}

		ntStatus= STATUS_SUCCESS;
		goto ExitNoLeaveRequest;
		break;

  default:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_WARN, ("PPJoy_PnP: IrpStack->MinorFunction Not handled 0x%x",IrpStack->MinorFunction) );

		/* Simply pass call to lower driver */
		IoSkipCurrentIrpStackLocation (Irp);
		ntStatus= IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject),Irp);
		break;
 }

Exit:
 PPJoy_LeaveRequest (DeviceExtension);
ExitNoLeaveRequest:
 PPJOY_EXITPROC (FILE_PNP|PPJOY_FEXIT,"PPJoy_PnP",ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Main entry point for contol dev PlugAndPlay IRPs. Handle or route them */
/**************************************************************************/
NTSTATUS PPJoy_Ctl_PnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PIO_STACK_LOCATION	IrpStack;

 PAGED_CODE();

 /* If it is not for one of our control devices - pass on to HID.sys */
 if (GET_NEXT_DEVICE_OBJECT(DeviceObject))
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_HIDHACK, ("Not a control device - passing request on to HID.sys (PPJoy_Ctl_PnP)") );
  return HIDMajorFunctions[IoGetCurrentIrpStackLocation(Irp)->MajorFunction](DeviceObject,Irp);
 }

 /* HMM - don't think we expose device to plug and play manager... Code below is never called */

 PPJOY_DBGPRINT (FILE_PNP|PPJOY_FENTRY, ("PPJoy_Ctl_PnP(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 IrpStack = IoGetCurrentIrpStackLocation (Irp);
 switch(IrpStack->MinorFunction)
 {
  case IRP_MN_START_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoy_Ctl_Pnp: IRP_MN_START_DEVICE") );

		ntStatus= STATUS_SUCCESS;
		Irp->IoStatus.Information= 0;
		Irp->IoStatus.Status= ntStatus;
		break;

  case IRP_MN_STOP_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoy_Ctl_Pnp: IRP_MN_STOP_DEVICE") );

		ntStatus= STATUS_SUCCESS;
		Irp->IoStatus.Information= 0;
		Irp->IoStatus.Status= ntStatus;
		break;

  case IRP_MN_SURPRISE_REMOVAL:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoy_Ctl_Pnp: IRP_MN_SURPRISE_REMOVAL") );

		ntStatus= STATUS_SUCCESS;
		Irp->IoStatus.Information= 0;
		Irp->IoStatus.Status= ntStatus;
		break;

  case IRP_MN_REMOVE_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoy_Ctl_Pnp: IRP_MN_REMOVE_DEVICE") );

		ntStatus= STATUS_SUCCESS;
		Irp->IoStatus.Information= 0;
		Irp->IoStatus.Status= ntStatus;
		break;

  default:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_WARN, ("PPJoy_Ctl_PnP: IrpStack->MinorFunction Not handled 0x%x",IrpStack->MinorFunction) );

		/* Simply return incoming status */
		ntStatus= Irp->IoStatus.Status;
		break;
 }

 IoCompleteRequest (Irp,IO_NO_INCREMENT);
 PPJOY_EXITPROC (FILE_PNP|PPJOY_FEXIT,"PPJoy_Ctl_PnP",ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine to handle the START PnP IRP. First send IRP on to the Bus      */
/* Then get the Joystick parameters and initialise the joystick interface */
/**************************************************************************/
NTSTATUS PPJoy_PnpStart (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;
 PIO_STACK_LOCATION	IrpStack;
 PIO_STACK_LOCATION	NextStack;
 PDEVICE_OBJECT		CtlDevObject;
 PCTLDEV_EXTENSION	CtlDevExtension;

 WCHAR				DevNameBuffer[128];
 UNICODE_STRING		DeviceName;
 WCHAR				SymNameBuffer[128];
 UNICODE_STRING		SymLinkName;

 int				Count;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_PNP|PPJOY_FENTRY, ("PPJoy_PnpStart(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 NextStack= IoGetNextIrpStackLocation(Irp);

 /* Send START IRP on to the Bus driver */
 ntStatus= PPortJoy_SendIrpSynchronously (GET_NEXT_DEVICE_OBJECT(DeviceObject),Irp,1);
 if(NT_SUCCESS(ntStatus))
  ntStatus= Irp->IoStatus.Status;

 if(!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("Error starting joystick PDO - 0x%X",ntStatus) );
  PPortJoy_WriteEventLog (PPJ_MSG_ERRORSTARTINGPDO,&ntStatus,sizeof(ntStatus),L"");
  goto Exit;	 
 }

 if (!RegisteredForOptionUpdates)
 {
  PPJOY_OPTION_CALLBACK	UpdateCallbackProc;

  RegisteredForOptionUpdates= TRUE;
  UpdateCallbackProc= PPJoy_UpdateOptions;

  ntStatus= PPortJoy_SendInternalIoctl (GET_NEXT_DEVICE_OBJECT(DeviceObject),
									   IOCTL_PPJOYBUS_REGOPTIONS,
									   &UpdateCallbackProc,sizeof(UpdateCallbackProc),
									   NULL,0,-1);

  if(!NT_SUCCESS(ntStatus))
  {
   PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("Error registering for option updates from PPJoyBus.sys 0x%X",ntStatus) );
   PPortJoy_WriteEventLog (PPJ_MSG_ERRORREGUPDATES,&ntStatus,sizeof(ntStatus),L"");
  }
 }

 /***********************************************************************/
 /* Now we will query the bus driver for the parameters for this device */
 /***********************************************************************/

 ntStatus= PPortJoy_SendInternalIoctl (GET_NEXT_DEVICE_OBJECT(DeviceObject),
									   IOCTL_PPJOYBUS_GETCONFIG1,NULL,0,
									   &(DeviceExtension->Config),
									   sizeof(DeviceExtension->Config),-1);

 if(!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("Error reading joystick parameters (CONFIG1) from bus driver - 0x%X",ntStatus) );
  PPortJoy_WriteEventLog (PPJ_MSG_ERRORREADINGPARAMS,&ntStatus,sizeof(ntStatus),L"");
  goto Exit;	 
 }

 if (DeviceExtension->Config.Size!=sizeof(DeviceExtension->Config))
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("Error reading joystick parameters - incorrect size for Config %d, expected %d",DeviceExtension->Config.Size,sizeof(DeviceExtension->Config)) );
  PPortJoy_WriteEventLog (PPJ_MSG_ERRORREADINGPARAMS,&ntStatus,sizeof(ntStatus),L"");
  ntStatus= STATUS_UNSUCCESSFUL;
  goto Exit;	 
 }

 PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE2, ("PPJoy_Pnp: [Raw] Joystick type 0x%X on port 0x%X",DeviceExtension->Config.JoyType,DeviceExtension->Config.PortAddress) );
 PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE2, ("PPJoy_Pnp: Joystick VID= 0x%04X PID= 0x%04X LPT%d",DeviceExtension->Config.VendorID,DeviceExtension->Config.ProductID,DeviceExtension->Config.LPTNumber) );

 /* Check that we know how to handle this type of joystick interface */
 if (DeviceExtension->Config.JoyType>PPJOY_MAX_JOYTYPE)
 {
  PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_ERROR, ("No such joystick type: %d",DeviceExtension->Config.JoyType) );
  PPortJoy_WriteEventLog (PPJ_MSG_ERRORJOYSTICKTYPE,&ntStatus,sizeof(ntStatus),L"");
  ntStatus= STATUS_INVALID_PARAMETER;
  goto Exit;
 }

 /* Read the joystick mapping from registry; if not found - construct default mapping */
 ntStatus= PPJoy_LoadJoystickMapping (DeviceObject);
 if(!NT_SUCCESS(ntStatus))
 {
  PPortJoy_WriteEventLog (PPJ_MSG_ERRORNOMAPPING,&ntStatus,sizeof(ntStatus),L"");
  goto Exit;
 }

 /* Read the timing parameters for this joystick from registry */
 ntStatus= PPJoy_LoadJoystickTiming (DeviceObject);
 if(!NT_SUCCESS(ntStatus))
 {
  PPortJoy_WriteEventLog (PPJ_MSG_ERRORNOTIMING,&ntStatus,sizeof(ntStatus),L"");
  goto Exit;
 }

 /* Update registry to match our new configuration ... */
 PPJoy_PnpUpdateRegistry (DeviceObject);

 /* Set the chip mode and set interface specific parameters */
 ntStatus= PPJoy_InitPortAndInterface (DeviceObject);
 if(!NT_SUCCESS(ntStatus))		// Will write eventlog entries below...
  goto Exit;

 RtlZeroMemory (&(DeviceExtension->RawInput),sizeof(DeviceExtension->RawInput));
 for (Count=0;Count<MAX_ANALOG_RAW;Count++)
  DeviceExtension->RawInput.Analog[Count]= (PPJOY_AXIS_MIN+PPJOY_AXIS_MAX)/2;	/* Centre */

 DeviceExtension->OpenCount= 0; /* No opens on device */

 /* If we haven't yet created the control device for this joystick */
 if (!(DeviceExtension->CtlDevObject))
 {
  /* Name of new control device object */
  swprintf (DevNameBuffer,L"\\Device\\" LJOY_CONTROL_DEV_NAME,DeviceExtension->Config.LPTNumber,DeviceExtension->Config.UnitNumber+1);
  RtlInitUnicodeString(&DeviceName,DevNameBuffer);

  /* Now that we got here - create control device for this joystick */
  ntStatus= IoCreateDevice(DeviceObject->DriverObject,sizeof(CTLDEV_EXTENSION),&DeviceName,FILE_DEVICE_PPORTJOY,0,FALSE,&CtlDevObject);
  if (!NT_SUCCESS (ntStatus))
  {
   PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("Error creating control device for new joystick. ntStatus(0x%x)",ntStatus) );
   PPortJoy_WriteEventLog (PPJ_MSG_ERRORCONTROLDEV,&ntStatus,sizeof(ntStatus),L"");
   goto Exit;
  }

  /* Create name for Control symlink. */
  swprintf (SymNameBuffer,L"\\DosDevices\\" LJOY_CONTROL_DEV_NAME,DeviceExtension->Config.LPTNumber,DeviceExtension->Config.UnitNumber+1);
  RtlInitUnicodeString(&SymLinkName,SymNameBuffer);

  PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("Creating symlink %S to device %S",SymLinkName.Buffer,DeviceName.Buffer) );
  
  ntStatus= IoCreateSymbolicLink(&SymLinkName,&DeviceName);
  if (!NT_SUCCESS (ntStatus))
  {
   PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("Error creating symlink %S to device %S. ntStatus(0x%x)",SymLinkName.Buffer,DeviceName.Buffer,ntStatus) );
   PPortJoy_WriteEventLog (PPJ_MSG_ERRORCONTROLLINK,&ntStatus,sizeof(ntStatus),L"");
   IoDeleteDevice (CtlDevObject);
   goto Exit;
  }

  if (DeviceExtension->Config.LPTNumber==0)
  {
   /* Create name for IOCTL symlink. */
   swprintf (SymNameBuffer,L"\\DosDevices\\" LJOY_IOCTL_DEV_NAME,DeviceExtension->Config.UnitNumber+1);
   RtlInitUnicodeString(&SymLinkName,SymNameBuffer);

   PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("Creating symlink %S to device %S",SymLinkName.Buffer,DeviceName.Buffer) );
  
   ntStatus= IoCreateSymbolicLink(&SymLinkName,&DeviceName);
   if (!NT_SUCCESS (ntStatus))
   {
    PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("Error creating symlink %S to device %S. ntStatus(0x%x)",SymLinkName.Buffer,DeviceName.Buffer,ntStatus) );
    PPortJoy_WriteEventLog (PPJ_MSG_ERRORIOCTLLINK,&ntStatus,sizeof(ntStatus),L"");
    IoDeleteDevice (CtlDevObject);
    goto Exit;
   }
  }

  /* Setup device extension for control device */
  CtlDevExtension= (PCTLDEV_EXTENSION) CtlDevObject->DeviceExtension;
  RtlZeroMemory (CtlDevExtension,sizeof(*CtlDevExtension));

  /* Save the pointer to the joystick dev ext in the control device ext */
  CtlDevExtension->hde.MiniDeviceExtension= DeviceExtension;
  CtlDevExtension->JoyDeviceObject= DeviceObject;

  /* Tell IOManager et al that we are pagable. */
  CtlDevObject->Flags|= DO_POWER_PAGABLE;

  /* Done initializing, clear flag to let system know the device is ready for use. */
  CtlDevObject->Flags &= ~DO_DEVICE_INITIALIZING;

  /* Save link to control device in joystick device extension */
  DeviceExtension->CtlDevObject= CtlDevObject;
 }

 /* Set the DeviceExtension flag to say that our joystick device is started */
 DeviceExtension->Flags|= PPJOY_FLAGS_STARTED;

Exit:
 PPJOY_EXITPROC (FILE_PNP | PPJOY_FEXIT, "PPJoy_PnpStart", ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine to increment the count of current requests being serviced.     */
/* It is not pagable because it can be called by high IRQL code.          */
/**************************************************************************/
NTSTATUS PPJoy_EnterRequest	(PDEVICE_EXTENSION DeviceExtension)
{
 NTSTATUS	ntStatus;

 /* Caller did not pass a valid DeviceExtension! */
 if (!DeviceExtension)
  return STATUS_NO_SUCH_DEVICE;

 /* Increase request count */
 InterlockedIncrement (&DeviceExtension->ReqCount);
    
 if (DeviceExtension->Flags&PPJOY_FLAGS_REMOVED)
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_WARN, ("Recieved a new request after a PnP Remove IRP.") );
  ntStatus= STATUS_DELETE_PENDING;
 }
 else
  ntStatus= STATUS_SUCCESS;

 return ntStatus;
}

/**************************************************************************/
/* Routine to decrement the count of current requests being serviced.     */
/* If we are exiting the last request when the remove flag has been set   */
/* then signal the event as well. It is not pagable because it can be     */
/* called by high IRQL code.                                              */
/**************************************************************************/
NTSTATUS PPJoy_LeaveRequest	(PDEVICE_EXTENSION DeviceExtension)
{
 /* Caller did not pass a valid DeviceExtension! */
 if (!DeviceExtension)
  return STATUS_NO_SUCH_DEVICE;

 /* Decrease request count. If zero we will check signal RemoveEvent if */
 /* the remove flag is set.                                             */
 if (!InterlockedDecrement(&DeviceExtension->ReqCount))
 {
  if (DeviceExtension->Flags&PPJOY_FLAGS_REMOVED)
   KeSetEvent (&DeviceExtension->RemoveEvent,IO_NO_INCREMENT,FALSE);
 }

 return STATUS_SUCCESS;
}

/**************************************************************************/
/* Routine to set the parallel port chip mode and re-init the direction   */
/* and other interface specific parameters. This is called on device      */
/* start and on return from hibernation.                                  */
/**************************************************************************/
NTSTATUS PPJoy_InitPortAndInterface (IN PDEVICE_OBJECT DeviceObject)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;
 UCHAR				ChipMode;
 JOYSTICK_IRQ1		IRQ;
 
 PAGED_CODE ();

 DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

 /***********************************************************************/
 /* Now set the port chipmode to the required mode for our interface    */
 /***********************************************************************/

 /* If this is a real controller we initialise the underlying parallel port */
 if (DeviceExtension->Config.LPTNumber)
 {
  ChipMode= JoyDefs[DeviceExtension->Config.JoyType].ChipMode;

  ntStatus= PPortJoy_SendInternalIoctl (GET_NEXT_DEVICE_OBJECT(DeviceObject),
									    IOCTL_PPJOYBUS_CHIPMODE,
									    &ChipMode,sizeof(ChipMode),NULL,0,-1);
  if(!NT_SUCCESS(ntStatus))
  {
   PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("Error calling IOCTL_PPJOYBUS_CHIPMODE with chipmode 0x%X - 0x%X",ChipMode,ntStatus) );
   PPortJoy_WriteEventLog (PPJ_MSG_ERRORCHIPMODE,&ntStatus,sizeof(ntStatus),L"");
   goto Exit;	 
  }
 }

 /* And initialise the interface */
 if (JoyDefs[DeviceExtension->Config.JoyType].InitFunc)
 {
  JoyDefs[DeviceExtension->Config.JoyType].InitFunc ((PUCHAR)(ULONG_PTR)DeviceExtension->Config.PortAddress,DeviceExtension->Config.JoyType);
  ntStatus= STATUS_SUCCESS;
 }
 else
 {
  PPJOY_DBGPRINT (FILE_PNP | PPJOY_ERROR, ("NULL init function for joystick type: %d",DeviceExtension->Config.JoyType) );
  PPortJoy_WriteEventLog (PPJ_MSG_ERRORINITFUNC,&ntStatus,sizeof(ntStatus),L"");
  ntStatus= STATUS_UNSUCCESSFUL;
 }

 /* If we successfully initialised the interface - try to allocate IRQ if the interface requires it */
 if(NT_SUCCESS(ntStatus))
 {
  /* Allocate IRQ for BuddyBox connection */
  if (DeviceExtension->Config.JoyType==IF_FMSBUDDYBOX)
  {
   IRQ.Callback= (PKSERVICE_ROUTINE) ISR_FMSBuddyBox;
   IRQ.Context= DeviceExtension;

   ntStatus= PPortJoy_SendInternalIoctl (GET_NEXT_DEVICE_OBJECT(DeviceObject),IOCTL_PPJOYBUS_ALLOCIRQ1,
										 &IRQ,sizeof(IRQ),NULL,0,-1);
   if(!NT_SUCCESS(ntStatus))
   {
    PPortJoy_WriteEventLog (PPJ_MSG_ERRORINTERRUPT,&ntStatus,sizeof(ntStatus),L"");
    PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("Error calling IOCTL_PPJOYBUS_ALLOCIRQ1") );
   }
  }
 }

Exit:
 return ntStatus;
}

/**************************************************************************/
/* Update registry values to reflect current joystick name and mapping    */
/**************************************************************************/
NTSTATUS PPJoy_PnpUpdateRegistry (IN PDEVICE_OBJECT DeviceObject)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;

 WCHAR				RegPath[256];
 JOYREGHWSETTINGS	OEMData;
 
 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_PNP|PPJOY_FENTRY, ("PPJoy_PnpUpdateRegistry(DeviceObject=0x%p)",DeviceObject) );

 /// TEMP TEMP TEMP
#if 1
  return STATUS_SUCCESS;
#endif
  /// TEMP TEMP TEMP
 

 DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

 /* Set HKLM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_DEAD&PID_BEF0\OEMData */
 if (DeviceExtension->Mapping->NumAxes>5)
  OEMData.dwFlags= JOY_HWS_HASZ|JOY_HWS_HASR|JOY_HWS_HASU|JOY_HWS_HASV;
 else if (DeviceExtension->Mapping->NumAxes>4)
  OEMData.dwFlags= JOY_HWS_HASZ|JOY_HWS_HASR|JOY_HWS_HASU;
 else if (DeviceExtension->Mapping->NumAxes>3)
  OEMData.dwFlags= JOY_HWS_HASZ|JOY_HWS_HASR;
 else if (DeviceExtension->Mapping->NumAxes>2)
  OEMData.dwFlags= JOY_HWS_HASZ;
 else
  OEMData.dwFlags= 0;

 /* TODO: Update OEMData flags for POV Hats exported by the mapping? */

 OEMData.dwNumButtons= DeviceExtension->Mapping->NumButtons;

 swprintf (RegPath,L"MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_%04X&PID_%04X",DeviceExtension->Config.VendorID,DeviceExtension->Config.ProductID);
 ntStatus= RtlWriteRegistryValue (RTL_REGISTRY_CONTROL,RegPath,L"OEMData",REG_BINARY,&OEMData,sizeof(OEMData));

 if(!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_WARN, ("RtlWriteRegistryValue error 0x%X trying to try %S\\OEMData",ntStatus,RegPath) );
  goto Exit;	 
 }

Exit:
 return ntStatus;
}

void PPJoy_UpdateOptions (ULONG Option, ULONG Value)
{
 PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoy_UpdateOptions: received updated option %d value %d",Option,Value) );
 if (Option==PPJOY_OPTION_RUNTIMEDEBUG)
 {
  PPJoy_RuntimeDebug= Value;
  return;
 }
 if (Option==PPJOY_OPTION_PORTFLAGS)
 {
  /*ignored*/
  return;
 }

 PPJOY_DBGPRINT (FILE_PNP|PPJOY_WARN, ("PPJoy_UpdateOptions: Unknown option %d ignored",Option) );
}
