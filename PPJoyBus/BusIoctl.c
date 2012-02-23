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
/**  BusIoctl.c  Routines to handle IOCTL and Internal IOCTL requests     **/
/**              for the Joystick bus enumerator FDO                      **/
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
    #pragma alloc_text (PAGE,PPJoyBus_InternalIoctl_Bus)
    #pragma alloc_text (PAGE,PPJoyBus_Ioctl_Bus)
    #pragma alloc_text (PAGE,PPJoyBus_AddJoystick)
    #pragma alloc_text (PAGE,PPJoyBus_DelJoystick)
    #pragma alloc_text (PAGE,PPJoyBus_EnumJoystick)
    #pragma alloc_text (PAGE,PPJoyBus_GetOptions)
    #pragma alloc_text (PAGE,PPJoyBus_SetOptions)
#endif

/**************************************************************************/
/* Routine that handles Internal IOCTL requests for the Bus FDO. Since we */
/* don't support any Internal IOCTLs we will simply pass to request down  */
/* to the next lower driver.                                              */
/**************************************************************************/
NTSTATUS PPJoyBus_InternalIoctl_Bus (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS				ntStatus;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJoyBus_InternalIoctl_Bus (DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_WARN,("Unknown or unsupported IOCTL (%x)",IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl.IoControlCode));

 IoSkipCurrentIrpStackLocation (Irp);
 ntStatus= IoCallDriver (((PBUS_DEVICE_DATA)DeviceObject->DeviceExtension)->NextLowerDriver,Irp);

 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoyBus_InternalIoctl_Bus",ntStatus );
 return ntStatus;
}

/**************************************************************************/
/* Routine to process IOCTLs sent to the BUS Device Object. We expect     */
/* requests to add, delete and enumerate joysticks. Other IOCTLS will be  */
/* passed down to the Bus PDO.                                            */
/**************************************************************************/
NTSTATUS PPJoyBus_Ioctl_Bus (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 PIO_STACK_LOCATION	IrpStack;

 ULONG				InLen;
 ULONG				OutLen;
 PVOID				Buffer;
 ULONG				BytesUsed;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FENTRY, ("PPJoyBus_Ioctl_Bus (DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 /* Setup pointers to all the fields in the IRP */
 IrpStack= IoGetCurrentIrpStackLocation(Irp);
 InLen= IrpStack->Parameters.DeviceIoControl.InputBufferLength;
 OutLen= IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
 Buffer= Irp->AssociatedIrp.SystemBuffer;

 /* Default return code */
 ntStatus= STATUS_INVALID_PARAMETER;
 BytesUsed= 0;

 /* Decide to do with the IRP */
 switch (IrpStack->Parameters.DeviceIoControl.IoControlCode)
 {
  case IOCTL_PPJOYBUS_ADD_JOY:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("IOCTL_PPJOYBUS_ADD_JOY") );
		if (InLen==sizeof(JOYSTICK_ADD_DATA))
		 ntStatus= PPJoyBus_AddJoystick (DeviceObject,&((PJOYSTICK_ADD_DATA)Buffer)->JoyData,((PJOYSTICK_ADD_DATA)Buffer)->Persistent);
		break;

  case IOCTL_PPJOYBUS_DEL_JOY:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPJOYBUS_DEL_JOY") );
		if (InLen==sizeof(JOYSTICK_DEL_DATA))
		 ntStatus= PPJoyBus_DelJoystick (DeviceObject,&((PJOYSTICK_DEL_DATA)Buffer)->JoyData);
		break;

  case IOCTL_PPJOYBUS_ENUM_JOY:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPJOYBUS_GET_JOY") );

		if (OutLen>=sizeof(JOYSTICK_ENUM_DATA))
		 ntStatus= PPJoyBus_EnumJoystick (DeviceObject,(PJOYSTICK_ENUM_DATA)Buffer,OutLen,&BytesUsed);
		break;

  case IOCTL_PPJOYBUS_GET_OPTIONS:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPJOYBUS_GET_OPTIONS") );

		if ((InLen==sizeof(PPJOY_OPTIONS))&&(OutLen==sizeof(PPJOY_OPTIONS)))
		 ntStatus= PPJoyBus_GetOptions (DeviceObject,(PPPJOY_OPTIONS)Buffer,&BytesUsed);
		break;

  case IOCTL_PPJOYBUS_SET_OPTIONS:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("IOCTL_PPJOYBUS_SET_OPTIONS") );

		if (InLen==sizeof(PPJOY_OPTIONS))
		 ntStatus= PPJoyBus_SetOptions (DeviceObject,(PPPJOY_OPTIONS)Buffer,&BytesUsed);
		break;

  default:
		PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_WARN, ("Unknown or unsupported IOCTL (%x)",IrpStack->Parameters.DeviceIoControl.IoControlCode) );

		/* Do don't know this IOCTL so we pass it along to lower driver */
		IoSkipCurrentIrpStackLocation (Irp);
		ntStatus= IoCallDriver (((PBUS_DEVICE_DATA)DeviceObject->DeviceExtension)->NextLowerDriver,Irp);
		goto ExitNoComplete;
 }

 /* Set the real return status in Irp and complete for IRPs that we handle */
 Irp->IoStatus.Status= ntStatus;
 Irp->IoStatus.Information= BytesUsed;
 IoCompleteRequest (Irp,IO_NO_INCREMENT);

