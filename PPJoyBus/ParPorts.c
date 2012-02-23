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
/**  Parallel port joystick bus driver, (C) Deon van der Westhuysen 2002  **/
/**                                                                       **/
/**  Debug.h    Debug support routines for the Parallel Port Joystick     **/
/**             Bus driver.                                               **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/



#include "PPJoyBus.h"

/* Included for ECP Port ChipMode constants (ECR_*) */
#include <parallel.h>
/* Included to define GUIDs for the parallel interface and PnP notification */
#include <initguid.h>
#include <ntddpar.h>
#include <wdmguid.h>


#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE,PPJoyBus_SendParPortIoctl)
    #pragma alloc_text (PAGE,PPJoyBus_RegisterNotifications)
    #pragma alloc_text (PAGE,PPJoyBus_UnregisterNotifications)
    #pragma alloc_text (PAGE,PPJoyBus_GetLptNumber)
    #pragma alloc_text (PAGE,PPJoyBus_PnPArrival)
    #pragma alloc_text (PAGE,PPJoyBus_PnPRemoval)
    #pragma alloc_text (PAGE,PPJoyBus_PnPNotification)
    #pragma alloc_text (PAGE,PPJoyBus_ParPortAlloc)
    #pragma alloc_text (PAGE,PPJoyBus_ParPortFree)
    #pragma alloc_text (PAGE,PPJoyBus_ParSetChipMode)
    #pragma alloc_text (PAGE,PPJoyBus_ManualSetChipMode)

#endif /* ALLOC_PRAGMA */



/**************************************************************************/
/* Routine that send Internal IOCTL requests to the parport.sys driver.   */
/* This routine will return after TimeoutSecs seconds or (if negative)    */
/* until the request completes.                                           */
/**************************************************************************/
NTSTATUS PPJoyBus_SendParPortIoctl (IN PDEVICE_OBJECT DeviceObject, IN ULONG IoControlCode,
									IN PVOID InputBuffer OPTIONAL, IN ULONG InputBufferLength,
									OUT PVOID OutputBuffer OPTIONAL, IN ULONG OutputBufferLength,
									IN LONG TimeoutSecs)
{
 NTSTATUS			ntStatus;
 KEVENT				Event;
 PIRP				Irp;
 IO_STATUS_BLOCK	IoStatus;
 LARGE_INTEGER		Timeout;

 PAGED_CODE ();

 /* Initialise the event for notification when the IRP completes */
 KeInitializeEvent (&Event,NotificationEvent,FALSE);

 /* Construct the IO request for the parport.sys driver. */
 Irp= IoBuildDeviceIoControlRequest (IoControlCode,DeviceObject,InputBuffer,InputBufferLength,
									 OutputBuffer,OutputBufferLength,TRUE,&Event,&IoStatus);

 if (!Irp)
  return STATUS_INSUFFICIENT_RESOURCES;
    
 /* Call the parport.sys driver with the request. */
 ntStatus= IoCallDriver(DeviceObject,Irp);

 /* If we get an error we return straight away... STATUS_PENDING is not */
 /* an error according to NT_SUCCESS macro.                             */
 if (!NT_SUCCESS(ntStatus))
  return ntStatus;

 /* Timeout are in 100ns units! */
 Timeout.QuadPart= -((LONGLONG) TimeoutSecs*10*1000*1000);

 /* Wait for the event to be signaled by the completion routine. */
 ntStatus= KeWaitForSingleObject (&Event,Executive,KernelMode,FALSE,TimeoutSecs<0?NULL:&Timeout);

 if (ntStatus==STATUS_TIMEOUT)
 {
  /* If we get a timeout, cancel the outstanding IRP */
  IoCancelIrp (Irp);
  /* Then wait for the completion routine to be called */
  KeWaitForSingleObject (&Event,Executive,KernelMode,FALSE,NULL);
 }
 else if (!NT_SUCCESS(ntStatus))
 {
  /* if not STATUS_SUCCESS, return the status from KeWaitForSingleObject */
  return ntStatus;
 }

 /* Return status from the IRP. */
 return IoStatus.Status;
}

