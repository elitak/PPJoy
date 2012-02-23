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
/**  BusPnP.c   Routine to handle PnP IRPs for the bus enumerator FDO     **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "PPJoyBus.h"
#include <wchar.h>

/**************************************************************************/
/* Declare the code segments into which each routine will be placed       */
/* Used to declare pagable and initialisation code.                       */
/**************************************************************************/

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE,PPJoyBus_PnP_Bus)
    #pragma alloc_text (PAGE,PPJoyBus_BusStartDevice)
    #pragma alloc_text (PAGE,PPJoyBus_BusStopDevice)
    #pragma alloc_text (PAGE,PPJoyBus_BusRemoveDevice)
    #pragma alloc_text (PAGE,PPJoyBus_BusQueryRelations)
#endif



/**************************************************************************/
/* Main routine to handle PnP IRPs for the bus enumerator. We will try to */
/* to handle most of it ourselves; otherwise pass it on to helpers.       */
/**************************************************************************/
NTSTATUS PPJoyBus_PnP_Bus (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PIO_STACK_LOCATION	IrpStack;
 PBUS_DEVICE_DATA	BusDeviceData;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_PNP|PPJOY_FENTRY, ("PPJoyBus_PnP_Bus(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 BusDeviceData= (PBUS_DEVICE_DATA)DeviceObject->DeviceExtension;

 IrpStack = IoGetCurrentIrpStackLocation (Irp);
 switch(IrpStack->MinorFunction)
 {
  case IRP_MN_START_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Bus: IRP_MN_START_DEVICE") );

		ntStatus= PPJoyBus_SendIrpSynchronously (BusDeviceData->NextLowerDriver,Irp);
		if (NT_SUCCESS(ntStatus))
         ntStatus= PPJoyBus_BusStartDevice (BusDeviceData, Irp);

		/* We must now complete the IRP */ 
		goto CompleteIRP;

  case IRP_MN_STOP_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Bus: IRP_MN_STOP_DEVICE") );

		PPJoyBus_BusStopDevice (BusDeviceData,Irp);
		Irp->IoStatus.Status= STATUS_SUCCESS;
		goto PassOnIRP;  

  case IRP_MN_QUERY_STOP_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Bus: IRP_MN_QUERY_STOP_DEVICE") );

		/* We will (for now) always allow the device to be stopped. */
		Irp->IoStatus.Status= STATUS_SUCCESS;
		goto PassOnIRP;  

  case IRP_MN_CANCEL_STOP_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Bus: IRP_MN_CANCEL_STOP_DEVICE") );

		/* Can always cancel the pending stop query */
		Irp->IoStatus.Status= STATUS_SUCCESS;
		goto PassOnIRP;  

  case IRP_MN_QUERY_REMOVE_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Bus: IRP_MN_QUERY_REMOVE_DEVICE") );

		/* We will always allow the device to be removed. */
		Irp->IoStatus.Status= STATUS_SUCCESS;
		goto PassOnIRP;  

  case IRP_MN_CANCEL_REMOVE_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Bus: IRP_MN_CANCEL_REMOVE_DEVICE") );

		/* Can always cancel the pending remove query */
		Irp->IoStatus.Status = STATUS_SUCCESS; // We must not fail the IRP.
		goto PassOnIRP;  

  case IRP_MN_SURPRISE_REMOVAL:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Bus: IRP_MN_SURPRISE_REMOVAL") );

		/* Should we perhaps now delete all Joystick PDOs belonging to */
		/* this bus? For now we don't. Check later...                  */

		/* We are not allowed to fail this IRP */
		Irp->IoStatus.Status= STATUS_SUCCESS;
		goto PassOnIRP;  

  case IRP_MN_REMOVE_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Bus: IRP_MN_REMOVE_DEVICE") );

		/* We stop the device if it wasn't done before the remove */
		if (BusDeviceData->Flags&PPJFLAGS_STARTED)
		 PPJoyBus_BusStopDevice (BusDeviceData,Irp);

		/* Set the DeviceExtension flag to say that our device is removed */
		BusDeviceData->Flags|= PPJFLAGS_REMOVED;

		/* Set status success in this IRP before passing down. Else Driver */
		/* Verifier moans about it. Strange?!                              */
		Irp->IoStatus.Status= STATUS_SUCCESS;

		/* Pass call to lower driver */
		IoSkipCurrentIrpStackLocation (Irp);
		ntStatus= IoCallDriver (BusDeviceData->NextLowerDriver,Irp);
		
		/* Leave request to decrease requests counter. Event should be */
		/* signaled when last request leaves. Then we will return a    */
		/* successful status.                                          */
		PPJoyBus_LeaveRequest((PCOMMON_DATA)BusDeviceData);

        KeWaitForSingleObject (&BusDeviceData->RemoveEvent,Executive,KernelMode,FALSE,NULL);
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE2, ("KeWaitForSingleObject on RemoveEvent completed.") );

		/* We will now cleanup the bus. Remove any joystick PDOs still */
		/* present.                                                    */
		PPJoyBus_BusRemoveDevice (BusDeviceData,Irp);

		/* We are not allowed to fail this IRP */
		ntStatus= STATUS_SUCCESS;
		goto Exit;

  case IRP_MN_QUERY_DEVICE_RELATIONS:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Bus: IRP_MN_QUERY_DEVICE_RELATIONS") );

		/* Check to see if this is a request for BusRelations */
		if (IrpStack->Parameters.QueryDeviceRelations.Type!=BusRelations)
		{
		 /* Since we don't support any other type of relation, pass it on. */
		 PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("QueryDeviceRelation Type: %s ignored!\n",DbgDeviceRelationString(IrpStack->Parameters.QueryDeviceRelations.Type)) );
		 goto PassOnIRP;  
		}

		ntStatus= PPJoyBus_BusQueryRelations (BusDeviceData,Irp);
		/* If we fail then complete the IRP immediately and report status */
	    if (!NT_SUCCESS(ntStatus))
		 goto CompleteIRP;
	
		/* Otherwise allow other drivers to modify the IRP */
		goto PassOnIRP;  

  default:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_WARN, ("PPJoyBus_PnP_Bus: IrpStack->MinorFunction Not handled %s (0x%x)",PnPMinorFunctionString(IrpStack->MinorFunction),IrpStack->MinorFunction) );

		/* If we don't know what to do with the IRP then pass it on... */
		goto PassOnIRP;  
 }