ExitNoComplete:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoyBus_Ioctl_Bus",ntStatus );
 return ntStatus;
}

/**************************************************************************/
/* Routine to process Add-Joystick requests. If AddToReg is set the       */
/* joystick definition will also be written to the registry so that it    */
/* will be recreate at the next boot.                                     */
/*                                                                        */
/* We will create a PDO for the new joystick and add it to our PDO list.  */
/**************************************************************************/
NTSTATUS PPJoyBus_AddJoystick (IN PDEVICE_OBJECT BusDeviceObject, IN PJOYSTICK_CONFIG1 Config, IN ULONG AddToReg)
{
 NTSTATUS			ntStatus;
 WCHAR				ValueBuffer[256];
 WCHAR				DataBuffer[]= L"Driver Created";
 PDEVICE_OBJECT		JoyDeviceObject;
 PJOY_DEVICE_DATA	JoyDeviceData;
 PBUS_DEVICE_DATA	BusDeviceData;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("PPJoyBus_AddJoystick: Adding joystick type 0x%X unit 0x%X on port 0x%X (AddToReg= %d)",Config->JoyType,Config->UnitNumber+1,Config->LPTNumber,AddToReg) );

 /* If flag is set that we need to add definition to the registry */
 if (AddToReg)
 {
  /* Create the ValueName to be added to the registry. The format of       */
  /* ValueName must match the format expected by PPJoyBus_CrackParamString */
  /* Also watch out not to overflow ValueBuffer!!!                         */
  swprintf (ValueBuffer, L"p%02Xt%02Xu%02X",Config->LPTNumber,Config->JoyType,Config->UnitNumber+1);

  /* Attempt to write the new Value to the registry. If we fail then we */
  /* will simply ignore the error and continue on our way.              */
  ntStatus= RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE,Globals.ParamRegistryPath.Buffer,ValueBuffer,REG_BINARY,Config,Config->Size);
  if (!NT_SUCCESS (ntStatus)) 
   PPJOY_DBGPRINT (FILE_IOCTL | PPJOY_WARN, ("RtlWriteRegistryValue error 0x%x, not adding to registry",ntStatus) );
 }

 /* Create the PDO for the new joystick. */
 ntStatus= IoCreateDevice (BusDeviceObject->DriverObject,sizeof(JOY_DEVICE_DATA),NULL,FILE_DEVICE_PPORTJOY,
						   0,FALSE,&JoyDeviceObject);

 /* If we fail to add the PDO then exit (fail this routine.) */
 if (!NT_SUCCESS (ntStatus))
 {
  PPJoyBus_WriteEventLog (PPJ_MSG_ERRORJOYCREATE,&ntStatus,sizeof(ntStatus),L"");
  PPJOY_DBGPRINT (FILE_IOCTL | PPJOY_ERROR, ("Error calling IoCreateDevice. error 0x%x",ntStatus) );
  goto Exit;
 }

 /* Initialise DeviceExtension for the new Joystick device */
 JoyDeviceData= (PJOY_DEVICE_DATA) JoyDeviceObject->DeviceExtension;
 RtlZeroMemory (JoyDeviceData,sizeof(JOY_DEVICE_DATA));

 /* Add a pointer to the containing DeviceObject to the DeviceExtension */
 JoyDeviceData->Self= JoyDeviceObject;
 /* Tell NT the code for this driver is pagable */
 JoyDeviceObject->Flags|= DO_POWER_PAGABLE;
 /* Make ourselves a copy of the joystick parameters */ 
 RtlCopyMemory (&JoyDeviceData->Config,Config,sizeof(JoyDeviceData->Config));

 /* Lets see if the underlying parallel port is present. If so, set the   */
 /* HW Present flag - else the PnP notification routine will do so later. */
 if ((JoyDeviceData->Config.LPTNumber<=MAX_PARPORTS)&&(Globals.ParPorts[JoyDeviceData->Config.LPTNumber-1].DevObj))
 {
  JoyDeviceData->Flags|= PPJFLAGS_HWFOUND;
  JoyDeviceData->Config.PortAddress= Globals.ParPorts[JoyDeviceData->Config.LPTNumber-1].PortAddr;
 }
 
 /* If LPTNumber=0 (IOCTL based joystick) then the hardware is present    */
 if (JoyDeviceData->Config.LPTNumber==0)
 {
  JoyDeviceData->Flags|= PPJFLAGS_HWFOUND;
  JoyDeviceData->Config.PortAddress= 0;
 }

 /* Initialise the event for device removal */
 KeInitializeEvent (&JoyDeviceData->RemoveEvent,SynchronizationEvent,FALSE);

 KeEnterCriticalRegion();

 /* Add our device to the end of the linked list of joystick devices */
 BusDeviceData= (PBUS_DEVICE_DATA) BusDeviceObject->DeviceExtension;
 ExAcquireFastMutex (&Globals.Mutex);
 InsertTailList (&BusDeviceData->JoystickList,&JoyDeviceData->ListEntry);
 ExReleaseFastMutex (&Globals.Mutex);
  
 KeLeaveCriticalRegion();

 /* After initialisation is completed, clear the flag to say we are done. */
 JoyDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

 /* Then tell the I/O Manager / Pnp to look for new devices */
 IoInvalidateDeviceRelations (BusDeviceData->UnderlyingPDO, BusRelations);