/**************************************************************************/
/* Routine to register for Plug'nPlay notifications when parallel port    */
/* interfaces arrive and departs. This routine is called when the Bus     */
/* enumerator is started.                                                 */
/**************************************************************************/
NTSTATUS PPJoyBus_RegisterNotifications (IN PBUS_DEVICE_DATA BusDeviceData)
{
 /* Check to make sure haven't registered before... */
 if (BusDeviceData->NotificationHandle)
  return STATUS_SUCCESS;

 return IoRegisterPlugPlayNotification (EventCategoryDeviceInterfaceChange,
										PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
                                        (PVOID)&GUID_PARALLEL_DEVICE,
                                        BusDeviceData->DriverObject,
                                        (PDRIVER_NOTIFICATION_CALLBACK_ROUTINE)PPJoyBus_PnPNotification,
                                        (PVOID)BusDeviceData->Self,&BusDeviceData->NotificationHandle);
}

/**************************************************************************/
/* Routine to unregister the Plug'nPlay notifications. This routine is    */
/* called when the Bus enumerator is stopped                              */
/**************************************************************************/
void PPJoyBus_UnregisterNotifications (IN PBUS_DEVICE_DATA BusDeviceData)
{
 if (BusDeviceData->NotificationHandle)
  IoUnregisterPlugPlayNotification (BusDeviceData->NotificationHandle);
 BusDeviceData->NotificationHandle= NULL;
}

/**************************************************************************/
/* Routine to parse the LPT name into an number. The return value is x    */
/* for LPTx or 0 if the name is not in the format LPTx                    */
/**************************************************************************/
ULONG PPJoyBus_GetLptNumber (IN PWCHAR LptName)
{
 ULONG	Number;

 Number= 0;
 /* Make sure the name starts with LPT */
 if ((LptName[0]==L'L')&&(LptName[1]==L'P')&&(LptName[2]==L'T'))
 {
  LptName+= 3;
  /* Convert all following digits into a number. */
  while ((*LptName>=L'0')&&(*LptName<=L'9'))
   Number= Number*10+*(LptName++)-L'0';

  /* If string contains other non-digits we got an error. */
  if (*LptName)
   Number= 0;
 }
 return Number;
}

