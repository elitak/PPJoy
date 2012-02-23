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

#include "W98Ports.h"



/**************************************************************************/
/* Declare the code segments into which each routine will be placed       */
/* Used to declare pagable and initialisation code.                       */
/**************************************************************************/

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE,W98Ports_SendIrpSynchronously)
    #pragma alloc_text (PAGE,W98Ports_EnterRequest)
    #pragma alloc_text (PAGE,W98Ports_LeaveRequest)
#endif



/**************************************************************************/
/* Routine to synchronously send an IRP to the driver indicated by the    */
/* DeviceObject parameter. Once the routine returns the lower level       */
/* drivers will have processed the IRP.                                   */
/**************************************************************************/
NTSTATUS W98Ports_SendIrpSynchronously (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 KEVENT		event;
 NTSTATUS	ntStatus;

 PAGED_CODE();

 KeInitializeEvent (&event,NotificationEvent,FALSE);

 IoCopyCurrentIrpStackLocationToNext (Irp);

 /* Set completion routine, to be called when lower driver is done with */
 /* this IRP. From this routine we signal the event when done.          */
 IoSetCompletionRoutine (Irp,W98Ports_CompletionRoutine,&event,TRUE,TRUE,TRUE);

 /* Call the lower level driver. */ 
 ntStatus= IoCallDriver(DeviceObject,Irp);

 /* Wait for the lower drivers to finish with the IRP. */
 if (ntStatus == STATUS_PENDING)
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
NTSTATUS W98Ports_CompletionRoutine (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
 UNREFERENCED_PARAMETER (DeviceObject);

 /* Completion routines cannot be in paged code */
 
 KeSetEvent ((PKEVENT)Context,IO_NO_INCREMENT,FALSE);

 /* Set return code to indicate that we are not done with the IRP */
 return STATUS_MORE_PROCESSING_REQUIRED;
}

/**************************************************************************/
/* Routine to increment the count of current requests being serviced.     */
/**************************************************************************/
NTSTATUS W98Ports_EnterRequest (IN PDEVICE_OBJECT DeviceObject)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE ();

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

 /* Increase request count */
 InterlockedIncrement (&DeviceExtension->ReqCount);
    
 if (DeviceExtension->Flags&W98PFLAGS_REMOVED)
 {
  PPJOY_DBGPRINT (FILE_IRPUTIL|PPJOY_WARN, ("Recieved a new request after a PnP Remove IRP.") );
  ntStatus= STATUS_DELETE_PENDING;
 }
 else
  ntStatus= STATUS_SUCCESS;

 PPJOY_DBGPRINT (FILE_IRPUTIL|PPJOY_WARN, ("EnterRequest exit, request count now %d",DeviceExtension->ReqCount) );

 return ntStatus;
}

/**************************************************************************/
/* Routine to decrement the count of current requests being serviced.     */
/* If we are exiting the last request when the remove flag has been set   */
/* then signal the event as well.                                         */
/**************************************************************************/
NTSTATUS W98Ports_LeaveRequest (IN PDEVICE_OBJECT DeviceObject)
{
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE ();

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

 /* Decrease request count. If zero we will check signal RemoveEvent if */
 /* the remove flag is set.                                             */
 if (!InterlockedDecrement(&DeviceExtension->ReqCount))
 {
  if (DeviceExtension->Flags&W98PFLAGS_REMOVED)
   KeSetEvent (&DeviceExtension->RemoveEvent,IO_NO_INCREMENT,FALSE);
 }

 PPJOY_DBGPRINT (FILE_IRPUTIL|PPJOY_WARN, ("LeaveRequest exit, request count now %d",DeviceExtension->ReqCount) );

 return STATUS_SUCCESS;
}
