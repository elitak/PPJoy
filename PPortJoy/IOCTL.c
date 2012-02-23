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
/**  IOCTL.c    Routines to handle Internal IOCTL request from the        **/
/**             class driver                                              **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "PPortJoy.h"
#include "stdio.h"

/* These two include files ship with the HIDGAME Win2K DDK samples */
#include <hidtoken.h>
#include <hidusage.h>



/**************************************************************************/
/* Declare the code segments into which each routine will be placed       */
/* Used to declare pagable and initialisation code.                       */
/**************************************************************************/

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE, PPJoy_Ctl_Ioctl)
    #pragma alloc_text (PAGE, PPJoy_GetDeviceDescriptor)
    #pragma alloc_text (PAGE, PPJoy_GetReportDescriptor)
    #pragma alloc_text (PAGE, PPJoy_GetAttributes)
    #pragma alloc_text (PAGE, PPJoy_GenerateReport)
    #pragma alloc_text (PAGE, PPJoy_SetJoystickState)
    #pragma alloc_text (PAGE, PPJoy_GetJoystickState)
    #pragma alloc_text (PAGE, PPJoy_SetJoystickMap)
    #pragma alloc_text (PAGE, PPJoy_GetJoystickMap)
    #pragma alloc_text (PAGE, PPJoy_DelJoystickMap)
    #pragma alloc_text (PAGE, PPJoy_SetJoyTiming)
    #pragma alloc_text (PAGE, PPJoy_GetJoyTiming)
    #pragma alloc_text (PAGE, PPJoy_DelJoyTiming)
    #pragma alloc_text (PAGE, PPJoy_ActJoyTiming)
    #pragma alloc_text (PAGE, PPJoy_GetPPJoyOptions)
    #pragma alloc_text (PAGE, PPJoy_SetPPJoyOptions)
#endif



/**************************************************************************/
/* Routine to handle Internal IOCTL IRPs. Most of the work is delegated   */
/* helper functions.                                                      */
/**************************************************************************/
NTSTATUS PPJoy_InternalIoctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;
 PIO_STACK_LOCATION	IrpStack;

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJoy_InternalIoctl(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 /* InternalIoctl is not pagable. This is because IOCTL_HID_READ_REPORT     */
 /* can be called at dispatch-level.                                        */ 
 
 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 DeviceExtension= GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

 ntStatus= PPJoy_EnterRequest (DeviceExtension);
 if (!NT_SUCCESS(ntStatus))
  goto Exit;

 switch(IrpStack->Parameters.DeviceIoControl.IoControlCode)
 {
  case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_HID_GET_DEVICE_DESCRIPTOR") );
		ntStatus= PPJoy_GetDeviceDescriptor (DeviceObject,Irp);
        break;

  case IOCTL_HID_GET_REPORT_DESCRIPTOR:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_HID_GET_REPORT_DESCRIPTOR") );
		ntStatus= PPJoy_GetReportDescriptor (DeviceObject,Irp);
		break;

  case IOCTL_HID_READ_REPORT:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_HID_READ_REPORT") );
		ntStatus= PPJoy_ReadReport (DeviceObject,Irp);
		break;

  case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_HID_GET_DEVICE_ATTRIBUTES") );
		ntStatus= PPJoy_GetAttributes (DeviceObject,Irp);
		break;

  default:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_WARN,("Unknown or unsupported IOCTL (%x)",
                        IrpStack->Parameters.DeviceIoControl.IoControlCode) );
		ntStatus= STATUS_NOT_SUPPORTED;
		break;
 }

Exit:
 /* Set the return status for operation in the IRP */
 Irp->IoStatus.Status= ntStatus;
 PPJoy_LeaveRequest	(DeviceExtension);

 /* Complete the IRP */
 IoCompleteRequest (Irp,IO_NO_INCREMENT);
 ntStatus= STATUS_SUCCESS;

 PPJOY_EXITPROC(FILE_IOCTL | PPJOY_FEXIT_STATUSOK, "PPJoy_InternalIoctl", ntStatus);

 return ntStatus;
}

/**************************************************************************/
/* Routine to handle Internal IOCTL IRPs. If not control device then we   */
/* pass the IRP on to HID.sys.                                            */
/**************************************************************************/
NTSTATUS PPJoy_Ctl_InternalIoctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;
 PIO_STACK_LOCATION	IrpStack;

 /* InternalIoctl is not pagable. This is because IOCTL_HID_READ_REPORT     */
 /* can be called at dispatch-level.                                        */ 

 /* If it is not for one of our control devices - pass on to HID.sys */
 if (GET_NEXT_DEVICE_OBJECT(DeviceObject))
 {
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_HIDHACK, ("Not a control device - passing request on to HID.sys (PPJoy_Ctl_InternalIoctl)") );
  return HIDMajorFunctions[IoGetCurrentIrpStackLocation(Irp)->MajorFunction](DeviceObject,Irp);
 }

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJoy_Ctl_InternalIoctl(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 DeviceExtension= GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

 ntStatus= PPJoy_EnterRequest (DeviceExtension);
 if (!NT_SUCCESS(ntStatus))
  goto Exit;

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_WARN,("Unknown or unsupported IOCTL (%x)",IrpStack->Parameters.DeviceIoControl.IoControlCode) );
 ntStatus= STATUS_NOT_SUPPORTED;

Exit:
 /* Set the return status for operation in the IRP */
 Irp->IoStatus.Status= ntStatus;
 PPJoy_LeaveRequest	(DeviceExtension);

 /* Complete the IRP */
 IoCompleteRequest (Irp,IO_NO_INCREMENT);
 /* According to DriverVerifier return code should be same as IRP status. */
 /* ntStatus= STATUS_SUCCESS; */

 PPJOY_EXITPROC(FILE_IOCTL | PPJOY_FEXIT_STATUSOK, "PPJoy_Ctl_InternalIoctl", ntStatus);

 return ntStatus;
}