/**************************************************************************/
/* Routine to process PnP interface arrival messages. We query the port   */
/* for its parameters, save it in a global structure. Return code is LPT  */
/* port number for the interface that just arrived.                       */
/**************************************************************************/
ULONG PPJoyBus_PnPArrival (IN PDEVICE_OBJECT BusDevObject, IN PUNICODE_STRING SymLinkName)
{
 NTSTATUS					ntStatus;
 PFILE_OBJECT				FileObject;
 PDEVICE_OBJECT				DeviceObject;

 PARALLEL_PORT_INFORMATION	PortInfo;
 PARALLEL_PNP_INFORMATION	PnpInfo;

 ULONG						LPTNum;
 PPARPORTINFO				ParPortInfo;

 PAGED_CODE ();

 /* Assume no valid parallel port arrived / error status */
 LPTNum= 0;

 /* Get DeviceObject for the interface that just arrived. */	 
 ntStatus= IoGetDeviceObjectPointer (SymLinkName,STANDARD_RIGHTS_ALL,&FileObject,&DeviceObject);
 if (!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_ERROR, ("Error 0x%X calling IoGetDeviceObjectPointer()",ntStatus) );
  goto Exit;
 }

 PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_BABBLE, ("IoGetDeviceObjectPointer for %S result 0x%X",SymLinkName->Buffer,ntStatus) );

 /* Get standard parallel port information. */
 ntStatus= PPJoyBus_SendParPortIoctl (DeviceObject,IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO,NULL,0,&PortInfo,sizeof(PortInfo),-1);
 if (!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_ERROR, ("Error 0x%X sending IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO to parallel.sys driver",ntStatus) );
  goto ExitAndDealloc;
 }

 /* Get further (Plug and Play) information about this port */
 ntStatus= PPJoyBus_SendParPortIoctl (DeviceObject,IOCTL_INTERNAL_GET_PARALLEL_PNP_INFO,NULL,0,&PnpInfo,sizeof(PnpInfo),-1);
 if (!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_ERROR, ("Error 0x%X sending IOCTL_INTERNAL_GET_PARALLEL_PNP_INFO to parallel.sys driver",ntStatus) );
  goto ExitAndDealloc;
 }

 PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_BABBLE2, ("Got port info... base=0x%p, name= %S, mode 0x%X",PortInfo.Controller,PnpInfo.PortName,PnpInfo.CurrentMode) );

 LPTNum= PPJoyBus_GetLptNumber (PnpInfo.PortName);
 if (LPTNum==0)
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_WARN, ("Cannot determine LPT Number for %S",PnpInfo.PortName) );
  goto ExitAndDealloc;
 }

 PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_BABBLE2, ("LPT Number for %S is %d",PnpInfo.PortName,LPTNum) );

 if (LPTNum>MAX_PARPORTS)
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_WARN, ("LPT port %d is ignored, we are only configured for %d ports",LPTNum,MAX_PARPORTS) );
  goto ExitAndDealloc;
 }

 /* Get a pointer to global info record for this port. 0-based array! */
 ParPortInfo= &Globals.ParPorts[LPTNum-1];
 if (ParPortInfo->DevObj)
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_WARN, ("Duplicate arrival announcement, ignoring...") );
  goto ExitAndDealloc;
 }

 /* Allocate buffer to store the symbolic link name */
 ParPortInfo->SymLinkName.MaximumLength= SymLinkName->Length+sizeof(UNICODE_NULL);
 ParPortInfo->SymLinkName.Length= SymLinkName->Length;
 ParPortInfo->SymLinkName.Buffer= ExAllocatePoolWithTag (PagedPool,ParPortInfo->SymLinkName.MaximumLength,PPJOYBUS_POOL_TAG);    
 if (!ParPortInfo->SymLinkName.Buffer)
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_ERROR, ("Error allocating buffer to hold port symbolic link name") );
  goto ExitAndDealloc;
 }

 /* From here everything is a-okay. Save port info in global structure. */
 ParPortInfo->PortAddr= (ULONG)(ULONG_PTR) PortInfo.Controller;
 ParPortInfo->DevObj= DeviceObject;
 ParPortInfo->FileObj= FileObject;
 ParPortInfo->UseCount= 0;
 ParPortInfo->LastChipMode= (UCHAR) PnpInfo.CurrentMode;
 RtlCopyUnicodeString (&ParPortInfo->SymLinkName,SymLinkName);

 goto Exit;

ExitAndDealloc:
 LPTNum= 0;
 ObDereferenceObject (FileObject);
 
Exit:
 /* Return number of this LPT port. */
 return LPTNum;
}

/**************************************************************************/
/* Routine to process PnP interface removal messages. We clear the port's */
/* parameters in the global structure. Return code is the LPT number of   */
/* port that just departed, 0 if we don't know.                           */
/**************************************************************************/
ULONG PPJoyBus_PnPRemoval (IN PDEVICE_OBJECT BusDevObject, IN PUNICODE_STRING SymLinkName)
{
 ULONG						LPTNum;
 PPARPORTINFO				ParPortInfo;

 /* Search through global records until we match the Symbolic Link name */
 for (LPTNum=0;LPTNum<MAX_PARPORTS;LPTNum++)
 {
  if (RtlEqualUnicodeString(&Globals.ParPorts[LPTNum].SymLinkName,SymLinkName,TRUE))
  {
   ParPortInfo= Globals.ParPorts+LPTNum;

   /* Hey, we still got work to do here!!! Clean up all the stuff done  */
   /* in the arrival routine.                                           */
   if (ParPortInfo->FileObj)
    ObDereferenceObject (ParPortInfo->FileObj);
   if (ParPortInfo->SymLinkName.Buffer)
    ExFreePool(ParPortInfo->SymLinkName.Buffer);   

   ParPortInfo->PortAddr= 0;
   ParPortInfo->DevObj= 0;
   ParPortInfo->FileObj= 0;
   ParPortInfo->UseCount= 0;
   ParPortInfo->LastChipMode= 0;
   ParPortInfo->SymLinkName.Buffer= NULL;

   /* OK, now we can return the LPT number that was affected. */
   return LPTNum+1;
  }
 }
 return 0;
}

