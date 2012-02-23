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
/**  JoyIoctl.c  Routines to handle IOCTL and Internal IOCTL requests     **/
/**              for the Joystick PDOs                                    **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "PPJoyBus.h"



/**************************************************************************/
/* Declare the code segments into which each routine will be placed       */
/* Used to declare pagable and initialisation code.                       */
/**************************************************************************/

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE,PPJoyBus_InternalIoctl_Joy)
    #pragma alloc_text (PAGE,PPJoyBus_Ioctl_Joy)
#endif

/**************************************************************************/
/* Routine that handles Internal IOCTL requests for the Joystick PDOs     */
/* We will always complete an IRP because we are the lowest level driver. */
/**************************************************************************/
NTSTATUS PPJoyBus_InternalIoctl_Joy (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PIO_STACK_LOCATION		IrpStack;
 PJOY_DEVICE_DATA		JoyDeviceData;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJoyBus_InternalIoctl_Joy (DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 /* Default return code - assume IOCTL is not supported */
 ntStatus = STATUS_NOT_SUPPORTED;

 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 JoyDeviceData= (PJOY_DEVICE_DATA) DeviceObject->DeviceExtension;

 switch (IrpStack->Parameters.DeviceIoControl.IoControlCode)
 {
  case IOCTL_PPJOYBUS_CHIPMODE:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("IOCTL_PPJOYBUS_CHIPMODE") );

		/* Assume there is an error with the buffer length */
		if (IrpStack->Parameters.DeviceIoControl.InputBufferLength!=sizeof(UCHAR))
		{
		 ntStatus= STATUS_INVALID_PARAMETER;
		 break;
		}
		/* Buffer is OK, set chip mode and return result to the caller */
		ntStatus= PPJoyBus_ParSetChipMode (JoyDeviceData->Config.LPTNumber,*((UCHAR*)Irp->AssociatedIrp.SystemBuffer));
		break;

  case IOCTL_PPJOYBUS_GETCONFIG1:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("IOCTL_PPJOYBUS_GETCONFIG1") );

		/* Assume there is an error with the buffer length */
		ntStatus= STATUS_INVALID_PARAMETER;
		if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength>=JoyDeviceData->Config.Size)
		{
		 /* Buffer size is OK so we proceed to return to requested data */
		 RtlCopyMemory (Irp->AssociatedIrp.SystemBuffer,&JoyDeviceData->Config,JoyDeviceData->Config.Size);

		 ntStatus= STATUS_SUCCESS;
		 Irp->IoStatus.Information= sizeof(JOYSTICK_CONFIG1);
		}
		break;

  case IOCTL_PPJOYBUS_ALLOCIRQ1:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("IOCTL_PPJOYBUS_ALLOCIRQ1") );

		/* Assume there is an error with the buffer length */
		if (IrpStack->Parameters.DeviceIoControl.InputBufferLength!=sizeof(JOYSTICK_IRQ1))
		{
		 ntStatus= STATUS_INVALID_PARAMETER;
		 break;
		}
		/* Try to enable Interrupt Service Routine for LPT - If successful we store the parameters -  */
		/* we need to use them to free the ISR again. No stored params = no ISR.                      */
		ntStatus= PPJoyBus_ParAllocIRQ (JoyDeviceData->Config.LPTNumber,(PJOYSTICK_IRQ1)Irp->AssociatedIrp.SystemBuffer);
		if (NT_SUCCESS(ntStatus))
		 RtlCopyMemory (&JoyDeviceData->IRQ,Irp->AssociatedIrp.SystemBuffer,sizeof(JoyDeviceData->IRQ));
		break;

  case IOCTL_PPJOYBUS_REGOPTIONS:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("IOCTL_PPJOYBUS_REGOPTIONS") );

		/* Assume there is an error with the buffer length */
		if (IrpStack->Parameters.DeviceIoControl.InputBufferLength!=sizeof(PPJOY_OPTION_CALLBACK))
		{
		 ntStatus= STATUS_INVALID_PARAMETER;
		 break;
		}
		/* Try to enable Interrupt Service Routine for LPT - If successful we store the parameters -  */
		/* we need to use them to free the ISR again. No stored params = no ISR.                      */

		PPJoyOptionCallback= *((PPJOY_OPTION_CALLBACK*) Irp->AssociatedIrp.SystemBuffer);

		if (PPJoyOptionCallback)
		{
		 PPJoyOptionCallback (PPJOY_OPTION_RUNTIMEDEBUG,PPJoy_Opt_RuntimeDebug);
		 PPJoyOptionCallback (PPJOY_OPTION_PORTFLAGS,PPJoy_Opt_PortFlags);
		}
		ntStatus= STATUS_SUCCESS;
		break;


  default:
		break;
 }

 /* Set the real return status in Irp and complete the IRP */
 Irp->IoStatus.Status = ntStatus;
 IoCompleteRequest (Irp,IO_NO_INCREMENT);

 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoyBus_InternalIoctl_Joy",ntStatus );
 return ntStatus;
}

/**************************************************************************/
/* Routine that handles (normal) IOCTL requests for the Joystick PDOs     */
/* Since we got no IOCTLs defines we will simply return an error.         */
/* We will always complete an IRP because we are the lowest level driver. */
/**************************************************************************/
NTSTATUS PPJoyBus_Ioctl_Joy (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJoyBus_Ioctl_Joy (DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 ntStatus= STATUS_NOT_SUPPORTED;
 
 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_WARN, ("Unknown or unsupported IOCTL (%x)",IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl.IoControlCode) );

 /* Set the real return status in Irp and complete the IRP */
 Irp->IoStatus.Status = ntStatus;
 IoCompleteRequest (Irp,IO_NO_INCREMENT);

 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoyBus_Ioctl_Joy",ntStatus );
 return ntStatus;
}