Exit:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoyBus_AddJoystick",ntStatus );
 return ntStatus;
}

/**************************************************************************/
/* Routine to process Del-Joystick requests. We attempt to delete the     */
/* joystick definition from the registry and then set a flag in the       */
/* DeviceExtenstion. This flag will cause the PDO not to be reported      */
/* during the next Bus scan. Then we get a REMOVE PnP IRP.                */
/**************************************************************************/
NTSTATUS PPJoyBus_DelJoystick (IN PDEVICE_OBJECT BusDeviceObject, IN PJOYSTICK_CONFIG1 Config)
{
 NTSTATUS				ntStatus;
 WCHAR					ValueBuffer[256];
 PLIST_ENTRY			pEntry;
 PBUS_DEVICE_DATA		BusDeviceData;
 PJOY_DEVICE_DATA		JoyDeviceData;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("PPJoyBus_DelJoystick: Deleting joystick type 0x%X from port 0x%X",Config->JoyType,Config->LPTNumber) );

 /* Delete the definition of this joystick from the registry. This format */
 /* definition should match that of PPJoyBus_AddJoystick. If we fail to   */
 /* remove the registry entry we simply continue...                       */
 swprintf (ValueBuffer, L"p%02Xt%02Xu%02X",Config->LPTNumber,Config->JoyType,Config->UnitNumber+1);

 ntStatus= RtlDeleteRegistryValue (RTL_REGISTRY_ABSOLUTE,Globals.ParamRegistryPath.Buffer,ValueBuffer);
 if (!NT_SUCCESS (ntStatus)) 
  PPJOY_DBGPRINT (FILE_IOCTL | PPJOY_WARN, ("RtlDeleteRegistryValue error 0x%x, not deleting from registry",ntStatus) );

 /* Prepare to access the list of joystick PDOs */
 BusDeviceData= (PBUS_DEVICE_DATA) BusDeviceObject->DeviceExtension;
 KeEnterCriticalRegion();
 ExAcquireFastMutex (&Globals.Mutex);

 /* Now find the Joystick DeviceObject in our list and flag it as deleted. */
 pEntry= BusDeviceData->JoystickList.Flink;
 while (pEntry!=&BusDeviceData->JoystickList)
 {
  JoyDeviceData= CONTAINING_RECORD (pEntry,JOY_DEVICE_DATA,ListEntry);
  pEntry= pEntry->Flink;

  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("Found joystick type 0x%X unit 0x%X on port 0x%X",JoyDeviceData->Config.JoyType,JoyDeviceData->Config.UnitNumber+1,JoyDeviceData->Config.LPTNumber) );

  if ( (JoyDeviceData->Config.JoyType==Config->JoyType)&&
       (JoyDeviceData->Config.UnitNumber==Config->UnitNumber)&&
       (JoyDeviceData->Config.LPTNumber==Config->LPTNumber) )
  {
   PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("Now deleting joystick type 0x%X unit 0x%X from port 0x%X",JoyDeviceData->Config.JoyType,JoyDeviceData->Config.UnitNumber+1,JoyDeviceData->Config.LPTNumber) );
   JoyDeviceData->Flags|= PPJFLAGS_UNPLUGGED;
  }
 }

 /* Let go of our exclusive access to the joystick PDO list */
 ExReleaseFastMutex (&Globals.Mutex);
 KeLeaveCriticalRegion();
 
 /* Tell I/O manager to rescan the bus - detect the deleted devices. */
 IoInvalidateDeviceRelations (BusDeviceData->UnderlyingPDO, BusRelations);

 ntStatus= STATUS_SUCCESS;

 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoyBus_DelJoystick",ntStatus );
 return ntStatus;
}