/**************************************************************************/
/* Callback routine called when a parallel interface arrives or departs.  */
/* On arrival, expose all joystick PDO dependent on this interface. On    */
/* removal, remove the joystick PDOs.                                     */
/**************************************************************************/
NTSTATUS PPJoyBus_PnPNotification (IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION Notification, IN PDEVICE_OBJECT BusDevObject)
{
 NTSTATUS			ntStatus;

 int				Arrival;
 ULONG				LPTNum;

 PLIST_ENTRY		pEntry;
 PBUS_DEVICE_DATA	BusDeviceData;
 PJOY_DEVICE_DATA	JoyDeviceData;

 PAGED_CODE ();

 if ( !IsEqualGUID ((LPGUID)&(Notification->InterfaceClassGuid), (LPGUID)&GUID_PARALLEL_DEVICE) )
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_WARN, ("PnP notification for wrong interface class!!! Ignoring.") );
  return STATUS_SUCCESS;
 }

 Arrival= IsEqualGUID ((LPGUID)&(Notification->Event),(LPGUID)&GUID_DEVICE_INTERFACE_ARRIVAL);

 PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_BABBLE, ("PnP %s notification for symlink %S",Arrival?"arrival":"removal",Notification->SymbolicLinkName->Buffer) );

 /* Process the message... */
 if (Arrival)
  LPTNum= PPJoyBus_PnPArrival (BusDevObject,Notification->SymbolicLinkName);
 else
  LPTNum= PPJoyBus_PnPRemoval (BusDevObject,Notification->SymbolicLinkName);

 /* Exit if the notifcation was not for a valid port */
 if (!LPTNum)
  return STATUS_SUCCESS;

 /* Find all joysticks for the port and set H/W present flag flag for */
 /* arrival or clear it for removal.                                  */

 /* Prepare to access the list of joystick PDOs */
 BusDeviceData= (PBUS_DEVICE_DATA) BusDevObject->DeviceExtension;
 KeEnterCriticalRegion();
 ExAcquireFastMutex (&Globals.Mutex);

 /* Now find the Joystick DeviceObjects in the PDO list. */
 pEntry= BusDeviceData->JoystickList.Flink;
 while (pEntry!=&BusDeviceData->JoystickList)
 {
  JoyDeviceData= CONTAINING_RECORD (pEntry,JOY_DEVICE_DATA,ListEntry);
  pEntry= pEntry->Flink;

  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("Found joystick unit 0x%X on port 0x%X",JoyDeviceData->Config.UnitNumber+1,JoyDeviceData->Config.LPTNumber) );

  if (JoyDeviceData->Config.LPTNumber==LPTNum)
  {
   if (Arrival)
   {
    PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("Setting Hardware Found flag for this joystick") );
    JoyDeviceData->Flags|= PPJFLAGS_HWFOUND;
	JoyDeviceData->Config.PortAddress= Globals.ParPorts[LPTNum-1].PortAddr;
   }
   else
   {
    PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("Clearing Hardware Found flag for this joystick") );
    JoyDeviceData->Flags&= ~PPJFLAGS_HWFOUND;
	JoyDeviceData->Config.PortAddress= 0;
   }
  }
 }

 /* Let go of our exclusive access to the joystick PDO list */
 ExReleaseFastMutex (&Globals.Mutex);
 KeLeaveCriticalRegion();
 
 /* Tell I/O manager to rescan the bus - detect the deleted devices. */
 IoInvalidateDeviceRelations (BusDeviceData->UnderlyingPDO, BusRelations);
 return STATUS_SUCCESS;
}

