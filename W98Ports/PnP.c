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
/**  BusPnP.c   Routine to handle PnP IRPs for the FDO                    **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "W98Ports.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE,W98Ports_PnP)
    #pragma alloc_text (PAGE,W98Ports_StartDevice)
    #pragma alloc_text (PAGE,W98Ports_StopDevice)
    #pragma alloc_text (PAGE,W98Ports_RemoveDevice)
#endif



/**************************************************************************/
/* Main routine to handle PnP IRPs for the bus enumerator. We will try to */
/* to handle most of it ourselves; otherwise pass it on to helpers.       */
/**************************************************************************/
NTSTATUS W98Ports_DoPnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PIO_STACK_LOCATION	IrpStack;
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_PNP|PPJOY_FENTRY, ("W98Ports_PnP (DeviceObject=0x%x,Irp=0x%x)",DeviceObject,Irp) );

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

 IrpStack = IoGetCurrentIrpStackLocation (Irp);
 switch(IrpStack->MinorFunction)
 {
  case IRP_MN_START_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("W98Ports_PnP: IRP_MN_START_DEVICE") );

		ntStatus= W98Ports_SendIrpSynchronously (DeviceExtension->NextLowerDriver,Irp);
		if (NT_SUCCESS(ntStatus))
         ntStatus= W98Ports_StartDevice (DeviceObject,Irp);

		/* We must now complete the IRP */ 
		goto CompleteIRP;

  case IRP_MN_STOP_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("W98Ports_PnP: IRP_MN_STOP_DEVICE") );

		W98Ports_StopDevice (DeviceObject,Irp);
		Irp->IoStatus.Status= STATUS_SUCCESS;
		goto PassOnIRP;  

  case IRP_MN_QUERY_STOP_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("W98Ports_PnP: IRP_MN_QUERY_STOP_DEVICE") );

		/* We will (for now) always allow the device to be stopped. */
		Irp->IoStatus.Status= STATUS_SUCCESS;
		goto PassOnIRP;  

  case IRP_MN_CANCEL_STOP_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("W98Ports_PnP: IRP_MN_CANCEL_STOP_DEVICE") );

		/* Can always cancel the pending stop query */
		Irp->IoStatus.Status= STATUS_SUCCESS;
		goto PassOnIRP;  

  case IRP_MN_QUERY_REMOVE_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("W98Ports_PnP: IRP_MN_QUERY_REMOVE_DEVICE") );

		/* We will always allow the device to be removed. */
		Irp->IoStatus.Status= STATUS_SUCCESS;
		goto PassOnIRP;  

  case IRP_MN_CANCEL_REMOVE_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("W98Ports_PnP: IRP_MN_CANCEL_REMOVE_DEVICE") );

		/* Can always cancel the pending remove query */
		Irp->IoStatus.Status= STATUS_SUCCESS; // We must not fail the IRP.
		goto PassOnIRP;  

  case IRP_MN_SURPRISE_REMOVAL:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("W98Ports_PnP: IRP_MN_SURPRISE_REMOVAL") );

		/* Should we perhaps now delete all Joystick PDOs belonging to */
		/* this bus? For now we don't. Check later...                  */

		/* We are not allowed to fail this IRP */
		Irp->IoStatus.Status= STATUS_SUCCESS;
		goto PassOnIRP;  

  case IRP_MN_REMOVE_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("W98Ports_PnP: IRP_MN_REMOVE_DEVICE") );

		/* We stop the device if it wasn't done before the remove */
		if (DeviceExtension->Flags&W98PFLAGS_STARTED)
		 W98Ports_StopDevice (DeviceObject,Irp);

		/* Set the DeviceExtension flag to say that our device is removed */
		DeviceExtension->Flags|= W98PFLAGS_REMOVED;

		/* Set status success in this IRP before passing down. Else Driver */
		/* Verifier moans about it. Strange?!                              */
		Irp->IoStatus.Status= STATUS_SUCCESS;

		/* Pass call to lower driver */
		IoSkipCurrentIrpStackLocation (Irp);
		ntStatus= IoCallDriver (DeviceExtension->NextLowerDriver,Irp);
		
		/* Leave request to decrease requests counter. Event should be */
		/* signaled when last request leaves. Then we will return a    */
		/* successful status.                                          */
		W98Ports_LeaveRequest(DeviceObject);

        KeWaitForSingleObject (&DeviceExtension->RemoveEvent,Executive,KernelMode,FALSE,NULL);
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE2, ("KeWaitForSingleObject on RemoveEvent completed.") );

		/* We will now cleanup the bus. Remove any joystick PDOs still */
		/* present.                                                    */
		W98Ports_RemoveDevice (DeviceObject,Irp);

		/* We are not allowed to fail this IRP */
		ntStatus= STATUS_SUCCESS;
		goto Exit;

  case IRP_MN_QUERY_DEVICE_RELATIONS:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("W98Ports_PnP: IRP_MN_QUERY_DEVICE_RELATIONS") );

		/* Check to see if this is a request for BusRelations */
		if (IrpStack->Parameters.QueryDeviceRelations.Type!=BusRelations)
		{
		 /* Since we don't support any other type of relation, pass it on. */
		 PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("QueryDeviceRelation Type: %s ignored!",DbgDeviceRelationString(IrpStack->Parameters.QueryDeviceRelations.Type)) );
		 goto PassOnIRP;  
		}

		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("Old relations structure: 0x%X",Irp->IoStatus.Information) );

		if (!Irp->IoStatus.Information)
		{
		 PDEVICE_RELATIONS		NewRelations;

		 PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("Allocating a new relations structure. Just to keep everybody happy") );
			
		 NewRelations= (PDEVICE_RELATIONS) ExAllocatePoolWithTag (PagedPool,sizeof(DEVICE_RELATIONS),W98PORTS_POOL_TAG);
		 if (NewRelations==NULL)
		 {
		  /* Fail the IRP if the allocation fails. */
		  ntStatus= STATUS_INSUFFICIENT_RESOURCES;
		  goto CompleteIRP;
		 }
		 NewRelations->Count= 0;
		 Irp->IoStatus.Status= STATUS_SUCCESS;
		}	  

		/* Otherwise allow other drivers to modify the IRP */
		goto PassOnIRP;  

  default:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_WARN, ("W98Ports_PnP: IrpStack->MinorFunction Not handled %s (0x%x)",PnPMinorFunctionString(IrpStack->MinorFunction),IrpStack->MinorFunction) );

		/* If we don't know what to do with the IRP then pass it on... */
		goto PassOnIRP;  
 }

