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
/**  Ioctl.c     Routines to handle Internal IOCTL requests from the      **/
/**              Joystick bus enumerator FDO and return results similar   **/
/**              to parport.sys on Windows 2000/XP                        **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "W98Ports.h"
#include <wchar.h>


/**************************************************************************/
/* Declare the code segments into which each routine will be placed       */
/* Used to declare pagable and initialisation code.                       */
/**************************************************************************/

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE,W98Ports_DoInternalIoctl)
    #pragma alloc_text (PAGE,W98Ports_GetParallelPortInfo)
    #pragma alloc_text (PAGE,W98Ports_GetParallelPnpInfo)
    #pragma alloc_text (PAGE,W98Ports_ParallelPortAllocate)
    #pragma alloc_text (PAGE,W98Ports_ParallelPortFree)
    #pragma alloc_text (PAGE,W98Ports_ParallelSetChipMode)
    #pragma alloc_text (PAGE,W98Ports_ParallelClearChipMode)
#endif

/**************************************************************************/
/* Routine that handles Internal IOCTL requests from parallel port bus    */
/**************************************************************************/
NTSTATUS W98Ports_DoInternalIoctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PIO_STACK_LOCATION	IrpStack;
 ULONG				InLen;
 ULONG				OutLen;
 PVOID				Buffer;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("W98Ports_DoInternalIoctl (DeviceObject=0x%x,Irp=0x%x)",DeviceObject,Irp) );

 /* Setup pointers to all the fields in the IRP */
 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 InLen= IrpStack->Parameters.DeviceIoControl.InputBufferLength;
 OutLen= IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
 Buffer= Irp->AssociatedIrp.SystemBuffer;

 /* Default return code */
 ntStatus= STATUS_INVALID_PARAMETER;
 Irp->IoStatus.Information= 0;

 /* Decide to do with the IRP */
 switch (IrpStack->Parameters.DeviceIoControl.IoControlCode)
 {
  case IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO") );

		ntStatus= STATUS_BUFFER_TOO_SMALL;
		if (OutLen>=sizeof(PARALLEL_PORT_INFORMATION))
		{
		 ntStatus= W98Ports_GetParallelPortInfo (DeviceObject,(PPARALLEL_PORT_INFORMATION) Buffer);
		 if (NT_SUCCESS(ntStatus))
		  Irp->IoStatus.Information= sizeof(PARALLEL_PORT_INFORMATION);
		}
		break;

  case IOCTL_INTERNAL_GET_PARALLEL_PNP_INFO:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("IOCTL_INTERNAL_GET_PARALLEL_PNP_INFO") );

		ntStatus= STATUS_BUFFER_TOO_SMALL;
		if (OutLen>=sizeof(PARALLEL_PNP_INFORMATION))
		{
		 ntStatus= W98Ports_GetParallelPnpInfo (DeviceObject,(PPARALLEL_PNP_INFORMATION) Buffer);
		 if (NT_SUCCESS(ntStatus))
		  /* Windows 2000 DDK (2195) says value should be zero. Think the DDK is wrong */
		  Irp->IoStatus.Information= sizeof(PARALLEL_PNP_INFORMATION);
		}
		break;

  case IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE") );

		ntStatus= W98Ports_ParallelPortAllocate (DeviceObject);
		break;

  case IOCTL_INTERNAL_PARALLEL_PORT_FREE:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("IOCTL_INTERNAL_PARALLEL_PORT_FREE") );

		ntStatus= W98Ports_ParallelPortFree (DeviceObject);
		break;

  case IOCTL_INTERNAL_PARALLEL_SET_CHIP_MODE:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("IOCTL_INTERNAL_PARALLEL_SET_CHIP_MODE") );

		ntStatus= STATUS_BUFFER_TOO_SMALL;
		if (InLen>=sizeof(PARALLEL_CHIP_MODE))
		 ntStatus= W98Ports_ParallelSetChipMode (DeviceObject,(PPARALLEL_CHIP_MODE) Buffer);
		break;

  case IOCTL_INTERNAL_PARALLEL_CLEAR_CHIP_MODE:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("IOCTL_INTERNAL_PARALLEL_CLEAR_CHIP_MODE") );

		ntStatus= STATUS_BUFFER_TOO_SMALL;
		if (InLen>=sizeof(PARALLEL_CHIP_MODE))
		 ntStatus= W98Ports_ParallelClearChipMode (DeviceObject,(PPARALLEL_CHIP_MODE) Buffer);
		break;
		
  default:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_WARN, ("Unknown or unsupported IOCTL (%x)",IrpStack->Parameters.DeviceIoControl.IoControlCode) );

		/* Do don't know this IOCTL so we pass it along to lower driver */
		IoSkipCurrentIrpStackLocation (Irp);
		ntStatus= IoCallDriver (((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->NextLowerDriver,Irp);
		goto ExitNoComplete;
 }

 /* Set the real return status in Irp and complete for IRPs that we handle */
 Irp->IoStatus.Status= ntStatus;
 IoCompleteRequest (Irp,IO_NO_INCREMENT);

ExitNoComplete:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "W98Ports_DoInternalIoctl",ntStatus );
 return ntStatus;
}