/**************************************************************************/
/* Routine to increase the allocation count for a specific parallel port. */
/* If this is the first allocation, allocate the port from  parport.sys   */
/**************************************************************************/
NTSTATUS PPJoyBus_ParPortAlloc (ULONG PortIndex)
{
 NTSTATUS			ntStatus;
 PDEVICE_OBJECT		DeviceObject;

 PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_FENTRY, ("PPJoyBus_ParPortAlloc for port index %d",PortIndex) );

 /* If index larger than list of parallel ports, assume raw access. */
 if ((PortIndex--)>MAX_PARPORTS)
 {
  ntStatus= STATUS_SUCCESS;
  goto Exit;
 }

 /* Make sure the parallel port interface for this port index has been */
 /* exposed. If not, then report the device as not ready.              */
 DeviceObject= Globals.ParPorts[PortIndex].DevObj;
 if (!DeviceObject)
 {
  ntStatus= STATUS_DEVICE_NOT_READY;
  goto Exit;
 }

 /* Increase usage count for this port index. If count is >1 then the  */
 /* port must already have been allocated. So don't reallocate.        */
 Globals.ParPorts[PortIndex].UseCount++;
 if (Globals.ParPorts[PortIndex].UseCount>1)
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_BABBLE, ("Another joystick has already opened this port, no need to allocate") );
  ntStatus= STATUS_SUCCESS;
  goto Exit;
 }

 if (PPJoy_Opt_PortFlags&PORTFLAG_NOALLOC)
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_BABBLE, ("No allocate ports flag has been set - not attempting to allocate port") );
  ntStatus= STATUS_SUCCESS;
  goto Exit;
 }

 /* Send IRP to parport.sys driver requesting to allocate the port. */
 ntStatus= PPJoyBus_SendParPortIoctl (DeviceObject,IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE,NULL,0,NULL,0,PORT_ALLOC_TIMEOUT);
 /* If we fail, then reset the allocation count and exit */
 if (!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_WARN, ("IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE failed,ntStatus=0x%X",ntStatus) );
  Globals.ParPorts[PortIndex].UseCount--;
  ntStatus= STATUS_DEVICE_NOT_READY;
  goto Exit;
 }
 
Exit:
 PPJOY_EXITPROC (FILE_PARPORTS|PPJOY_FEXIT_STATUSOK, "PPJoyBus_ParPortAlloc",ntStatus );
 return ntStatus;
}

/**************************************************************************/
/* Routine to decrease the allocation count for a specific parallel port. */
/* If this is the last allocation, then free the port from  parport.sys   */
/* and also reset the chip mode to the way we found it. (just polite)     */
/**************************************************************************/
NTSTATUS PPJoyBus_ParPortFree (ULONG PortIndex)
{
 NTSTATUS			ntStatus;
 PDEVICE_OBJECT		DeviceObject;
 PARALLEL_CHIP_MODE	ChipMode;

 PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_FENTRY, ("PPJoyBus_ParPortFree for port index %d",PortIndex) );

 /* If index larger than list of parallel ports, assume raw access. */
 if ((PortIndex--)>MAX_PARPORTS)
 {
  ntStatus= STATUS_SUCCESS;
  goto Exit;
 }

 /* Make sure the parallel port interface for this port index has been */
 /* exposed. If not, then report the device as not ready.              */
 DeviceObject= Globals.ParPorts[PortIndex].DevObj;
 if (!DeviceObject)
 {
  ntStatus= STATUS_DEVICE_NOT_READY;
  goto Exit;
 }

 /* Decrease usage count for this port index. If count reaches 0 then  */
 /* we can free the port. Else, we exit right here...                  */
 Globals.ParPorts[PortIndex].UseCount--;
 if (Globals.ParPorts[PortIndex].UseCount)
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_BABBLE, ("Another joystick is still using this port, not freeing") );
  ntStatus= STATUS_SUCCESS;
  goto Exit;
 }

 if (PPJoy_Opt_PortFlags&PORTFLAG_NOALLOC)
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_BABBLE, ("No allocate ports flag has been set - not attempting to free port") );
  ntStatus= STATUS_SUCCESS;
  goto Exit;
 }
 
 /* Clear the chipmode we have set. If it fails we do not worry too much */
 /* This can happen if we only set standard parallel port mode.          */
 ChipMode.ModeFlags= Globals.ParPorts[PortIndex].LastChipMode;
 ntStatus= PPJoyBus_SendParPortIoctl (DeviceObject,IOCTL_INTERNAL_PARALLEL_CLEAR_CHIP_MODE,&ChipMode,sizeof(ChipMode),NULL,0,-1);
 if (!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_WARN, ("IOCTL_INTERNAL_PARALLEL_CLEAR_CHIP_MODE failed,ntStatus=0x%X",ntStatus) );
 }
 
 /* Now we free the port allocation */
 ntStatus= PPJoyBus_SendParPortIoctl (DeviceObject,IOCTL_INTERNAL_PARALLEL_PORT_FREE,NULL,0,NULL,0,PORT_ALLOC_TIMEOUT);
 if (!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_WARN, ("IOCTL_INTERNAL_PARALLEL_PORT_FREE failed,ntStatus=0x%X",ntStatus) );
  goto Exit;
 }