/**************************************************************************/
/* Routine to process Enum-Joystick requests. We will return all          */
/* joysticks that are not marked as UNPLUGGED or REMOVED.                 */
/**************************************************************************/
NTSTATUS PPJoyBus_EnumJoystick (IN PDEVICE_OBJECT BusDeviceObject, IN PJOYSTICK_ENUM_DATA EnumData, IN ULONG BufSize, OUT PULONG BytesUsed)
{
 NTSTATUS				ntStatus;
 PLIST_ENTRY			pEntry;
 PBUS_DEVICE_DATA		BusDeviceData;
 PJOY_DEVICE_DATA		JoyDeviceData;

 int					NumSticks;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("PPJoyBus_EnumJoystick (BusDeviceObject=0x%p)",BusDeviceObject) );

 /* Caller makes sure the buffer is at least sizeof (JOYSTICK_ENUM_DATA) */

 NumSticks= 0;
 BusDeviceData= (PBUS_DEVICE_DATA) BusDeviceObject->DeviceExtension;

 EnumData->Count= 0;
 *BytesUsed= sizeof(JOYSTICK_ENUM_DATA)-sizeof(JOYSTICK_CONFIG1);

 /* Make sure we get exclusive access to the joystick PDOs list */
 KeEnterCriticalRegion();
 ExAcquireFastMutex (&Globals.Mutex);

 /* Scan through the joystick PDO list. Count the joysticks as we go on */
 /* and report as many of them as the buffer size allows.               */
 pEntry= BusDeviceData->JoystickList.Flink;
 while (pEntry!=&BusDeviceData->JoystickList)
 {
  JoyDeviceData= CONTAINING_RECORD (pEntry,JOY_DEVICE_DATA,ListEntry);
  pEntry= pEntry->Flink;

  PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE2, ("Found joystick type 0x%X unit 0x%X on port 0x%X",JoyDeviceData->Config.JoyType,JoyDeviceData->Config.UnitNumber+1,JoyDeviceData->Config.LPTNumber) );

  /* Make sure that the REMOVED and UNPLUGGED flags are not set. */
  if (!(JoyDeviceData->Flags&(PPJFLAGS_REMOVED|PPJFLAGS_UNPLUGGED)))
  {
   NumSticks++;
   /* Add it to the list if there is space left */
   if ( (*BytesUsed+sizeof(JOYSTICK_CONFIG1))<=BufSize )
   {
    (*BytesUsed) += sizeof(JOYSTICK_CONFIG1);
    RtlCopyMemory (EnumData->Joysticks+EnumData->Count,&JoyDeviceData->Config,sizeof(JOYSTICK_CONFIG1));
    EnumData->Count++; 
   }
  }
 }

 /* Give up exclusive access to the joystick PDO list */
 ExReleaseFastMutex (&Globals.Mutex);
 KeLeaveCriticalRegion();

 EnumData->Size= sizeof(JOYSTICK_ENUM_DATA)+sizeof(JOYSTICK_CONFIG1)*(NumSticks-1);

 /* Looks like there is no way to return a "Buffer Overflow" result to */
 /* Win32 land. Ugh. So we always returns STATUS_SUCCESS. For now.     */
 ntStatus= EnumData->Size<=BufSize?STATUS_SUCCESS:STATUS_BUFFER_OVERFLOW;
 
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoyBus_EnumJoystick",ntStatus );
 return ntStatus;
}