PassOnIRP:
 IoSkipCurrentIrpStackLocation (Irp);
 ntStatus= IoCallDriver (DeviceExtension->NextLowerDriver,Irp);
 goto Exit;

CompleteIRP:
 Irp->IoStatus.Status= ntStatus;
 IoCompleteRequest (Irp,IO_NO_INCREMENT);
 goto Exit;

Exit:

 PPJOY_EXITPROC (FILE_PNP|PPJOY_FEXIT, "W98Ports_PnP",ntStatus );
 return ntStatus;
}

/**************************************************************************/
/* Routine to handle start requests for the bus enumerator FDO. Register  */
/* for PnP notification and enable our interface for IOCTL requests       */
/**************************************************************************/
NTSTATUS W98Ports_StartDevice  (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE();

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

 if (DeviceExtension->Flags&W98PFLAGS_STARTED)
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("Bus device already started, ignoring request") );
  return STATUS_SUCCESS;
 }

 /* Set flag to indicate that we are successfully started */
 DeviceExtension->Flags|= W98PFLAGS_STARTED;

 /* Don't know if we perhaps need to explicitly map memory to the BIOS data */
 /* segment; or get the printer port base address in some other way.        */
 DeviceExtension->BaseAddress= *( (USHORT*)(ULONG_PTR)(0x406+(DeviceExtension->LPTNumber*2)) );
 PPJOY_DBGPRINT (FILE_W98PORTS|PPJOY_BABBLE2, ("Port LPT%d base address 0x%X",DeviceExtension->LPTNumber,DeviceExtension->BaseAddress) );

 /* Enable our device interface if we have an LPT Number and port. */
 if (DeviceExtension->LPTNumber&&DeviceExtension->BaseAddress)
 {
  ntStatus= IoSetDeviceInterfaceState(&DeviceExtension->InterfaceName,TRUE);
  if (!NT_SUCCESS (ntStatus))
  {
   PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("IoSetDeviceInterfaceState failed: 0x%x\n",ntStatus) );
  }
  return ntStatus;
 }

 return STATUS_UNSUCCESSFUL;
}

/**************************************************************************/
/* Routine to handle stop requests for the bus enumerator FDO. Unregister */
/* the PnP notifications and disable our interface for IOCTL requests     */
/**************************************************************************/
NTSTATUS W98Ports_StopDevice  (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE();

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

 if (!(DeviceExtension->Flags&W98PFLAGS_STARTED))
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("Bus device already stopped, ignoring request") );
  return STATUS_SUCCESS;
 }

 /* Disable our device interface; Ignore any error that occurs. */
 ntStatus= IoSetDeviceInterfaceState(&DeviceExtension->InterfaceName,FALSE);
 if (!NT_SUCCESS (ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_ERROR, ("IoSetDeviceInterfaceState failed: 0x%x\n",ntStatus) );
 }

 /* Clear the started flag for our Device Object */
 DeviceExtension->Flags&= ~W98PFLAGS_STARTED;

 return STATUS_SUCCESS;
}

/**************************************************************************/
/* Routine to handle removal requests for this device. Free any           */
/* per-device resource allocations, delete and remaining Joystick PDOs    */
/* then delete our DeviceObject                                           */
/**************************************************************************/
NTSTATUS W98Ports_RemoveDevice  (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE();

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

 /* Get out of the chain of command. */
 IoDetachDevice (DeviceExtension->NextLowerDriver);

 /* Free resources in the DeviceExtension */
 if (DeviceExtension->InterfaceName.Buffer)
 {
  /* Set interface state to FALSE, just in case... */
  IoSetDeviceInterfaceState (&DeviceExtension->InterfaceName,FALSE);
  /* Free memory allocated by IoRegisterDeviceInterface in AddDevice */
  RtlFreeUnicodeString (&DeviceExtension->InterfaceName);
  RtlZeroMemory (&DeviceExtension->InterfaceName,sizeof (UNICODE_STRING)); 
 }

 /* Finally delete our own Device Object. */
 IoDeleteDevice (DeviceObject);

 return STATUS_SUCCESS;
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
  default:
		return "UnKnown ID";
 }
}

#endif
#endif