Exit:
 PPJOY_EXITPROC (FILE_PARPORTS|PPJOY_FEXIT_STATUSOK, "PPJoyBus_ParPortFree",ntStatus );
 return ntStatus;
}

/**************************************************************************/
/* Routine to set the parallel port mode. We can only do this once we     */
/* have allocated the port.                                               */
/**************************************************************************/
NTSTATUS PPJoyBus_ParSetChipMode (ULONG PortIndex, UCHAR NewChipMode)
{
 NTSTATUS			ntStatus;
 PDEVICE_OBJECT		DeviceObject;
 PARALLEL_CHIP_MODE	ChipMode;

 PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_FENTRY, ("PPJoyBus_ParSetChipMode for port index %d",PortIndex) );

 /* If index larger than list of ports, assume raw access and fail request */
 if ((PortIndex--)>MAX_PARPORTS)
 {
  ntStatus= STATUS_INVALID_PARAMETER;
  goto Exit;
 }

 /* Make sure the parallel port interface for this port index has been */
 /* exposed. If not, then report the device as not ready.              */
 DeviceObject= Globals.ParPorts[PortIndex].DevObj;
 if (!DeviceObject)
 {
  ntStatus= STATUS_DEVICE_NOT_READY;
  goto Exit;
 }

 if (PPJoy_Opt_PortFlags&PORTFLAG_NOALLOC)
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_BABBLE, ("No allocate ports flag has been set - must manually set port mode...") );
  ntStatus= PPJoyBus_ManualSetChipMode (Globals.ParPorts[PortIndex].PortAddr,NewChipMode);
  goto Exit;
 }

 /* Clear the chipmode we have set. If it fails we do not worry too much */
 /* This can happen if we only set standard parallel port mode.          */
 ChipMode.ModeFlags= Globals.ParPorts[PortIndex].LastChipMode;
 ntStatus= PPJoyBus_SendParPortIoctl (DeviceObject,IOCTL_INTERNAL_PARALLEL_CLEAR_CHIP_MODE,&ChipMode,sizeof(ChipMode),NULL,0,-1);
 if (!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_WARN, ("IOCTL_INTERNAL_PARALLEL_CLEAR_CHIP_MODE failed,ntStatus=0x%X",ntStatus) );
 }
 
 /* Set the new chip mode. Fail if we cannot set the correct mode. */
 ChipMode.ModeFlags= NewChipMode;
 ntStatus= PPJoyBus_SendParPortIoctl (DeviceObject,IOCTL_INTERNAL_PARALLEL_SET_CHIP_MODE,&ChipMode,sizeof(ChipMode),NULL,0,-1);
 if (!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_WARN, ("IOCTL_INTERNAL_PARALLEL_SET_CHIP_MODE failed,ntStatus=0x%X",ntStatus) );
  goto Exit;
 }
 
 PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_BABBLE2, ("Parallel port successfully switched to mode 0x%X",NewChipMode) );

 /* Save the last mode we have successfully set. */
 Globals.ParPorts[PortIndex].LastChipMode= ChipMode.ModeFlags;