PassOnIRP:
 IoSkipCurrentIrpStackLocation (Irp);
 ntStatus= IoCallDriver (BusDeviceData->NextLowerDriver,Irp);
 goto Exit;

CompleteIRP:
 Irp->IoStatus.Status= ntStatus;
 IoCompleteRequest (Irp,IO_NO_INCREMENT);
 goto Exit;

Exit:

 PPJOY_EXITPROC (FILE_PNP|PPJOY_FEXIT, "PPJoyBus_PnP_Bus",ntStatus );
 return ntStatus;
}

/**************************************************************************/
/* Routine to handle start requests for the bus enumerator FDO. Register  */
/* for PnP notification and enable our interface for IOCTL requests       */
/**************************************************************************/
NTSTATUS PPJoyBus_BusStartDevice (IN PBUS_DEVICE_DATA BusDeviceData, IN PIRP Irp)
{
 NTSTATUS			ntStatus;

 PAGED_CODE();

 if (BusDeviceData->Flags&PPJFLAGS_STARTED)
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("Bus device already started, ignoring request") );
  return STATUS_SUCCESS;
 }

 /* Hmm, should check that only one Bus enumerator is started, and only once... */
 /* However we will not do so today. Perhaps later.                             */

 /* Enable our device interface so we can get IOCTLs from userland.             */
 /* Ignore any error that occurs. This only means the control panel applet will */
 /* not be able to modify the joystick definitions.                             */ 
 ntStatus= IoSetDeviceInterfaceState(&BusDeviceData->InterfaceName,TRUE);
 if (!NT_SUCCESS (ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("IoSetDeviceInterfaceState failed: 0x%x\n",ntStatus) );
 }

 /* Now we register for Plug and Play notifications. If this fails we got */    
 /* bigger problems since we won't be able to enable any joysticks.       */
 ntStatus= PPJoyBus_RegisterNotifications (BusDeviceData);
 if (!NT_SUCCESS (ntStatus))
 {
  PPJoyBus_WriteEventLog (PPJ_MSG_ERRORPNPNOTIFY,&ntStatus,sizeof(ntStatus),L"");
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("IoRegisterPlugPlayNotification failed: 0x%x\n",ntStatus) );
  return STATUS_UNSUCCESSFUL;
 }

 /* Set flag to indicate that we are successfully started */
 BusDeviceData->Flags|= PPJFLAGS_STARTED;
 return STATUS_SUCCESS;
}

