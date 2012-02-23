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
/**  JoyPnP.c   Routine to handle PnP IRPs for the joystick PDOs          **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "PPJoyBus.h"
#include <wchar.h>

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE,PPJoyBus_PnP_Joy)
    #pragma alloc_text (PAGE,PPJoyBus_JoyQueryIDs)
    #pragma alloc_text (PAGE,PPJoyBus_JoyQueryDevText)
    #pragma alloc_text (PAGE,PPJoyBus_JoyDevRelations)
    #pragma alloc_text (PAGE,PPJoyBus_JoyQueryCaps)
    #pragma alloc_text (PAGE,PPJoyBus_JoyQueryResourceRequirements)
    #pragma alloc_text (PAGE,PPJoyBus_JoyStartDevice)
    #pragma alloc_text (PAGE,PPJoyBus_JoyStopDevice)
#endif



/**************************************************************************/
/* Main routine to handle PnP IRPs for the joystick PDOs. We will try to  */
/* to handle most of it ourselves; otherwise pass it on to helpers.       */
/**************************************************************************/
NTSTATUS PPJoyBus_PnP_Joy (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PIO_STACK_LOCATION	IrpStack;
 PJOY_DEVICE_DATA	JoyDeviceData;

 WCHAR				LinkNameBuffer[128];
 UNICODE_STRING		LinkName;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_PNP|PPJOY_FENTRY,("PPJoyBus_PnP_Joy(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 IrpStack = IoGetCurrentIrpStackLocation (Irp);
 JoyDeviceData= (PJOY_DEVICE_DATA)DeviceObject->DeviceExtension;

 switch(IrpStack->MinorFunction)
 {
  case IRP_MN_START_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_START_DEVICE") );

	    ntStatus= PPJoyBus_JoyStartDevice (JoyDeviceData,Irp);
		goto CompleteIRP;

  case IRP_MN_STOP_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_STOP_DEVICE") );

	    PPJoyBus_JoyStopDevice (JoyDeviceData,Irp);
		/* This routine must always return success. */
		ntStatus= STATUS_SUCCESS;
		goto CompleteIRP;

  case IRP_MN_QUERY_STOP_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_QUERY_STOP_DEVICE") );

		/* Always accept stop device request */
		ntStatus= STATUS_SUCCESS;
		goto CompleteIRP;

  case IRP_MN_CANCEL_STOP_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_CANCEL_STOP_DEVICE") );

		/* Canceling stop requests always successful */
		ntStatus= STATUS_SUCCESS;
		goto CompleteIRP;

  case IRP_MN_QUERY_REMOVE_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_QUERY_REMOVE_DEVICE") );

		/* Always accept remove requests */
		ntStatus= STATUS_SUCCESS;
		goto CompleteIRP;

  case IRP_MN_CANCEL_REMOVE_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_CANCEL_REMOVE_DEVICE") );

		/* Cancelling remove requests always successful */
		ntStatus= STATUS_SUCCESS;
		goto CompleteIRP;


  case IRP_MN_QUERY_CAPABILITIES:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_QUERY_CAPABILITIES") );
 
		ntStatus= PPJoyBus_JoyQueryCaps (JoyDeviceData,Irp);
		goto CompleteIRP;
 
  case IRP_MN_QUERY_DEVICE_RELATIONS:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_QUERY_DEVICE_RELATIONS type %s",DbgDeviceRelationString(IrpStack->Parameters.QueryDeviceRelations.Type)) );
 
		ntStatus= PPJoyBus_JoyDevRelations (JoyDeviceData,Irp);
		goto CompleteIRP;

  case IRP_MN_QUERY_RESOURCES:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_QUERY_RESOURCES") );

		/* Simply copy the status of the incoming IRP to return back */
		ntStatus= Irp->IoStatus.Status;
		goto CompleteIRP;
	
#if 0
  case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_QUERY_RESOURCE_REQUIREMENTS") );

		ntStatus= PPJoyBus_JoyQueryResourceRequirements (JoyDeviceData,Irp);
		goto CompleteIRP;
#endif

  case IRP_MN_DEVICE_USAGE_NOTIFICATION:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_DEVICE_USAGE_NOTIFICATION") );
		ntStatus= STATUS_UNSUCCESSFUL;
		goto CompleteIRP;

  case IRP_MN_SURPRISE_REMOVAL:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_SURPRISE_REMOVAL") );

		/* Simply say request was successful */
		ntStatus= STATUS_SUCCESS;
		goto CompleteIRP;

  case IRP_MN_REMOVE_DEVICE:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_REMOVE_DEVICE") );

		/* We stop the device if it wasn't done before the remove */
		if (JoyDeviceData->Flags&PPJFLAGS_STARTED)
		 PPJoyBus_JoyStopDevice (JoyDeviceData,Irp);

		/* Set the DeviceExtension flag to say that our device is removed */
		JoyDeviceData->Flags|= PPJFLAGS_REMOVED;

		/* Leave request to decrease requests counter. Event should be */
		/* signaled when last request leaves. Then we will return a    */
		/* successful status.                                          */
		PPJoyBus_LeaveRequest((PCOMMON_DATA)JoyDeviceData);

        KeWaitForSingleObject (&JoyDeviceData->RemoveEvent,Executive,KernelMode,FALSE,NULL);
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE2, ("KeWaitForSingleObject on RemoveEvent completed.") );

		/* If our Joystick is unplugged (via control panel applet) then we */
		/* delete the device object for it as well. Else we only mark it   */
		/* as being removed. (Can later be AddDevice and START'ed again)   */
		if (JoyDeviceData->Flags&PPJFLAGS_UNPLUGGED)
		{
		 /* Remove the PDO from joystick list. */
		 ExAcquireFastMutex (&Globals.Mutex);
		 RemoveEntryList (&JoyDeviceData->ListEntry);
		 ExReleaseFastMutex (&Globals.Mutex);

 		 PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE2, ("Device unplugged so we will delete the PDO.") );

		 /* Then nuke the PDO out of existance */
		 IoDeleteDevice (JoyDeviceData->Self);
		}

		/* The hardware is still physically present. We are ready to be */
		/* started again - so clear the REMOVED flag. Should work.      */
		JoyDeviceData->Flags&= ~PPJFLAGS_REMOVED;

		/* After all that indicate that the request was successful */
		ntStatus= STATUS_SUCCESS;
		goto CompleteIRP;

  case IRP_MN_QUERY_ID:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_QUERY_ID type %s",DbgDeviceIDString(IrpStack->Parameters.QueryId.IdType)) );
	
		ntStatus= PPJoyBus_JoyQueryIDs (JoyDeviceData,Irp);
		goto CompleteIRP;

  case IRP_MN_QUERY_DEVICE_TEXT:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_PnP_Joy: IRP_MN_QUERY_DEVICE_TEXT") );
	
		ntStatus= PPJoyBus_JoyQueryDevText (JoyDeviceData,Irp);
		goto CompleteIRP;

  default:
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_WARN,("PPJoyBus_PnP_Joy: IrpStack->MinorFunction Not handled %s (0x%x)",PnPMinorFunctionString (IrpStack->MinorFunction),IrpStack->MinorFunction) );

		/* Simply copy the status of the incoming IRP to return back */
		ntStatus= Irp->IoStatus.Status;
		goto CompleteIRP;
 }