/**************************************************************************/
/* Routine to handle IOCTL IRPs from the outside world.                   */
/**************************************************************************/
NTSTATUS PPJoy_Ctl_Ioctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;
 PIO_STACK_LOCATION	IrpStack;

 PAGED_CODE ();

 /* If it is not for one of our control devices - pass on to HID.sys */
 if (GET_NEXT_DEVICE_OBJECT(DeviceObject))
 {
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_HIDHACK, ("Not a control device - passing request on to HID.sys (PPJoy_Ctl_Ioctl)") );
  return HIDMajorFunctions[IoGetCurrentIrpStackLocation(Irp)->MajorFunction](DeviceObject,Irp);
 }

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJoy_Ctl_Ioctl(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 DeviceExtension= GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

 ntStatus= PPJoy_EnterRequest (DeviceExtension);
 if (!NT_SUCCESS(ntStatus))
  goto Exit;

 switch(IrpStack->Parameters.DeviceIoControl.IoControlCode)
 {
  case IOCTL_PPORTJOY_SET_STATE:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPORTJOY_SET_STATE") );
		ntStatus= PPJoy_SetJoystickState (DeviceObject,Irp);
        break;

  case IOCTL_PPORTJOY_GET_STATE:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPORTJOY_GET_STATE") );
		ntStatus= PPJoy_GetJoystickState (DeviceObject,Irp);
        break;

  case IOCTL_PPORTJOY_SET_MAPPING:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPORTJOY_SET_MAPPING") );
		ntStatus= PPJoy_SetJoystickMap (DeviceObject,Irp);
        break;

  case IOCTL_PPORTJOY_GET_MAPPING:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPORTJOY_GET_MAPPING") );
		ntStatus= PPJoy_GetJoystickMap (DeviceObject,Irp);
        break;

  case IOCTL_PPORTJOY_DEL_MAPPING:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPORTJOY_DEL_MAPPING") );
		ntStatus= PPJoy_DelJoystickMap (DeviceObject,Irp);
        break;

  case IOCTL_PPORTJOY_SET_FORCE:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPORTJOY_SET_FORCE") );
		ntStatus= STATUS_NOT_SUPPORTED;
		break;

  case IOCTL_PPORTJOY_SET_TIMING:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPORTJOY_SET_TIMING") );
		ntStatus= PPJoy_SetJoyTiming (DeviceObject,Irp);
        break;

  case IOCTL_PPORTJOY_GET_TIMING:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPORTJOY_GET_TIMING") );
		ntStatus= PPJoy_GetJoyTiming (DeviceObject,Irp);
        break;

  case IOCTL_PPORTJOY_DEL_TIMING:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPORTJOY_DEL_TIMING") );
		ntStatus= PPJoy_DelJoyTiming (DeviceObject,Irp);
        break;

  case IOCTL_PPORTJOY_ACT_TIMING:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPORTJOY_ACT_TIMING") );
		ntStatus= PPJoy_ActJoyTiming (DeviceObject,Irp);
        break;

  case IOCTL_PPORTJOY_GET_OPTIONS:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPORTJOY_GET_OPTIONS") );
		ntStatus= PPJoy_GetPPJoyOptions (DeviceObject,Irp);
        break;

  case IOCTL_PPORTJOY_SET_OPTIONS:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPORTJOY_SET_OPTIONS") );
		ntStatus= PPJoy_SetPPJoyOptions (DeviceObject,Irp);
        break;

  default:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_WARN,("Unknown or unsupported IOCTL (%x)",
                        IrpStack->Parameters.DeviceIoControl.IoControlCode) );
		ntStatus= STATUS_NOT_SUPPORTED;
		break;
 }

Exit:
 /* Set the return status for operation in the IRP */
 Irp->IoStatus.Status= ntStatus;
 PPJoy_LeaveRequest	(DeviceExtension);

 /* Complete the IRP */
 IoCompleteRequest (Irp,IO_NO_INCREMENT);
 /* According to DriverVerifier return code should be same as IRP status. */
 /* ntStatus= STATUS_SUCCESS; */

 PPJOY_EXITPROC(FILE_IOCTL | PPJOY_FEXIT_STATUSOK, "PPJoy_Ctl_Ioctl", ntStatus);

 return ntStatus;
}

/**************************************************************************/
/* Routine to construct a USB HID Device Descriptor for the joystick.     */
/**************************************************************************/
NTSTATUS PPJoy_GetDeviceDescriptor (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 PHID_DESCRIPTOR	pHidDescriptor;
 USHORT				cbReport;
 UCHAR				rgGameReport[PPJOY_REPORT_MAXSIZE];
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;
 PIO_STACK_LOCATION	IrpStack;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJoy_GetDeviceDescriptor(DeviceObject=0x%p,Irp=0x%p)",
										   DeviceObject,Irp) );

 ntStatus= STATUS_SUCCESS;
 
 IrpStack = IoGetCurrentIrpStackLocation(Irp);
 DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

 /* Return HID descriptor in the buffer pointed to by Irp->UserBuffer */
 pHidDescriptor =  (PHID_DESCRIPTOR) Irp->UserBuffer;

 /* Check that we will not overflow the supplied buffer. Bad karma */
 if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(*pHidDescriptor))
 {
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR,("PPJoy_GetDeviceDescriptor: OutBufferLength(0x%x) < sizeof(HID_DESCRIPTOR)(0x%x)", \
										  IrpStack->Parameters.DeviceIoControl.OutputBufferLength, sizeof(*pHidDescriptor)) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
 }
 else
 {
  /* OK we know the buffer is large enough. Good */
	 
  /* Generate a report (report description really) so we know its size.  */
  /* We use a locally allocated buffer to store it in; and the Generate  */
  /* Report routine will not return more than PPJOY_REPORT_MAXSIZE bytes */
  ntStatus= PPJoy_GenerateReport (DeviceObject,rgGameReport,&cbReport);

  /* If we successfully generated the report, construct the descriptor */
  if (NT_SUCCESS (ntStatus))
  {
   RtlZeroMemory( pHidDescriptor, sizeof(*pHidDescriptor) );

   pHidDescriptor->bLength= sizeof(*pHidDescriptor);
   pHidDescriptor->bDescriptorType= HID_HID_DESCRIPTOR_TYPE;
   pHidDescriptor->bcdHID= HID_REVISION;
   pHidDescriptor->bCountry= 0; /*not localized*/
   pHidDescriptor->bNumDescriptors= 1;
   pHidDescriptor->DescriptorList[0].bReportType= HID_REPORT_DESCRIPTOR_TYPE;
   pHidDescriptor->DescriptorList[0].wReportLength= cbReport;

   /* Return the size of the HID report descriptor */
   Irp->IoStatus.Information = sizeof(*pHidDescriptor);
  }
  else
  {
   /* Report size overflow, don't return a descriptor */
   Irp->IoStatus.Information = 0x0;
  }
 }

 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoy_GetDeviceDescriptor",ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine that returns the USB HID Report Descriptor (describing the     */