/**************************************************************************/
/* Routine to simulate parport.sys GetParallelPortInfo routine.           */
/* For the most part we put NULL values in the fields - hopefully clients */
/* will have the sense to test these before calling!
/**************************************************************************/
NTSTATUS W98Ports_GetParallelPortInfo (IN PDEVICE_OBJECT DeviceObject, IN PPARALLEL_PORT_INFORMATION PortInfo)
{
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE ();

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

 PortInfo->OriginalController.QuadPart= 0;
 PortInfo->Controller= (PUCHAR) DeviceExtension->BaseAddress;
 PortInfo->SpanOfController= 3;
 PortInfo->TryAllocatePort= NULL;
 PortInfo->FreePort= NULL;
 PortInfo->QueryNumWaiters= NULL;
 PortInfo->Context= NULL;

 return STATUS_SUCCESS;
}

/**************************************************************************/
/* Routine to simulate parport.sys GetParallelPnpInfo routine.           */
/* For the most part we put NULL values in the fields - hopefully clients */
/* will have the sense to test these before calling!
/**************************************************************************/
NTSTATUS W98Ports_GetParallelPnpInfo (IN PDEVICE_OBJECT DeviceObject, IN PPARALLEL_PNP_INFORMATION PnpInfo)
{
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE ();

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

 PnpInfo->OriginalEcpController.QuadPart= 0;
 PnpInfo->EcpController= 0;
 PnpInfo->SpanOfEcpController= 0;
 PnpInfo->PortNumber= 0;
 PnpInfo->HardwareCapabilities= 0;
 PnpInfo->TrySetChipMode= NULL;
 PnpInfo->ClearChipMode= NULL;
 PnpInfo->FifoDepth= 0;
 PnpInfo->FifoWidth= 0;
 PnpInfo->EppControllerPhysicalAddress.QuadPart= 0;
 PnpInfo->SpanOfEppController= 0;
 PnpInfo->Ieee1284_3DeviceCount= 0;
 PnpInfo->TrySelectDevice= NULL;
 PnpInfo->DeselectDevice= NULL;
 PnpInfo->Context= NULL;
 PnpInfo->CurrentMode= 0;
 PnpInfo->PortName= DeviceExtension->LPTName;

 return STATUS_SUCCESS;
}

/**************************************************************************/
/* Routine to simulate parport.sys Allocate routine.                      */
/* We will (for simplicity) not queue callers, fail immediately if        */
/* already allocated. No integration into VCOMM/VPD at the moment.        */
/* Leave something for another day :-)                                    */
/**************************************************************************/
NTSTATUS W98Ports_ParallelPortAllocate (IN PDEVICE_OBJECT DeviceObject)
{
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE ();

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

 if (DeviceExtension->Flags&W98PFLAGS_ALLOCATED)
  return STATUS_CANCELLED;	/* Or perhaps STATUS_DEVICE_BUSY? */

 DeviceExtension->Flags|= W98PFLAGS_ALLOCATED;

 return STATUS_SUCCESS;
}

/**************************************************************************/
/* Routine to simulate parport.sys Free routine. This will always succeed */
/* No integration into VCOMM/VPD at the moment.                           */
/**************************************************************************/
NTSTATUS W98Ports_ParallelPortFree (IN PDEVICE_OBJECT DeviceObject)
{
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE ();

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
 DeviceExtension->Flags&= ~W98PFLAGS_ALLOCATED;

 return STATUS_SUCCESS;
}

/**************************************************************************/
/* Routine to simulate parport.sys SetChipMode routine. We will check if  */
/* there is ECP support and if so set the mode. Otherwise we will test to */
/* see if BiDirectional mode is required and works.                       */
/**************************************************************************/
NTSTATUS W98Ports_ParallelSetChipMode (IN PDEVICE_OBJECT DeviceObject, IN PPARALLEL_CHIP_MODE ChipMode)
{
#define	CONTROL_FORWARD	0x04
#define	CONTROL_REVERSE	0x24

 PDEVICE_EXTENSION	DeviceExtension;
 PUCHAR				ECR;
 PUCHAR				DCR;
 PUCHAR				DR;
 UCHAR				Data;

 PAGED_CODE ();

 /* First check that we know the mode requested. Anything else is an error */
 if (!( (ChipMode->ModeFlags==ECR_SPP_MODE) || (ChipMode->ModeFlags==ECR_BYTE_PIO_MODE) ))
  return STATUS_INVALID_DEVICE_STATE;

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

 if (DeviceExtension->BaseAddress!=0x3BC)
 {
  ECR= (PUCHAR) DeviceExtension->BaseAddress+ECR_OFFSET;

  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("W98Ports_ParallelSetChipMode: Set ECR register 0x%x to 0x%x)",ECR,ChipMode->ModeFlags) );
  WRITE_PORT_UCHAR (ECR,ChipMode->ModeFlags);
 }

 /* Set reverse mode if ChipMode is bidirectional, else set forward mode */
 DCR= (PUCHAR) DeviceExtension->BaseAddress+DCR_OFFSET;
 if (ChipMode->ModeFlags)
 {
  WRITE_PORT_UCHAR (DCR,CONTROL_REVERSE);

  DR= (PUCHAR) DeviceExtension->BaseAddress;

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

/**************************************************************************/
/* Routine to simulate parport.sys ClearChipMode routine. We will simply  */
/* succeed the request without clearing the chip mode.                    */
/**************************************************************************/
NTSTATUS W98Ports_ParallelClearChipMode (IN PDEVICE_OBJECT DeviceObject, IN PPARALLEL_CHIP_MODE ChipMode)
{
 PAGED_CODE ();

 return STATUS_SUCCESS;
}