CompleteIRP:
 Irp->IoStatus.Status = ntStatus;
 IoCompleteRequest (Irp, IO_NO_INCREMENT);
 goto Exit;

Exit:
 PPJOY_EXITPROC (FILE_PNP|PPJOY_FEXIT, "PPJoyBus_PnP_Joy",ntStatus );
 return ntStatus;
}

/**************************************************************************/
/* Routine to return the hardware ID of the joystick device. We will      */
/* report the same IDs for all devices.                                   */
/**************************************************************************/
NTSTATUS PPJoyBus_JoyQueryIDs (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp)
{
 NTSTATUS		ntStatus;
 PWCHAR			Buffer;
    
 PAGED_CODE ();

 ntStatus= STATUS_SUCCESS;

 switch (IoGetCurrentIrpStackLocation (Irp)->Parameters.QueryId.IdType)
 {
  case BusQueryCompatibleIDs:
  case BusQueryHardwareIDs:
  case BusQueryDeviceID:
		/* Allocate buffer in which to return the hardware ID. Make VERY */
		/* sure the buffer is large enough!!! Must be larger than the    */
		/* the result of the swprintf below.                             */
		Buffer= ExAllocatePoolWithTag (PagedPool,128,PPJOYBUS_POOL_TAG);
		if (!Buffer)
		{
		 ntStatus= STATUS_INSUFFICIENT_RESOURCES;
		 break;
		}
		/* Create Device ID from index in joystick list. */
		RtlZeroMemory (Buffer,128);
		swprintf(Buffer, LJOY_DEVICE_PNP_ID ,JoyDeviceData->Config.VendorID,JoyDeviceData->Config.ProductID);

		/* Return our hardware ID */
		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_JoyQueryIDs: Device ID is %S",Buffer) );
		Irp->IoStatus.Information= (ULONG_PTR) Buffer;
		break;

  case BusQueryInstanceID:
		/* Allocate buffer in which to return the hardware ID. Make VERY */
		/* sure the buffer is large enough!!! Must be larger than the    */
		/* the result of the swprintf below.                             */
		Buffer= ExAllocatePoolWithTag (PagedPool,128,PPJOYBUS_POOL_TAG);
		if (!Buffer)
		{
		 ntStatus= STATUS_INSUFFICIENT_RESOURCES;
		 break;
		}
		/* Create Instance ID from port and joystick type. */
//		swprintf(Buffer, L"PPJoy%08X%08X",JoyDeviceData->JoyData.Port,JoyDeviceData->JoyData.SubAddr);
		swprintf(Buffer,LJOY_DEVINSTANCE_NAME);

		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE2, ("PPJoyBus_JoyQueryIDs: Instance ID is %S",Buffer) );
		Irp->IoStatus.Information= (ULONG_PTR) Buffer;
		break;

  default:
		ntStatus= Irp->IoStatus.Status;
		break;
 }
 return ntStatus;
}