NTSTATUS PPJoyBus_GetOptions (IN PDEVICE_OBJECT BusDeviceObject, IN PPPJOY_OPTIONS Option, OUT PULONG BytesOut)
{
 NTSTATUS				ntStatus;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("PPJoyBus_GetOptions (BusDeviceObject=0x%p)",BusDeviceObject) );

 switch (Option->Option)
 {
  case	PPJOY_OPTION_RUNTIMEDEBUG:
		Option->Value= PPJoy_Opt_RuntimeDebug;
		ntStatus= STATUS_SUCCESS;
		break;

  case	PPJOY_OPTION_PORTFLAGS:
		Option->Value= PPJoy_Opt_PortFlags;
		ntStatus= STATUS_SUCCESS;
		break;

  default:
		ntStatus= STATUS_NOT_SUPPORTED;
		goto Exit;
 }
 
 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("PPJoyBus_GetOptions option 0x%X value 0x%X ",Option->Option,Option->Value) );
 *BytesOut= sizeof(PPJOY_OPTIONS);

Exit:
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoyBus_GetOptions",ntStatus);
 return ntStatus;
}

NTSTATUS PPJoyBus_SetOptions (IN PDEVICE_OBJECT BusDeviceObject, IN PPPJOY_OPTIONS Option, OUT PULONG BytesOut)
{
 NTSTATUS		ntStatus;
 WCHAR			*OptionName;

 PAGED_CODE();

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("PPJoyBus_SetOptions (BusDeviceObject=0x%p)",BusDeviceObject) );

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_BABBLE, ("PPJoyBus_SetOptions option 0x%X value 0x%X ",Option->Option,Option->Value) );

 switch (Option->Option)
 {
  case	PPJOY_OPTION_RUNTIMEDEBUG:
		PPJoy_Opt_RuntimeDebug= Option->Value;
		OptionName= PPJOY_OPTNAME_RUNTIMEDEBUG;
		break;

  case	PPJOY_OPTION_PORTFLAGS:
		/* Not updating PPJoy_Opt_PortFlags value - these options are not dynamic */
		/* We run into problems if we change the port alloc policy while ports are allocated */
		/* PPJoy_Opt_PortFlags= Option->Value; */
		OptionName= PPJOY_OPTNAME_PORTFLAGS;
		break;

  default:
		ntStatus= STATUS_NOT_SUPPORTED;
		goto Exit;
 }
 
 ntStatus= RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE,Globals.ParamRegistryPath.Buffer,OptionName,REG_DWORD,&(Option->Value),sizeof(Option->Value));
 if (!NT_SUCCESS (ntStatus)) 
  PPJOY_DBGPRINT (FILE_IOCTL | PPJOY_WARN, ("RtlWriteRegistryValue error 0x%x, writing option to registry",ntStatus) );

 if (PPJoyOptionCallback)
  PPJoyOptionCallback (Option->Option,Option->Value);

Exit:
 *BytesOut= 0;
 PPJOY_EXITPROC (FILE_IOCTL|PPJOY_FEXIT_STATUSOK, "PPJoyBus_GetOptions",ntStatus);
 return ntStatus;
}