/**************************************************************************/
/* Routine to handle stop requests for the bus enumerator FDO. Unregister */
/* the PnP notifications and disable our interface for IOCTL requests     */
/**************************************************************************/
NTSTATUS PPJoyBus_BusStopDevice (IN PBUS_DEVICE_DATA BusDeviceData, IN PIRP Irp)
{
 NTSTATUS			ntStatus;

 PAGED_CODE();

 if (!(BusDeviceData->Flags&PPJFLAGS_STARTED))
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("Bus device already stopped, ignoring request") );
  return STATUS_SUCCESS;
 }

 /* Unregister the PnP notifications */
 PPJoyBus_UnregisterNotifications (BusDeviceData);

 /* Disable our device interface; Ignore any error that occurs. */
 ntStatus= IoSetDeviceInterfaceState(&BusDeviceData->InterfaceName,FALSE);
 if (!NT_SUCCESS (ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("IoSetDeviceInterfaceState failed: 0x%x\n",ntStatus) );
 }

 /* Clear the started flag for our Device Object */
 BusDeviceData->Flags&= ~PPJFLAGS_STARTED;

 return STATUS_SUCCESS;
}

/**************************************************************************/
/* Routine to handle removal requests for this device. Free any           */
/* per-device resource allocations, delete and remaining Joystick PDOs    */
/* then delete our DeviceObject                                           */
/**************************************************************************/
NTSTATUS PPJoyBus_BusRemoveDevice (IN PBUS_DEVICE_DATA BusDeviceData, IN PIRP Irp)
{
 PLIST_ENTRY			pEntry;
 PJOY_DEVICE_DATA		JoyDeviceData;

 WCHAR					LinkNameBuffer[128];
 UNICODE_STRING			LinkName;

 PAGED_CODE();

 /* Get out of the chain of command. */
 IoDetachDevice (BusDeviceData->NextLowerDriver);

 /* Free resources in the Bus DeviceExtension */
 if (BusDeviceData->InterfaceName.Buffer)
 {
  /* Set interface state to FALSE, just in case... */
  IoSetDeviceInterfaceState (&BusDeviceData->InterfaceName,FALSE);
  /* Free memory allocated by IoRegisterDeviceInterface in AddDevice */
  RtlFreeUnicodeString (&BusDeviceData->InterfaceName);
  RtlZeroMemory (&BusDeviceData->InterfaceName,sizeof (UNICODE_STRING)); 
 }

 /* Now we proceed to delete the DeviceObjects for all our child PDOs */

 /* Acquire the mutex to make sure nobody else changes the list. */
 ExAcquireFastMutex (&Globals.Mutex);
 
 /* Iterate through the PDOs and remove their DeviceObjects */
 pEntry= BusDeviceData->JoystickList.Flink;
 while (pEntry!=&BusDeviceData->JoystickList)
 {
  JoyDeviceData= CONTAINING_RECORD (pEntry,JOY_DEVICE_DATA,ListEntry);
  pEntry= pEntry->Flink;

  /* Stop device before deleting it. Note: we hold the mutex so we should  */
  /* not try and grab it again. PPJoyBus_JoyStopDevice is safe.            */
  /* We have no IRP to pass to StopDevice so we send NULL. StopDevice does */
  /* not use it in any case.                                               */
  if (JoyDeviceData->Flags&PPJFLAGS_STARTED)
   PPJoyBus_JoyStopDevice (JoyDeviceData,NULL);

  /* Now delete the DeviceObject for the current joystick */
  IoDeleteDevice (JoyDeviceData->Self);
 }

 /* Allow others (what others? <g>) to access the joystick device list */
 ExReleaseFastMutex (&Globals.Mutex);

 /* Finally delete our own Device Object. */
 IoDeleteDevice (BusDeviceData->Self);

 return STATUS_SUCCESS;
}