Exit:
 PPJOY_EXITPROC(FILE_PARPORTS | PPJOY_FEXIT_STATUSOK, "PPJoyBus_ParSetChipMode", ntStatus);
 return ntStatus;
}

#define	MAX_MACRO(X,Y)	((X)>(Y)?(X):(Y))

/* IS THE IRQ SETUP ROUTINES NOT PAGABLE CODE???? */


/**************************************************************************/
/* Routine to allocate an Interrupt Service Routine callback for the      */
/* parallel port. Remember to free it when we are done!!!                 */
/**************************************************************************/
NTSTATUS PPJoyBus_ParAllocIRQ (ULONG PortIndex, PJOYSTICK_IRQ1 IRQ)
{
 NTSTATUS		ntStatus;
 PDEVICE_OBJECT	DeviceObject;
 
 PARALLEL_INTERRUPT_SERVICE_ROUTINE	pisr;
 PARALLEL_INTERRUPT_INFORMATION		pii;

 PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_FENTRY, ("PPJoyBus_ParEnableIRQ for port index %d IRQ.Callback= 0x%p, IRQ.Context= 0x%p",PortIndex,IRQ->Callback,IRQ->Context) );

 /* If index larger than list of ports, assume raw access and fail request */
 if ((PortIndex==0)||(PortIndex>MAX_PARPORTS))
 {
  ntStatus= STATUS_INVALID_PARAMETER;
  goto Exit;
 }

 if (PPJoy_Opt_PortFlags&PORTFLAG_NOALLOC)
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_WARN, ("IRQ operation not support when PORTFLAG_NOALLOC set") );
  ntStatus= STATUS_UNSUCCESSFUL;
  goto Exit;
 }

 PortIndex--;

 /* Make sure the parallel port interface for this port index has been */
 /* exposed. If not, then report the device as not ready.              */
 DeviceObject= Globals.ParPorts[PortIndex].DevObj;
 if (!DeviceObject)
 {
  ntStatus= STATUS_DEVICE_NOT_READY;
  goto Exit;
 }

 /* OK, Now we will try to allocate a new ISR routine... */

 /* Setup parameter for IOCTL call */
 memset (&pisr,0,sizeof(pisr));
 pisr.InterruptServiceRoutine= IRQ->Callback;
 pisr.InterruptServiceContext= IRQ->Context;

 /* Make IOCTL call to set interrupt routine */
 ntStatus= PPJoyBus_SendParPortIoctl (DeviceObject,IOCTL_INTERNAL_PARALLEL_CONNECT_INTERRUPT,&pisr,sizeof(pisr),&pii,sizeof(pii),-1);
 if (!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_WARN, ("IOCTL_INTERNAL_PARALLEL_CONNECT_INTERRUPT failed,ntStatus=0x%X",ntStatus) );
  goto Exit;
 }
 
Exit:
 PPJOY_EXITPROC(FILE_PARPORTS | PPJOY_FEXIT_STATUSOK, "PPJoyBus_ParEnableIRQ", ntStatus);
 return ntStatus;
}