/**************************************************************************/
/* Routine to return textual device and location descriptions.            */
/**************************************************************************/
NTSTATUS PPJoyBus_JoyQueryDevText (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp)
{
 NTSTATUS	ntStatus;
 PWCHAR		Buffer;

 PAGED_CODE ();

 /* Determine what to do with IRP */
 switch (IoGetCurrentIrpStackLocation (Irp)->Parameters.QueryDeviceText.DeviceTextType)
 {
  case DeviceTextDescription:

		PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("PPJoyBus_JoyQueryIDs: PPJOYBUS_DEV_NAME_LEN= %d",PPJOYBUS_DEV_NAME_LEN) );
	  
	    /* We ignore the locale ID; only return information if no one else */
		/* has done so before us.                                          */
		if (!Irp->IoStatus.Information)
		{
		 /* Allocate buffer for returned text. Make sure it large enough!! */
		 Buffer= ExAllocatePoolWithTag (PagedPool,PPJOYBUS_DEV_NAME_LEN,PPJOYBUS_POOL_TAG);
		 if (Buffer==NULL)
		 {
		  ntStatus= STATUS_INSUFFICIENT_RESOURCES;
		  break;
		 }
		 /* Return our canned device name */
		 RtlCopyMemory (Buffer,PPJOYBUS_DEV_NAME,PPJOYBUS_DEV_NAME_LEN);
		 Irp->IoStatus.Information= (UINT_PTR) Buffer;
		}
		ntStatus= STATUS_SUCCESS;
		break;           

  case DeviceTextLocationInformation:
		/* We ignore the locale ID; only return information if no one else */
		/* has done so before us.                                          */
		if (!Irp->IoStatus.Information)
		{
		 /* Allocate buffer for returned text. Make sure it large enough!! */
		 Buffer= ExAllocatePoolWithTag (PagedPool,256,PPJOYBUS_POOL_TAG);
		 if (!Buffer)
		 {
		  ntStatus= STATUS_INSUFFICIENT_RESOURCES;
		  break;
		 }
		 if (JoyDeviceData->Config.LPTNumber)
		 {
		  swprintf(Buffer,L"LPT%d controller %d: type %d, subtype %d",
					 JoyDeviceData->Config.LPTNumber,JoyDeviceData->Config.UnitNumber+1,
					 JoyDeviceData->Config.JoyType,JoyDeviceData->Config.JoySubType);
		 }
		 else
		 {
		  swprintf(Buffer,L"Virtual controller %d: type %d, subtype %d",
					 JoyDeviceData->Config.UnitNumber+1,
					 JoyDeviceData->Config.JoyType,JoyDeviceData->Config.JoySubType);
		 }

		 PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE2, ("PPJoyBus_JoyQueryDevText: Location is: %S",Buffer) );
		 Irp->IoStatus.Information= (UINT_PTR) Buffer;
		}
		ntStatus= STATUS_SUCCESS;
		break;           

  default:
		ntStatus= Irp->IoStatus.Status;
		break;
 } 

 return ntStatus;
}