/**************************************************************************/
/* Routine to return a structure with all our active joystick PDOs in it. */
/**************************************************************************/
NTSTATUS PPJoyBus_BusQueryRelations (IN PBUS_DEVICE_DATA BusDeviceData, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PLIST_ENTRY			pEntry;
 PJOY_DEVICE_DATA		JoyDeviceData;

 PDEVICE_RELATIONS		OldRelations;
 PDEVICE_RELATIONS		NewRelations;
 ULONG					RelationsSize;

 ULONG					NumSticks;
 ULONG					PrevCount;

 PAGED_CODE();

 NumSticks= 0;

 /* Acquire the mutex to make sure nobody changes the list. */
 ExAcquireFastMutex (&Globals.Mutex);
 
 /* Count the number of Joystick PDOs in the list that are not marked */
 /* removed or unplugged and for which the hardware was found.        */
 pEntry= BusDeviceData->JoystickList.Flink;
 while (pEntry!=&BusDeviceData->JoystickList)
 {
  JoyDeviceData= CONTAINING_RECORD (pEntry,JOY_DEVICE_DATA,ListEntry);
  pEntry= pEntry->Flink;

  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("Found joystick type %d on port %d Flags: 0x%X",JoyDeviceData->Config.JoyType,JoyDeviceData->Config.LPTNumber,JoyDeviceData->Flags) );

  if (EXPOSE_DEVICE(JoyDeviceData->Flags))
  {
   PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("...reporting this device to the Plug and Play Manager") );
   NumSticks++;
  }
 }

 /* Determine whether another (filter) driver has already started to */
 /* populate the DEVICE_RELATIONS structure. If so, count entries.   */
 PrevCount= 0;
 OldRelations= (PDEVICE_RELATIONS) Irp->IoStatus.Information;
 if (OldRelations)
 {
  PrevCount= OldRelations->Count; 
  if (!NumSticks)
  {
   /* No joysticks of our own to add. Pass the IRP on to next driver. */
   ntStatus= STATUS_SUCCESS;
   goto Exit;
  }
 }

 /* Calculate the new size for the DEVICE_RELATIONS structure and allocate */
 RelationsSize= sizeof(DEVICE_RELATIONS)+ ((NumSticks+PrevCount-1) * sizeof (PDEVICE_OBJECT));
 NewRelations= (PDEVICE_RELATIONS) ExAllocatePoolWithTag (PagedPool,RelationsSize,PPJOYBUS_POOL_TAG);
 if (NewRelations==NULL)
 {
  /* Fail the IRP if the allocation fails. */
  ntStatus= STATUS_INSUFFICIENT_RESOURCES;
  goto Exit;
 }

 /* Copy existing device relations to the new stucture. Free old structure */
 if (PrevCount)
  RtlCopyMemory (NewRelations->Objects,OldRelations->Objects,PrevCount*sizeof(PDEVICE_OBJECT));
 if (OldRelations)
  ExFreePool (OldRelations);

 /* Now report on all our joystick PDOs */ 
 NewRelations->Count= PrevCount+NumSticks;
 for (pEntry= BusDeviceData->JoystickList.Flink; pEntry!=&BusDeviceData->JoystickList; pEntry= pEntry->Flink)
 {
  JoyDeviceData= CONTAINING_RECORD (pEntry,JOY_DEVICE_DATA,ListEntry);
  if (EXPOSE_DEVICE(JoyDeviceData->Flags))
  {
   NewRelations->Objects[PrevCount]= JoyDeviceData->Self;
   /* PnP mananger will free this reference */
   ObReferenceObject (JoyDeviceData->Self);
   PrevCount++;
  }
 }
 
 /* Return size of the new structure */
 Irp->IoStatus.Information = (ULONG_PTR) NewRelations;

 /* Set success status */
 ntStatus= STATUS_SUCCESS;
 Irp->IoStatus.Status= ntStatus;