NTSTATUS PPJoyBus_ParFreeIRQ (ULONG PortIndex, PJOYSTICK_IRQ1 IRQ)
{
 NTSTATUS		ntStatus;
 PDEVICE_OBJECT	DeviceObject;
 
 PARALLEL_INTERRUPT_SERVICE_ROUTINE	pisr;

 PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_FENTRY, ("PPJoyBus_ParFreeIRQ for port index %d IRQ.Callback= 0x%p, IRQ.Context= 0x%p",PortIndex,IRQ->Callback,IRQ->Context) );

 /* If index larger than list of ports, assume raw access and fail request */
 if ((PortIndex==0)||(PortIndex>MAX_PARPORTS))
 {
  ntStatus= STATUS_INVALID_PARAMETER;
  goto Exit;
 }

 if (PPJoy_Opt_PortFlags&PORTFLAG_NOALLOC)
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_WARN, ("IRQ operation not support when PORTFLAG_NOALLOC set") );
  ntStatus= STATUS_UNSUCCESSFUL;
  goto Exit;
 }

 PortIndex--;

 /* Make sure the parallel port interface for this port index has been */
 /* exposed. If not, then report the device as not ready.              */
 DeviceObject= Globals.ParPorts[PortIndex].DevObj;
 if (!DeviceObject)
 {
  ntStatus= STATUS_DEVICE_NOT_READY;
  goto Exit;
 }

 /* OK, Now we will try to free the existing ISR routine... */

 /* Setup parameter for IOCTL call */
 memset (&pisr,0,sizeof(pisr));
 pisr.InterruptServiceRoutine= IRQ->Callback;
 pisr.InterruptServiceContext= IRQ->Context;

 /* Make IOCTL call to free interrupt routine */
 ntStatus= PPJoyBus_SendParPortIoctl (DeviceObject,IOCTL_INTERNAL_PARALLEL_DISCONNECT_INTERRUPT,&pisr,sizeof(pisr),NULL,0,-1);
 if (!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_PARPORTS|PPJOY_WARN, ("IOCTL_INTERNAL_PARALLEL_DISCONNECT_INTERRUPT failed,ntStatus=0x%X",ntStatus) );
  goto Exit;
 }
 
Exit:
 PPJOY_EXITPROC(FILE_PARPORTS | PPJOY_FEXIT_STATUSOK, "PPJoyBus_ParFreeIRQ", ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine to simulate parport.sys SetChipMode routine. We will check if  */
/* there is ECP support and if so set the mode. Otherwise we will test to */
/* see if BiDirectional mode is required and works.                       */
/**************************************************************************/
NTSTATUS PPJoyBus_ManualSetChipMode (ULONG BaseAddress, IN UCHAR ChipModeFlags)
{
#define	CONTROL_FORWARD	0x04
#define	CONTROL_REVERSE	0x24

 PUCHAR				ECR;
 PUCHAR				DCR;
 PUCHAR				DR;
 UCHAR				Data;

 PAGED_CODE ();

 /* First check that we know the mode requested. Anything else is an error */
 if (!( (ChipModeFlags==ECR_SPP_MODE) || (ChipModeFlags==ECR_BYTE_PIO_MODE) ))
  return STATUS_INVALID_DEVICE_STATE;

 if (BaseAddress!=0x3BC)
 {
  ECR= (PUCHAR)(ULONG_PTR) (BaseAddress+ECR_OFFSET);

  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("W98Ports_ParallelSetChipMode: Set ECR register 0x%x to 0x%x)",ECR,ChipModeFlags) );
  WRITE_PORT_UCHAR (ECR,ChipModeFlags);
 }

 /* Set reverse mode if ChipMode is bidirectional, else set forward mode */
 DCR= (PUCHAR)(ULONG_PTR) (BaseAddress+DCR_OFFSET);
 if (ChipModeFlags)
 {
  WRITE_PORT_UCHAR (DCR,CONTROL_REVERSE);

  DR= (PUCHAR)(ULONG_PTR) BaseAddress;

  /* Now test that reverse mode works. */
  WRITE_PORT_UCHAR (DR,0xA5);
  Data= READ_PORT_UCHAR (DR);

  if (Data==0xA5)
  {
   /* Hmm, first time we got same data, try again with new value */
   WRITE_PORT_UCHAR (DR,0x5A);
   Data= READ_PORT_UCHAR (DR);

   if (Data==0x5A)
   {
	/* Same data again. This is too much... assume port is not bidirectional */
    return STATUS_INVALID_DEVICE_STATE;
   }
  }
 }
 else
 {
  WRITE_PORT_UCHAR (DCR,CONTROL_FORWARD);
 }

 return STATUS_SUCCESS;
}
