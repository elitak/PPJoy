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
/**  IRPUtil.c  Utility routines for IRP handling                         **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "PPortJoy.h"



/**************************************************************************/
/* Declare the code segments into which each routine will be placed       */
/* Used to declare pagable and initialisation code.                       */
/**************************************************************************/

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE,PPortJoy_SendIrpSynchronously)
    #pragma alloc_text (PAGE,PPortJoy_SendInternalIoctl)
#endif



/**************************************************************************/
/* Routine synchronously send an IRP to the driver indicated by the       */
/* DeviceObject parameter. Once the routine returns the lower level       */
/* drivers will have processed the IRP. If CopyStack is set then this     */
/* routine will copy the current stack location for the next driver,      */
/* else the caller should set up the next stack themselves.               */
/**************************************************************************/
NTSTATUS PPortJoy_SendIrpSynchronously (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN UCHAR CopyStack)
{
 KEVENT		event;
 NTSTATUS	ntStatus;

 PAGED_CODE();

 KeInitializeEvent (&event,NotificationEvent,FALSE);

 if (CopyStack)
  IoCopyCurrentIrpStackLocationToNext (Irp);

 /* Set completion routine, to be called when lower driver is done with */
 /* this IRP. From this routine we signal the event when done.          */
 IoSetCompletionRoutine (Irp,PPortJoy_CompletionRoutine,&event,TRUE,TRUE,TRUE);

 /* Call the lower level driver. */ 
 ntStatus= IoCallDriver(DeviceObject,Irp);

 /* Wait for the lower drivers to finish with the IRP. */
 if (ntStatus==STATUS_PENDING)
 {
  KeWaitForSingleObject (&event,Executive,KernelMode,FALSE,NULL);
  ntStatus= Irp->IoStatus.Status;
 }

 return ntStatus;
}

/**************************************************************************/
/* Completion routine used by SendIrpSynchronously. Sets event passed as  */
/* the Context parameter and then return.                                 */
/**************************************************************************/
NTSTATUS PPortJoy_CompletionRoutine (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
 UNREFERENCED_PARAMETER (DeviceObject);

 /* Completion routines cannot be in paged code */
 
 KeSetEvent ((PKEVENT)Context,IO_NO_INCREMENT,FALSE);

 /* Set return code to indicate that we are not done with the IRP */
 return STATUS_MORE_PROCESSING_REQUIRED;
}

/**************************************************************************/
/* Routine that send Internal IOCTL requests (to PPJoyBus driver).        */
/* This routine will return after TimeoutSecs seconds or (if negative)    */
/* until the request completes.                                           */
/**************************************************************************/
NTSTATUS PPortJoy_SendInternalIoctl (IN PDEVICE_OBJECT DeviceObject, IN ULONG IoControlCode,
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

 /* Construct the IO request for the driver. */
 Irp= IoBuildDeviceIoControlRequest (IoControlCode,DeviceObject,InputBuffer,InputBufferLength,
									 OutputBuffer,OutputBufferLength,TRUE,&Event,&IoStatus);

 if (!Irp)
  return STATUS_INSUFFICIENT_RESOURCES;
    
 /* Call the driver with the request. */
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