Exit:
 /* Allow others to access the joystick device list */
 ExReleaseFastMutex (&Globals.Mutex);

 return ntStatus;
}

#ifdef DBG
#if DBG

PCHAR PnPMinorFunctionString (UCHAR MinorFunction)
{
 switch (MinorFunction)
 {
  case IRP_MN_START_DEVICE:
		return "IRP_MN_START_DEVICE";
  case IRP_MN_QUERY_REMOVE_DEVICE:
		return "IRP_MN_QUERY_REMOVE_DEVICE";
  case IRP_MN_REMOVE_DEVICE:
		return "IRP_MN_REMOVE_DEVICE";
  case IRP_MN_CANCEL_REMOVE_DEVICE:
		return "IRP_MN_CANCEL_REMOVE_DEVICE";
  case IRP_MN_STOP_DEVICE:
		return "IRP_MN_STOP_DEVICE";
  case IRP_MN_QUERY_STOP_DEVICE:
		return "IRP_MN_QUERY_STOP_DEVICE";
  case IRP_MN_CANCEL_STOP_DEVICE:
		return "IRP_MN_CANCEL_STOP_DEVICE";
  case IRP_MN_QUERY_DEVICE_RELATIONS:
		return "IRP_MN_QUERY_DEVICE_RELATIONS";
  case IRP_MN_QUERY_INTERFACE:
		return "IRP_MN_QUERY_INTERFACE";
  case IRP_MN_QUERY_CAPABILITIES:
		return "IRP_MN_QUERY_CAPABILITIES";
  case IRP_MN_QUERY_RESOURCES:
		return "IRP_MN_QUERY_RESOURCES";
  case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
		return "IRP_MN_QUERY_RESOURCE_REQUIREMENTS";
  case IRP_MN_QUERY_DEVICE_TEXT:
		return "IRP_MN_QUERY_DEVICE_TEXT";
  case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
		return "IRP_MN_FILTER_RESOURCE_REQUIREMENTS";
  case IRP_MN_READ_CONFIG:
		return "IRP_MN_READ_CONFIG";
  case IRP_MN_WRITE_CONFIG:
		return "IRP_MN_WRITE_CONFIG";
  case IRP_MN_EJECT:
		return "IRP_MN_EJECT";
  case IRP_MN_SET_LOCK:
		return "IRP_MN_SET_LOCK";
  case IRP_MN_QUERY_ID:
		return "IRP_MN_QUERY_ID";
  case IRP_MN_QUERY_PNP_DEVICE_STATE:
		return "IRP_MN_QUERY_PNP_DEVICE_STATE";
  case IRP_MN_QUERY_BUS_INFORMATION:
		return "IRP_MN_QUERY_BUS_INFORMATION";
  case IRP_MN_DEVICE_USAGE_NOTIFICATION:
		return "IRP_MN_DEVICE_USAGE_NOTIFICATION";
  case IRP_MN_SURPRISE_REMOVAL:
		return "IRP_MN_SURPRISE_REMOVAL";
  default:
		return "IRP_MN_?????";
 }
}

PCHAR DbgDeviceRelationString (IN DEVICE_RELATION_TYPE Type)
{  
 switch (Type)
 {
  case BusRelations:
		return "BusRelations";
  case EjectionRelations:
		return "EjectionRelations";
  case RemovalRelations:
		return "RemovalRelations";
  case TargetDeviceRelation:
		return "TargetDeviceRelation";
  default:
		return "UnKnown Relation";
 }
}

PCHAR DbgDeviceIDString (BUS_QUERY_ID_TYPE Type)
{
 switch (Type)
 {
  case BusQueryDeviceID:
		return "BusQueryDeviceID";
  case BusQueryHardwareIDs:
		return "BusQueryHardwareIDs";
  case BusQueryCompatibleIDs:
		return "BusQueryCompatibleIDs";
  case BusQueryInstanceID:
		return "BusQueryInstanceID";
  case BusQueryDeviceSerialNumber:
		return "BusQueryDeviceSerialNumber";
  default:
		return "UnKnown ID";
 }
}

#endif
#endif