/**************************************************************************/
/* Routine to return device relation. Return a simple relation.           */
/**************************************************************************/
NTSTATUS PPJoyBus_JoyDevRelations (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp)
{
 PDEVICE_RELATIONS	DeviceRelations;
 NTSTATUS			ntStatus;

 PAGED_CODE ();

 switch (IoGetCurrentIrpStackLocation(Irp)->Parameters.QueryDeviceRelations.Type)
 {
  case TargetDeviceRelation:  
		DeviceRelations= (PDEVICE_RELATIONS) ExAllocatePoolWithTag (PagedPool,sizeof(DEVICE_RELATIONS),PPJOYBUS_POOL_TAG);
		if (!DeviceRelations)
		{
		 ntStatus= STATUS_INSUFFICIENT_RESOURCES;
		 break;
		}
		DeviceRelations->Count= 1;
		DeviceRelations->Objects[0]= JoyDeviceData->Self;
		ObReferenceObject (JoyDeviceData->Self);

		ntStatus = STATUS_SUCCESS;
		Irp->IoStatus.Information= (ULONG_PTR) DeviceRelations;
		break;
        
  case BusRelations:
  case EjectionRelations:
  case RemovalRelations:
  default:
		ntStatus= Irp->IoStatus.Status;
 }

 return ntStatus;
}

/**************************************************************************/
/* Routine to return capabilities of this driver.                         */
/**************************************************************************/
NTSTATUS PPJoyBus_JoyQueryCaps (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp)
{
 PIO_STACK_LOCATION		Stack;
 PDEVICE_CAPABILITIES	DevCaps;
 DEVICE_CAPABILITIES	parentCapabilities;
 NTSTATUS				ntStatus;
    
 PAGED_CODE ();

 Stack= IoGetCurrentIrpStackLocation (Irp);
 DevCaps= Stack->Parameters.DeviceCapabilities.Capabilities;

 /* Make sure we know how to handle this version of the structure. */
 if ((DevCaps->Version<1) || (DevCaps->Size<sizeof(DEVICE_CAPABILITIES)) )
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_WARN,("PPJoyBus_JoyQueryCaps: We don't understand structure version %d (size %d) - was expecting version 1, size %d",DevCaps->Version,DevCaps->Size,sizeof(DEVICE_CAPABILITIES)) );
  if (DevCaps->Version||DevCaps->Size)
   return STATUS_UNSUCCESSFUL; 

  /* I hate Win98. */
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_WARN,("PPJoyBus_JoyQueryCaps: Performing workaround for Windows 9X. Manually setting version and size values") );
   
  /* Set the capabilities structure size and version. See GAMEENUM sample */
  /* in the Windows 98 and Windows 2000 DDKs.                             */
  DevCaps->Version = 1;
  DevCaps->Size= sizeof (DEVICE_CAPABILITIES);
 }

 /* Set our device powerstates that correspond to system power states */
 DevCaps->DeviceState[PowerSystemWorking]= PowerDeviceD0;
 DevCaps->DeviceState[PowerSystemSleeping1]= PowerDeviceD3;
 DevCaps->DeviceState[PowerSystemSleeping2]= PowerDeviceD3;
 DevCaps->DeviceState[PowerSystemSleeping3]= PowerDeviceD3;
 DevCaps->DeviceState[PowerSystemHibernate]= PowerDeviceD3;
 DevCaps->DeviceState[PowerSystemShutdown]= PowerDeviceD3;
 
 /* We don't suport D1 and D2 power states. Set explicitly. */
 DevCaps->DeviceD1= FALSE;
 DevCaps->DeviceD2= FALSE;

 /* Cannot send wakeup signals. */
 DevCaps->WakeFromD0= FALSE;
 DevCaps->WakeFromD1= FALSE;
 DevCaps->WakeFromD2= FALSE;
 DevCaps->WakeFromD3= FALSE;
 DevCaps->SystemWake= PowerSystemUnspecified;
 DevCaps->DeviceWake= PowerDeviceUnspecified;
 
 /* We have no latencies */
 DevCaps->D1Latency= 0;
 DevCaps->D2Latency= 0;
 DevCaps->D3Latency= 0;

 /* Eject and lock not supported */
 DevCaps->EjectSupported= FALSE;
 DevCaps->LockSupported= FALSE;

 DevCaps->Removable= FALSE;
 DevCaps->SurpriseRemovalOK= TRUE;

 /* not Docking device */
 DevCaps->DockDevice = FALSE;

 /* Our IDs should be unique across the system (I hope!). */
 DevCaps->UniqueID= TRUE;

 /* We don't set this - otherwise "Location x" is show in the location field */
 /* of Device Manager GUI in addition to DeviceTextLocationInformation text. */
 /* DevCaps->UINumber= 0; */

 return STATUS_SUCCESS;
}