/* the joystick input) created by PPJoy_GenerateReport().                 */
/**************************************************************************/
NTSTATUS PPJoy_GetReportDescriptor (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 PDEVICE_EXTENSION		DeviceExtension;
 PIO_STACK_LOCATION		IrpStack;
 NTSTATUS				ntStatus;
 UCHAR					rgGameReport[PPJOY_REPORT_MAXSIZE] ;
 USHORT					cbReport;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJoy_GetReportDescriptor(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 IrpStack = IoGetCurrentIrpStackLocation(Irp);
 DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

 /* Generate our report, based on the current number of axes and buttons */
 ntStatus =  PPJoy_GenerateReport(DeviceObject, rgGameReport, &cbReport);

 /* If we successfully generated the report... */
 if (NT_SUCCESS (ntStatus))
 {
  /* Check to see if supplied buffer will hold our descriptor */     
  if (cbReport > (USHORT)IrpStack->Parameters.DeviceIoControl.OutputBufferLength)
  {
   /* Surprise, buffer too small. */
   PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("HGM_GetReportDescriptor: cbReport(0x%x) OutputBufferLength(0x%x)",\
											cbReport, IrpStack->Parameters.DeviceIoControl.OutputBufferLength));
   ntStatus = STATUS_BUFFER_TOO_SMALL;
  }
  else
  {
   /* Copy report in supplied buffer */
   RtlCopyMemory (Irp->UserBuffer,rgGameReport,cbReport);

   /* Report how many bytes were copied */
   Irp->IoStatus.Information = cbReport;
   
   ntStatus = STATUS_SUCCESS;
  }
 }

 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJOY_GetReportDescriptor",ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine that returns the Vendor and Product IDs for the joystick.      */
/* We will return the same IDs for all the joysticks                      */
/**************************************************************************/
NTSTATUS PPJoy_GetAttributes (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PIO_STACK_LOCATION		IrpStack;
 PDEVICE_EXTENSION		DeviceExtension;
 PHID_DEVICE_ATTRIBUTES	DeviceAttributes;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJoy_GetAttributes(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 ntStatus= STATUS_SUCCESS;

 DeviceExtension= GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
 IrpStack= IoGetCurrentIrpStackLocation(Irp);

 /* Check to see if the supplied buffer is large enough */
 if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(HID_DEVICE_ATTRIBUTES))
 {
  /* No it is not */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetAttributes: cbReport(0x%x) OutputBufferLength(0x%x)",\
										   sizeof (HID_DEVICE_ATTRIBUTES), IrpStack->Parameters.DeviceIoControl.OutputBufferLength) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
 }
 else
 {
  /* Get pointer to result buffer and initialise it with zeros */
  DeviceAttributes= (PHID_DEVICE_ATTRIBUTES) Irp->UserBuffer;
  RtlZeroMemory (DeviceAttributes,sizeof(*DeviceAttributes));

  /* Set the size of the returned result */
  Irp->IoStatus.Information= sizeof(*DeviceAttributes);
  DeviceAttributes->Size= sizeof (*DeviceAttributes);
        
  /* And return the Attributes as requested */
  DeviceAttributes->VendorID= DeviceExtension->Config.VendorID;
  DeviceAttributes->ProductID= DeviceExtension->Config.ProductID;
  DeviceAttributes->VersionNumber= PPJOY_VERSION_NUMBER;
 }
 
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoy_GetAttributes",ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine that returns the Report (i.e. input data) back to HID Class    */
/* driver.                                                                */
/**************************************************************************/
NTSTATUS PPJoy_ReadReport (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;
 PIO_STACK_LOCATION	IrpStack;

 /* ReadReport is not pagable. This is because IOCTL_HID_READ_REPORT can  */
 /* be called at dispatch-level.                                          */ 

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJoy_ReadReport(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 ntStatus= STATUS_SUCCESS;
 IrpStack= IoGetCurrentIrpStackLocation(Irp);

 /* Check that the output buffer supplied is large enough */
 if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(HIDREPORT))
 {
  /* No it is not */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_WARN,("PPJoy_ReadReport: Buffer too small, output=0x%x need=0x%x", \
										 IrpStack->Parameters.DeviceIoControl.OutputBufferLength,sizeof(HIDREPORT)) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
 }
 
 DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

 /* Return device busy status if our DeviceObject has not been started */
 if (!(DeviceExtension->Flags&PPJOY_FLAGS_STARTED))
  ntStatus= STATUS_DEVICE_NOT_READY;

 /* Set zero length read - assume the worst */
 Irp->IoStatus.Information= 0x0;

 if(NT_SUCCESS (ntStatus))
 {
  ntStatus= PPJoy_UpdateJoystickData (DeviceExtension);
  PPJoy_MapRawToReport ((PHIDREPORT)(Irp->UserBuffer),&(DeviceExtension->RawInput),DeviceExtension->Mapping,DeviceExtension->ActiveMap);

//  ntStatus= STATUS_SUCCESS;
//  RtlZeroMemory(Irp->UserBuffer,sizeof(HIDREPORT));

  /* If we successfully read the report */
  if (NT_SUCCESS (ntStatus))
  {
#ifdef DBG
#if DBG
{
 char	Buffer[1024];
 int	Count;

 for (Count=0;Count<sizeof(HIDREPORT);Count++)
  sprintf (Buffer+(Count*2),"%02X",*(((UCHAR*)Irp->UserBuffer)+Count));
 Buffer[Count*2]= 0;
 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2,("PPJoy_ReadReport: sizeof(HIDREPORT)=%d Report contents= %s\n",sizeof(HIDREPORT),Buffer) );
}
#endif
#endif
   Irp->IoStatus.Information = sizeof(HIDREPORT);
  } 
 }

 Irp->IoStatus.Status= ntStatus;

 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT,  "PPJoy_ReadReport", ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine to construct a HID Report Descriptor for the joystick input.   */
/* Based in part in code from the hidjoy.c Win2K DDK sample code.         */
/**************************************************************************/
NTSTATUS PPJoy_GenerateReport (IN PDEVICE_OBJECT DeviceObject, OUT UCHAR rgGameReport[PPJOY_REPORT_MAXSIZE], OUT PUSHORT pCbReport)
{
 /* First, some private declarations for the GenerateReport routine. */
#define	ITEM_VARIABLE		0x02 /* as ITEM_DEFAULT but value is a variable, not an array */
#define	ITEM_HASNULL		0x40 /* as ITEM_DEFAULT but values out of range are considered NULL */
#define	ITEM_ANALOG_AXIS	(ITEM_VARIABLE)
#define	ITEM_POVHAT			(ITEM_VARIABLE|ITEM_HASNULL)
#define	ITEM_BUTTON			(ITEM_VARIABLE)
#define	ITEM_PADDING		0x01 /* Constant (nothing else applies) */

 /* These macros are used to build the report by adding a byte/word  */
 /* at atime while checking for buffer overflows.                    */
 #define NEXT_BYTE(pReport,Data)	if ((ReportSize+=1)<=PPJOY_REPORT_MAXSIZE) *pReport++ = (Data);    
 #define NEXT_LONG(pReport,Data)	if ((ReportSize+=4)<=PPJOY_REPORT_MAXSIZE) *(((LONG UNALIGNED*)(pReport))++) = (Data);

 NTSTATUS		    ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;
 UCHAR				*pucReport; 
 int				ReportSize;
 int				Count;
 
 UCHAR				NumAxes;
 UCHAR				NumHats;
 UCHAR				NumButtons;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_JOYSTICK|PPJOY_FENTRY, ("PPJoy_GenerateReport(ucIn=0x%p,DeviceObject=0x%p)",rgGameReport,DeviceObject) );

 DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

 /* Initialise variables used to build report. The NEXT_BYTE/NEXT_LONG */
 /* macros modifies them.                                              */
 ReportSize= 0;
 pucReport= rgGameReport;

 /* USAGE_PAGE (Generic Desktop) */
 NEXT_BYTE (pucReport,HIDP_GLOBAL_USAGE_PAGE_1);
 NEXT_BYTE (pucReport,HID_USAGE_PAGE_GENERIC);

 /* USAGE (Joystick) */
 NEXT_BYTE (pucReport,HIDP_LOCAL_USAGE_1);
 NEXT_BYTE (pucReport,HID_USAGE_GENERIC_JOYSTICK);

 /* Logical Min is the smallest value that an axis will return */
 NEXT_BYTE (pucReport,HIDP_GLOBAL_LOG_MIN_4);
 NEXT_LONG (pucReport,PPJOY_AXIS_MIN);

 /* Logical Max is the largest value that an axis will return */
 NEXT_BYTE (pucReport,HIDP_GLOBAL_LOG_MAX_4);
 NEXT_LONG (pucReport,PPJOY_AXIS_MAX);

 /* Start a Linked collection */
 NEXT_BYTE (pucReport,HIDP_MAIN_COLLECTION); 
 NEXT_BYTE (pucReport,0x0); 

 /* Define one axis at a time */
 NEXT_BYTE (pucReport,HIDP_GLOBAL_REPORT_COUNT_1);
 NEXT_BYTE (pucReport,0x1);

 /* Each axis is a ULONG (times 8 to get number of bits) */
 NEXT_BYTE (pucReport,HIDP_GLOBAL_REPORT_SIZE);
 NEXT_BYTE (pucReport,8*sizeof(ULONG));

 NumAxes= DeviceExtension->Mapping->NumAxes;
 NumHats= DeviceExtension->Mapping->NumHats;
 NumButtons= DeviceExtension->Mapping->NumButtons;

 PPJOY_DBGPRINT (FILE_JOYSTICK|PPJOY_BABBLE2, ("Num axes: %d (JoyMapping)",NumAxes) );

 /* First, report the axes configured for this joystick */
 for (Count=0;Count<NumAxes;Count++)
 {
  NEXT_BYTE (pucReport,HIDP_LOCAL_USAGE_4);
  NEXT_BYTE (pucReport,DeviceExtension->Mapping->Data[Count*2+1]);
  NEXT_BYTE (pucReport,0x0);
  NEXT_BYTE (pucReport,DeviceExtension->Mapping->Data[Count*2]);
  NEXT_BYTE (pucReport,0x0);

  /* Data Field */
  NEXT_BYTE (pucReport,HIDP_MAIN_INPUT_1);
  NEXT_BYTE (pucReport,ITEM_ANALOG_AXIS);
 }

 /* Then report padding (dummy) fields up to MAX_REPORT_AXES. This is used  */
 /* because the HIDREPORT structure always has MAX_REPORT_AXES, whether     */
 /* the joystick used them or not. This report descriptor must describe     */
 /* exactly sizeof(HIDREPORT) bytes. This also lines up the hats fields.    */
 while (Count<MAX_REPORT_AXES)
 {
  /* Constant Field */
  NEXT_BYTE (pucReport,HIDP_MAIN_INPUT_1);
  NEXT_BYTE (pucReport,ITEM_PADDING);
  Count++;
 }

 PPJOY_DBGPRINT (FILE_JOYSTICK|PPJOY_BABBLE2, ("Num hats: %d (JoyMapping)",NumHats) );

 /* Logical Min is the smallest value that an hat will return */
 NEXT_BYTE (pucReport,HIDP_GLOBAL_LOG_MIN_4);
 NEXT_LONG (pucReport,PPJOY_HAT_MIN);

 /* Logical Max is the largest value that an hat will return */
 NEXT_BYTE (pucReport,HIDP_GLOBAL_LOG_MAX_4);
 NEXT_LONG (pucReport,PPJOY_HAT_MAX);

 /* First, report the hats configured for this joystick */
 for (Count=0;Count<NumHats;Count++)
 {
  NEXT_BYTE (pucReport,HIDP_LOCAL_USAGE_4);
  NEXT_BYTE (pucReport,DeviceExtension->Mapping->Data[(NumAxes+NumButtons)*2+Count*2+1]);
  NEXT_BYTE (pucReport,0x0);
  NEXT_BYTE (pucReport,DeviceExtension->Mapping->Data[(NumAxes+NumButtons)*2+Count*2]);
  NEXT_BYTE (pucReport,0x0);

  /* Data Field */
  NEXT_BYTE (pucReport,HIDP_MAIN_INPUT_1);
  NEXT_BYTE (pucReport,ITEM_ANALOG_AXIS);
 }

 /* Then report padding (dummy) fields up to MAX_REPORT_HATS. This is used  */
 /* because the HIDREPORT structure always has MAX_REPORT_HSTS, whether     */
 /* the joystick used them or not. This report descriptor must describe     */
 /* exactly sizeof(HIDREPORT) bytes. This also lines up the button fields.  */
 while (Count<MAX_REPORT_HATS)
 {
  /* Constant Field */
  NEXT_BYTE (pucReport,HIDP_MAIN_INPUT_1);
  NEXT_BYTE (pucReport,ITEM_PADDING);
  Count++;
 }

 PPJOY_DBGPRINT (FILE_JOYSTICK|PPJOY_BABBLE2, ("Num buttons: %d (JoyMapping)",NumButtons) );

 /* Now the buttons */
 for (Count=0;Count<NumButtons;Count++)
 {
  /* Report size is 1 bit for button */
  NEXT_BYTE (pucReport,HIDP_GLOBAL_REPORT_SIZE);
  NEXT_BYTE (pucReport,0x1);

  NEXT_BYTE (pucReport,HIDP_LOCAL_USAGE_4);
  NEXT_BYTE (pucReport,DeviceExtension->Mapping->Data[NumAxes*2+Count*2+1]);
  NEXT_BYTE (pucReport,0x0);
  NEXT_BYTE (pucReport,DeviceExtension->Mapping->Data[NumAxes*2+Count*2]);
  NEXT_BYTE (pucReport,0x0);

  /* Data field */
  NEXT_BYTE (pucReport,HIDP_MAIN_INPUT_1);
  NEXT_BYTE (pucReport,ITEM_BUTTON);

  /* 7 bits of constant data (padding) */
  NEXT_BYTE (pucReport,HIDP_GLOBAL_REPORT_SIZE);
  NEXT_BYTE (pucReport,0x7);
  NEXT_BYTE (pucReport,HIDP_MAIN_INPUT_1);
  NEXT_BYTE (pucReport,ITEM_PADDING);
 } 

 /* Now we pad up the descriptor until exactly sizeof(HIDREPORT) bytes has */
 /* been described.                                                        */
 if (Count<MAX_REPORT_BUTTONS)
 {
  while ((Count+16)<MAX_REPORT_BUTTONS)
  {
   /* Set 16 bytes/buttons at a time to unused */
   NEXT_BYTE (pucReport,HIDP_GLOBAL_REPORT_SIZE);
   NEXT_BYTE (pucReport,(UCHAR)(16*8));

   /* Define the constant field */
   NEXT_BYTE (pucReport,HIDP_MAIN_INPUT_1);
   NEXT_BYTE (pucReport,ITEM_PADDING);

   Count+= 16;
  }

  /* Constant report for 8 * unused buttons bits */
  NEXT_BYTE (pucReport,HIDP_GLOBAL_REPORT_SIZE);
  NEXT_BYTE (pucReport,(UCHAR)((MAX_REPORT_BUTTONS-Count)*8));

  /* Define the constant field */
  NEXT_BYTE (pucReport,HIDP_MAIN_INPUT_1);
  NEXT_BYTE (pucReport,ITEM_PADDING);
 }

 /* End of collection,  We're done ! */
 NEXT_BYTE (pucReport,HIDP_MAIN_ENDCOLLECTION); 

 /* Check too see that we did not overflow the buffer */
 if (ReportSize>PPJOY_REPORT_MAXSIZE)
 {
  /* Sorry, we need a bigger buffer. */
  PPJOY_DBGPRINT (FILE_JOYSTICK|PPJOY_ERROR, ("PPJoy_GenerateReport: Report (%d bytes) is larger than PPJOY_REPORT_MAXSIZE",ReportSize) );
  ntStatus= STATUS_BUFFER_TOO_SMALL;
  *pCbReport= 0x0;
  RtlZeroMemory(rgGameReport, sizeof(rgGameReport));
 }
 else
 {
  *pCbReport= (USHORT) ReportSize;
  ntStatus= STATUS_SUCCESS;
 }

 PPJOY_DBGPRINT (FILE_JOYSTICK|PPJOY_GEN_REPORT, ("PPJoy_GenerateReport: ReportSize=0x%x", *pCbReport) );
 PPJOY_EXITPROC (FILE_JOYSTICK|PPJOY_FEXIT_STATUSOK, "PPJoy_GenerateReport",ntStatus);
 return ntStatus;

#undef NEXT_BYTE
#undef NEXT_LONG
}

/**************************************************************************/
/* Routine that sets the button and position state for a virtual joystick */
/**************************************************************************/
NTSTATUS PPJoy_SetJoystickState (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PIO_STACK_LOCATION		IrpStack;
 PDEVICE_EXTENSION		JoyDevExtension;
 ULONG					InputBufferLen;
 PJOYSTICK_SET_STATE	InputBuffer;
 ULONG					NumButtons;
 ULONG					NumAxes;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJOY_SetJoystickState(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 ntStatus= STATUS_SUCCESS;

 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 JoyDevExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

 InputBufferLen= IrpStack->Parameters.DeviceIoControl.InputBufferLength;
 InputBuffer= Irp->AssociatedIrp.SystemBuffer;

 if (!InputBuffer)
 {
  /* Hey, who passed a NULL pointer as buffer? */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_SetJoystickState: Stop passing us NULL pointers") );
  ntStatus = STATUS_BUFFER_TOO_SMALL;		/* Can later look for better error code */
  goto Exit;
 }

 /* Check to see if the supplied buffer is large enough */
 if (InputBufferLen<3*sizeof(ULONG))			/* 3*ULONG seems too much - why not ULONG+2*UCHAR??? */
 {
  /* No it is not */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_SetJoystickState: Packet too small - packet size 0x%x",InputBufferLen) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 /* Process packet if we know how*/
 if (InputBuffer->Version==JOYSTICK_STATE_V1)
 {
  NumAxes= InputBuffer->Data[0];
  if (InputBufferLen>=(NumAxes*sizeof(ULONG)+5))
  {
   NumButtons= InputBuffer->Data[NumAxes*sizeof(ULONG)+1];
   if (InputBufferLen>=(NumAxes*sizeof(ULONG)+6+NumButtons))
   {
	/* First do buttons as we need the unmodified "NumAxes" value for number of axes in the input buffer  */
    if (NumButtons>MAX_DIGITAL_RAW)
     NumButtons= MAX_DIGITAL_RAW;

	RtlCopyMemory (JoyDevExtension->RawInput.Digital,&(InputBuffer->Data[NumAxes*sizeof(ULONG)+2]),NumButtons);

    if (NumAxes>MAX_ANALOG_RAW)
     NumAxes= MAX_ANALOG_RAW;

	RtlCopyMemory (JoyDevExtension->RawInput.Analog,&(InputBuffer->Data[1]),NumAxes*sizeof(ULONG));

    ntStatus= STATUS_SUCCESS;
	goto Exit;	
   }
  }

  /* Hmm, somewhere the packet got damaged */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_SetJoystickState: Packet too small - packet size 0x%x",InputBufferLen) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 ntStatus= STATUS_NOT_SUPPORTED;
 
Exit:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoy_SetJoystickState",ntStatus);
 return ntStatus;
}


/**************************************************************************/
/* Routine that return the current button and position state for joystick */
/**************************************************************************/
NTSTATUS PPJoy_GetJoystickState (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PIO_STACK_LOCATION		IrpStack;
 PDEVICE_EXTENSION		JoyDevExtension;

 ULONG					BufLen;
 PJOYSTICK_SET_STATE	Buffer;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJoy_GetJoystickState(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 ntStatus= STATUS_SUCCESS;

 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 JoyDevExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

 ntStatus= PPJoy_UpdateJoystickData(JoyDevExtension);
 if(!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetJoystickState: Error updating current joystick state") );
  goto Exit;
 }

 Buffer= Irp->AssociatedIrp.SystemBuffer;
 BufLen= IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

 if (!Buffer)
 {
  /* Hey, who passed a NULL pointer as buffer? */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetJoystickState: Stop passing us NULL pointers") );
  ntStatus = STATUS_BUFFER_TOO_SMALL;		/* Can later look for better error code */
  goto Exit;
 }

 /* Check to see if the supplied buffer is large enough */
 if (BufLen<6)
 {
  /* No it is not */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetJoystickState: Buffer too small - buffer size 0x%x",BufLen) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 /* Now fill as much of the sucker as we can */
 Buffer->Version= JOYSTICK_STATE_V1;
 Buffer->Data[0]= MAX_ANALOG_RAW;
 Buffer->Data[1+MAX_ANALOG_RAW*sizeof(ULONG)]= MAX_DIGITAL_RAW;

 if (BufLen<(6+MAX_ANALOG_RAW*sizeof(ULONG)+MAX_DIGITAL_RAW*sizeof(UCHAR)))
 {
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_WARN, ("PPJoy_GetJoystickState: Buffer too small - buffer size 0x%x - only returning header",BufLen) );
  Irp->IoStatus.Information= 6;	/* Size of header - version + num analog + num digital */
  ntStatus= STATUS_SUCCESS;
  goto Exit;
 }

 RtlCopyMemory(Buffer->Data+1,JoyDevExtension->RawInput.Analog,MAX_ANALOG_RAW*sizeof(ULONG));
 RtlCopyMemory(Buffer->Data+2+MAX_ANALOG_RAW*sizeof(ULONG),JoyDevExtension->RawInput.Digital,MAX_DIGITAL_RAW*sizeof(UCHAR));

 Irp->IoStatus.Information= 6+MAX_ANALOG_RAW*sizeof(ULONG)+MAX_DIGITAL_RAW*sizeof(UCHAR);
 ntStatus= STATUS_SUCCESS;
 
Exit:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoy_GetJoystickState",ntStatus);
 return ntStatus;
}

/**************************************************************************/
/*                                                                        */
/**************************************************************************/
NTSTATUS PPJoy_SetJoystickMap (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PIO_STACK_LOCATION		IrpStack;
 PDEVICE_OBJECT			JoyDeviceObject;

 ULONG					BufferLen;
 PJOYSTICKMAP_HEADER	Buffer;
 PJOYSTICK_MAP			JoyMap;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJOY_SetJoystickMap(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 JoyDeviceObject= ((PCTLDEV_EXTENSION)DeviceObject->DeviceExtension)->JoyDeviceObject;

 BufferLen= IrpStack->Parameters.DeviceIoControl.InputBufferLength;
 Buffer= Irp->AssociatedIrp.SystemBuffer;

 if (!Buffer)
 {
  /* Hey, who passed a NULL pointer as buffer? */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_SetJoystickMap: Stop passing us NULL pointers") );
  ntStatus = STATUS_BUFFER_TOO_SMALL;		/* Can later look for better error code */
  goto Exit;
 }

 /* Check to see if the supplied buffer is large enough */
 if ((BufferLen<(sizeof(JOYSTICKMAP_HEADER)+4))||(BufferLen<(sizeof(JOYSTICKMAP_HEADER)+Buffer->MapSize)))
 {
  /* No it is not */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("PPJoy_SetJoystickMap: BufferLen= %d sizeof(JOYSTICKMAP_HEADER)= %d Buffer->MapSize= %d",BufferLen,sizeof(JOYSTICKMAP_HEADER),Buffer->MapSize) );
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_SetJoystickMap: Packet too small - packet size 0x%x",BufferLen) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 ntStatus= STATUS_UNSUCCESSFUL;
 /* Process packet if we know how*/
 JoyMap=(PJOYSTICK_MAP) ( ((UCHAR*)Buffer) + sizeof(JOYSTICKMAP_HEADER) );
 if ((Buffer->Version==JOYSTICK_MAP_V1)&&(Buffer->MapSize==PPJOY_ComputeMappingSize(JoyMap)))
 {
  switch (Buffer->MapScope)
  {
   case MAP_SCOPE_DRIVER:
			ntStatus= PPJoy_WriteDriverMapping (JoyDeviceObject,JoyMap,Buffer->JoyType);
			break;

   case MAP_SCOPE_DEVICE:
			ntStatus= PPJoy_WriteDeviceMapping (JoyDeviceObject,JoyMap,Buffer->JoyType);
			break;

   case MAP_SCOPE_DEFAULT:
   default:
			ntStatus= STATUS_NOT_SUPPORTED;
			break;
  }
 }
 else
 {


 }
 
Exit:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoy_SetJoystickMap",ntStatus);
 return ntStatus;
}

/**************************************************************************/
/*                                                                        */
/**************************************************************************/
NTSTATUS PPJoy_GetJoystickMap (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PIO_STACK_LOCATION		IrpStack;
 PDEVICE_OBJECT			JoyDeviceObject;

 ULONG					BufLen;
 PJOYSTICKMAP_HEADER	Buffer;
 PJOYSTICK_MAP			JoyMap;
 int					MapSize;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJOY_GetJoystickMap(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 JoyDeviceObject= ((PCTLDEV_EXTENSION)DeviceObject->DeviceExtension)->JoyDeviceObject;

 Buffer= Irp->AssociatedIrp.SystemBuffer;
 BufLen= IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

 if (!Buffer)
 {
  /* Hey, who passed a NULL pointer as buffer? */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetJoystickMap: Stop passing us NULL pointers") );
  ntStatus = STATUS_BUFFER_TOO_SMALL;		/* Can later look for better error code */
  goto Exit;
 }

 /* Check to see if the supplied buffer is large enough - part 1 */
 if (BufLen<sizeof(JOYSTICKMAP_HEADER))
 {
  /* No it is not */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetJoystickMap: Buffer too small - buffer size 0x%x",BufLen) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 /* Check for a know request version... */
 if (Buffer->Version!=JOYSTICK_MAP_V1)
 {
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetJoystickMap: Wrong version in header 0x%x",Buffer->Version) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 /* Read the requested joystick map */
 switch (Buffer->MapScope)
 {
  case MAP_SCOPE_DEFAULT:
			JoyMap= PPJoy_BuildDefaultMapping (Buffer->JoyType);
			break;

  case MAP_SCOPE_DRIVER:
			JoyMap= PPJoy_ReadDriverMapping (JoyDeviceObject,Buffer->JoyType);
			break;

  case MAP_SCOPE_DEVICE:
			JoyMap= PPJoy_ReadDeviceMapping (JoyDeviceObject,Buffer->JoyType);
			break;

  default:	ntStatus= STATUS_NOT_SUPPORTED;
			goto Exit;
 }

 if (!JoyMap)
 {
  /* Could not read joystick map for some reason - could be it doesn't exist */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetJoystickMap: Could not read joystick map") );
  ntStatus = STATUS_UNSUCCESSFUL;
  goto Exit;
 }

 /* Check to see if the supplied buffer is large enough - part 2 */
 MapSize= PPJOY_ComputeMappingSize(JoyMap);
 if (BufLen<(sizeof(JOYSTICKMAP_HEADER)+MapSize))
 {
  /* No it is not */
  ExFreePool(JoyMap);
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetJoystickMap: Buffer too small - buffer size 0x%x, expected 0x%x",BufLen,sizeof(JOYSTICKMAP_HEADER)+MapSize) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 /* OK, success */
 Buffer->MapSize= MapSize;
 RtlCopyMemory(((UCHAR*)Buffer)+sizeof(JOYSTICKMAP_HEADER),JoyMap,MapSize);
 ExFreePool(JoyMap);

 Irp->IoStatus.Information= sizeof(JOYSTICKMAP_HEADER)+MapSize;
 ntStatus= STATUS_SUCCESS;
 
Exit:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoy_GetJoystickMap",ntStatus);
 return ntStatus;
}

/**************************************************************************/
/*                                                                        */
/**************************************************************************/
NTSTATUS PPJoy_DelJoystickMap (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PIO_STACK_LOCATION		IrpStack;
 PDEVICE_OBJECT			JoyDeviceObject;

 ULONG					BufferLen;
 PJOYSTICKMAP_HEADER	Buffer;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJOY_DelJoystickMap(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 JoyDeviceObject= ((PCTLDEV_EXTENSION)DeviceObject->DeviceExtension)->JoyDeviceObject;

 BufferLen= IrpStack->Parameters.DeviceIoControl.InputBufferLength;
 Buffer= Irp->AssociatedIrp.SystemBuffer;

 if (!Buffer)
 {
  /* Hey, who passed a NULL pointer as buffer? */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_DelJoystickMap: Stop passing us NULL pointers") );
  ntStatus = STATUS_BUFFER_TOO_SMALL;		/* Can later look for better error code */
  goto Exit;
 }

 /* Check to see if the supplied buffer is large enough */
 if (BufferLen<sizeof(JOYSTICKMAP_HEADER))
 {
  /* No it is not */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_DelJoystickMap: Packet too small - packet size 0x%x",BufferLen) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 ntStatus= STATUS_UNSUCCESSFUL;
 /* Process packet if we know how*/
 if (Buffer->Version==JOYSTICK_MAP_V1)
 {
  switch (Buffer->MapScope)
  {
   case MAP_SCOPE_DRIVER:
			ntStatus= PPJoy_DeleteDriverMapping (JoyDeviceObject,Buffer->JoyType);
			break;

   case MAP_SCOPE_DEVICE:
			ntStatus= PPJoy_DeleteDeviceMapping (JoyDeviceObject,Buffer->JoyType);
			break;

   case MAP_SCOPE_DEFAULT:
   default:
			ntStatus= STATUS_NOT_SUPPORTED;
			break;
  }
 }
 
Exit:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoy_DelJoystickMap",ntStatus);
 return ntStatus;
}

NTSTATUS PPJoy_SetJoyTiming (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PIO_STACK_LOCATION		IrpStack;
 PDEVICE_OBJECT			JoyDeviceObject;

 ULONG					BufferLen;
 PTIMING_HEADER			Buffer;

 UCHAR					JoyType;
 ULONG					TimingSize;
 PJOYTIMING				JoyTiming;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJOY_SetJoyTiming(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 JoyDeviceObject= ((PCTLDEV_EXTENSION)DeviceObject->DeviceExtension)->JoyDeviceObject;

 BufferLen= IrpStack->Parameters.DeviceIoControl.InputBufferLength;
 Buffer= Irp->AssociatedIrp.SystemBuffer;

 if (!Buffer)
 {
  /* Hey, who passed a NULL pointer as buffer? */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_SetJoyTiming: Stop passing us NULL pointers") );
  ntStatus = STATUS_BUFFER_TOO_SMALL;		/* Can later look for better error code */
  goto Exit;
 }

 TimingSize= PPJOY_ComputeTimingSize(Buffer->JoyType);
 if (!TimingSize)
 {
  /* No timing parameters for this device type */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_WARN, ("PPJoy_SetJoyTiming: Cannot set timing for this type of device") );
  ntStatus = STATUS_NOT_SUPPORTED;		/* Can later look for better error code */
  goto Exit;
 }

 /* Check to see if the supplied buffer is the right size */
 if (BufferLen!=(sizeof(TIMING_HEADER)+TimingSize))
 {
  /* No it is not */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("PPJoy_SetJoyTiming: Wrong size packet. BufferLen= %d sizeof(TIMING_HEADER)= %d TimingSize= %d",BufferLen,sizeof(TIMING_HEADER),TimingSize) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 ntStatus= STATUS_UNSUCCESSFUL;
 /* Process packet if we know how*/
 JoyTiming=(PJOYTIMING) ( ((UCHAR*)Buffer) + sizeof(TIMING_HEADER) );
 if ((Buffer->Version==TIMING_V1)&&(Buffer->TimingSize==TimingSize))
 {
  switch (Buffer->TimingScope)
  {
   case TIMING_SCOPE_DRIVER:
			ntStatus= PPJoy_WriteDriverTiming (JoyDeviceObject,JoyTiming,Buffer->JoyType);
			break;

   case TIMING_SCOPE_DEVICE:
			ntStatus= PPJoy_WriteDeviceTiming (JoyDeviceObject,JoyTiming,Buffer->JoyType);
			break;

   case TIMING_SCOPE_DEFAULT:
   default:
			ntStatus= STATUS_NOT_SUPPORTED;
			break;
  }
 }
 
Exit:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoy_SetJoyTiming",ntStatus);
 return ntStatus;
}

NTSTATUS PPJoy_GetJoyTiming (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PIO_STACK_LOCATION		IrpStack;
 PDEVICE_OBJECT			JoyDeviceObject;

 ULONG					BufferLen;
 PTIMING_HEADER			Buffer;

 PJOYTIMING				JoyTiming;
 ULONG					TimingSize;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJOY_GetJoyTiming(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 JoyDeviceObject= ((PCTLDEV_EXTENSION)DeviceObject->DeviceExtension)->JoyDeviceObject;

 Buffer= Irp->AssociatedIrp.SystemBuffer;
 BufferLen= IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

 if (!Buffer)
 {
  /* Hey, who passed a NULL pointer as buffer? */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetJoyTiming: Stop passing us NULL pointers") );
  ntStatus = STATUS_BUFFER_TOO_SMALL;		/* Can later look for better error code */
  goto Exit;
 }

 TimingSize= PPJOY_ComputeTimingSize(Buffer->JoyType);
 if (!TimingSize)
 {
  /* No timing parameters for this device type */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_WARN, ("PPJoy_GetJoyTiming: Cannot get timing for this type of device") );
  ntStatus = STATUS_NOT_SUPPORTED;		/* Can later look for better error code */
  goto Exit;
 }

 /* Check to see if the supplied buffer is the right size */
 if (BufferLen<(sizeof(TIMING_HEADER)+TimingSize))
 {
  /* No it is not */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetJoyTiming: Buffer too small. BufferLen= %d sizeof(TIMING_HEADER)= %d TimingSize= %d",BufferLen,sizeof(TIMING_HEADER),TimingSize) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 /* Check for a know request version... */
 if (Buffer->Version!=TIMING_V1)
 {
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetJoyTiming: Wrong version in header 0x%x",Buffer->Version) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 /* Read the requested joystick map */
 switch (Buffer->TimingScope)
 {
  case TIMING_SCOPE_DEFAULT:
			JoyTiming= PPJoy_BuildDefaultTiming (Buffer->JoyType);
			break;

  case TIMING_SCOPE_DRIVER:
			JoyTiming= PPJoy_ReadDriverTiming (JoyDeviceObject,Buffer->JoyType);
			break;

  case TIMING_SCOPE_DEVICE:
			JoyTiming= PPJoy_ReadDeviceTiming (JoyDeviceObject,Buffer->JoyType);
			break;

  default:	ntStatus= STATUS_NOT_SUPPORTED;
			goto Exit;
 }

 if (!JoyTiming)
 {
  /* Could not read joystick map for some reason - could be it doesn't exist */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetJoyTiming: Could not read joystick timing") );
  ntStatus = STATUS_UNSUCCESSFUL;
  goto Exit;
 }

 /* OK, success */
 Buffer->TimingSize= TimingSize;
 RtlCopyMemory(((UCHAR*)Buffer)+sizeof(TIMING_HEADER),JoyTiming,TimingSize);
 ExFreePool(JoyTiming);

 Irp->IoStatus.Information= sizeof(TIMING_HEADER)+TimingSize;
 ntStatus= STATUS_SUCCESS;
 
Exit:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoy_GetJoyTiming",ntStatus);
 return ntStatus;
}

NTSTATUS PPJoy_DelJoyTiming (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PIO_STACK_LOCATION		IrpStack;
 PDEVICE_OBJECT			JoyDeviceObject;

 ULONG					BufferLen;
 PTIMING_HEADER			Buffer;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJOY_DelJoyTiming(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 JoyDeviceObject= ((PCTLDEV_EXTENSION)DeviceObject->DeviceExtension)->JoyDeviceObject;

 BufferLen= IrpStack->Parameters.DeviceIoControl.InputBufferLength;
 Buffer= Irp->AssociatedIrp.SystemBuffer;

 if (!Buffer)
 {
  /* Hey, who passed a NULL pointer as buffer? */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_DelJoyTiming: Stop passing us NULL pointers") );
  ntStatus = STATUS_BUFFER_TOO_SMALL;		/* Can later look for better error code */
  goto Exit;
 }

 /* Check to see if the supplied buffer is the right size */
 if (BufferLen!=sizeof(TIMING_HEADER))
 {
  /* No it is not */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_DelJoyTiming: Buffer too small. BufferLen= %d sizeof(TIMING_HEADER)= %d",BufferLen,sizeof(TIMING_HEADER)) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 ntStatus= STATUS_UNSUCCESSFUL;
 /* Process packet if we know how*/
 if (Buffer->Version==TIMING_V1)
 {
  switch (Buffer->TimingScope)
  {
   case TIMING_SCOPE_DRIVER:
			ntStatus= PPJoy_DeleteDriverTiming (JoyDeviceObject,Buffer->JoyType);
			break;

   case TIMING_SCOPE_DEVICE:
			ntStatus= PPJoy_DeleteDeviceTiming (JoyDeviceObject,Buffer->JoyType);
			break;

   case TIMING_SCOPE_DEFAULT:
   default:
			ntStatus= STATUS_NOT_SUPPORTED;
			break;
  }
 }
 
Exit:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoy_DelJoyTiming",ntStatus);
 return ntStatus;
}

NTSTATUS PPJoy_ActJoyTiming (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PIO_STACK_LOCATION		IrpStack;
 PDEVICE_OBJECT			JoyDeviceObject;

 ULONG					BufferLen;
 PTIMING_HEADER			Buffer;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJOY_ActJoyTiming(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 JoyDeviceObject= ((PCTLDEV_EXTENSION)DeviceObject->DeviceExtension)->JoyDeviceObject;

 BufferLen= IrpStack->Parameters.DeviceIoControl.InputBufferLength;
 Buffer= Irp->AssociatedIrp.SystemBuffer;

 if (!Buffer)
 {
  /* Hey, who passed a NULL pointer as buffer? */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_ActJoyTiming: Stop passing us NULL pointers") );
  ntStatus = STATUS_BUFFER_TOO_SMALL;		/* Can later look for better error code */
  goto Exit;
 }

 /* Check to see if the supplied buffer is the right size */
 if (BufferLen!=sizeof(TIMING_HEADER))
 {
  /* No it is not */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_ActJoyTiming: Buffer too small. BufferLen= %d sizeof(TIMING_HEADER)= %d",BufferLen,sizeof(TIMING_HEADER)) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 ntStatus= STATUS_UNSUCCESSFUL;
 /* Process packet if we know how*/
 if (Buffer->Version==TIMING_V1)
 {
  ntStatus= PPJoy_LoadJoystickTiming (JoyDeviceObject);
 }
 
Exit:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoy_DelJoyTiming",ntStatus);
 return ntStatus;
}


NTSTATUS PPJoy_GetPPJoyOptions (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PIO_STACK_LOCATION		IrpStack;
 ULONG					InputBufferLen;
 ULONG					OutputBufferLen;
 PPPJOY_OPTIONS			InputBuffer;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJOY_GetPPJoyOptions(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 ntStatus= STATUS_SUCCESS;

 IrpStack= IoGetCurrentIrpStackLocation(Irp);

 InputBufferLen= IrpStack->Parameters.DeviceIoControl.InputBufferLength;
 OutputBufferLen= IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
 InputBuffer= Irp->AssociatedIrp.SystemBuffer;

 if (!InputBuffer)
 {
  /* Hey, who passed a NULL pointer as buffer? */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_GetPPJoyOptions: Stop passing us NULL pointers") );
  ntStatus = STATUS_BUFFER_TOO_SMALL;		/* Can later look for better error code */
  goto Exit;
 }

 /* Check to see if the supplied buffer is large enough */
 if ((InputBufferLen!=sizeof(PPJOY_OPTIONS))||(OutputBufferLen!=sizeof(PPJOY_OPTIONS)))
 {
  /* No it is not */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_SetPPJoyOptions: Packet too small - packet size 0x%x in  0x%x out",InputBufferLen,OutputBufferLen) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 switch (InputBuffer->Option)
 {
  case	PPJOY_OPTION_RUNTIMEDEBUG:
		InputBuffer->Value= PPJoy_RuntimeDebug;
		ntStatus= STATUS_SUCCESS;
		break;

  default:
		ntStatus= STATUS_NOT_SUPPORTED;
		goto Exit;
 }
 
 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("PPJOY_GetPPJoyOptions option 0x%X value 0x%X ",InputBuffer->Option,InputBuffer->Value) );
 Irp->IoStatus.Information= sizeof(PPJOY_OPTIONS);

Exit:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoy_GetPPJoyOptions",ntStatus);
 return ntStatus;
}


NTSTATUS PPJoy_SetPPJoyOptions (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;
 PIO_STACK_LOCATION		IrpStack;
 ULONG					InputBufferLen;
 PPPJOY_OPTIONS			InputBuffer;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJOY_SetPPJoyOptions(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 ntStatus= STATUS_SUCCESS;

 IrpStack= IoGetCurrentIrpStackLocation(Irp);

 InputBufferLen= IrpStack->Parameters.DeviceIoControl.InputBufferLength;
 InputBuffer= Irp->AssociatedIrp.SystemBuffer;

 if (!InputBuffer)
 {
  /* Hey, who passed a NULL pointer as buffer? */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_SetPPJoyOptions: Stop passing us NULL pointers") );
  ntStatus = STATUS_BUFFER_TOO_SMALL;		/* Can later look for better error code */
  goto Exit;
 }

 /* Check to see if the supplied buffer is large enough */
 if (InputBufferLen!=sizeof(PPJOY_OPTIONS))	
 {
  /* No it is not */
  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_ERROR, ("PPJoy_SetPPJoyOptions: Packet too small - packet size 0x%x",InputBufferLen) );
  ntStatus = STATUS_BUFFER_TOO_SMALL;
  goto Exit;
 }

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("PPJOY_SetPPJoyOptions option 0x%X value 0x%X ",InputBuffer->Option,InputBuffer->Value) );

 switch (InputBuffer->Option)
 {
  case	PPJOY_OPTION_RUNTIMEDEBUG:
		PPJoy_RuntimeDebug= InputBuffer->Value;
		ntStatus= PPJoy_WriteDriverConfig();
		break;

  default:
		ntStatus= STATUS_NOT_SUPPORTED;
		break;
 }
 
Exit:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoy_SetPPJoyOptions",ntStatus);
 return ntStatus;
}