/**************************************************************************/
/* Routine to return a resource requirements list.                        */
/**************************************************************************/
NTSTATUS PPJoyBus_JoyQueryResourceRequirements (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp)
{
 PIO_RESOURCE_REQUIREMENTS_LIST		pResourceList;
 PIO_RESOURCE_DESCRIPTOR			pDescriptor;

 PAGED_CODE ();

 /* Allocate storage for the resource requirement list */
 pResourceList= ExAllocatePoolWithTag (PagedPool,sizeof(IO_RESOURCE_REQUIREMENTS_LIST),PPJOYBUS_POOL_TAG);
 if (pResourceList==NULL)
  return STATUS_INSUFFICIENT_RESOURCES;

 /* Initialise the list */
 RtlZeroMemory (pResourceList,sizeof(IO_RESOURCE_REQUIREMENTS_LIST));
 pResourceList->ListSize= sizeof(IO_RESOURCE_REQUIREMENTS_LIST);

 /* Fill in the list - copied from DDK sample */
 pResourceList->AlternativeLists= 1;
 pResourceList->List[0].Version= 1;
 pResourceList->List[0].Revision= 1;
 pResourceList->List[0].Count= 0;

 /* Return pointer to the allocated list */
 Irp->IoStatus.Information= (ULONG_PTR) pResourceList;
 return STATUS_SUCCESS;
}

/**************************************************************************/
/* Routine to start a joystick PDO.                                       */
/**************************************************************************/
NTSTATUS PPJoyBus_JoyStartDevice (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp)
{
 NTSTATUS	ntStatus;

 PAGED_CODE ();

 /* Make sure we don't start when already started. */
 if (JoyDeviceData->Flags&PPJFLAGS_STARTED)
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("Already started, request ignored") );
  return STATUS_SUCCESS;
 }

 /* Set the device started flag */
 JoyDeviceData->Flags|= PPJFLAGS_STARTED;

 /* For virtual joysticks we simply return success */
 if (JoyDeviceData->Config.LPTNumber==0)
  return STATUS_SUCCESS;

 ntStatus= PPJoyBus_ParPortAlloc (JoyDeviceData->Config.LPTNumber);
 if (!NT_SUCCESS (ntStatus))
 {
  PPJoyBus_WriteEventLog (PPJ_MSG_ERRORALLOCPORT,&ntStatus,sizeof(ntStatus),L"");
 }

 /* Allocate the parallel port we allocated for this device */
 return ntStatus;
}

/**************************************************************************/
/* Routine to stop a joystick PDO.                                        */
/**************************************************************************/
NTSTATUS PPJoyBus_JoyStopDevice (IN PJOY_DEVICE_DATA JoyDeviceData, IN PIRP Irp)
{
 PAGED_CODE ();

 /* Make sure we don't stop when already stopped. */
 if (!(JoyDeviceData->Flags&PPJFLAGS_STARTED))
 {
  PPJOY_DBGPRINT (FILE_PNP|PPJOY_BABBLE, ("Already stopped, request ignored") );
  return STATUS_SUCCESS;
 }

 /* Clear the device started flag */
 JoyDeviceData->Flags&= ~PPJFLAGS_STARTED;

 /* For virtual joysticks we simply return success */
 if (JoyDeviceData->Config.LPTNumber==0)
  return STATUS_SUCCESS;

 PPJoyBus_ParFreeIRQ (JoyDeviceData->Config.LPTNumber,&JoyDeviceData->IRQ);

 /* Free the parallel port we allocated for this device */
 return PPJoyBus_ParPortFree (JoyDeviceData->Config.LPTNumber);
}
