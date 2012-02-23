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
/**  Power.c    Handle the Power IRPs in this module. We will simply pass **/
/**             them to the PDO (Bus) driver to handle.                   **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "PPortJoy.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE, PPJoy_Power)
    #pragma alloc_text (PAGE, PPJoy_Ctl_Power)
#endif

/**************************************************************************/
/* Routine to handle Power IRPs. We simply pass them down the stack       */
/**************************************************************************/
NTSTATUS PPJoy_Power (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 PDEVICE_EXTENSION  DeviceExtension;
 NTSTATUS           ntStatus;
 POWER_STATE		PowerState;
 POWER_STATE_TYPE	PowerType;
 PIO_STACK_LOCATION	Stack;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_POWER | PPJOY_FENTRY, ("Enter PPJoy_Power(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

 ntStatus= PPJoy_EnterRequest (DeviceExtension);
 if (NT_SUCCESS(ntStatus))
 {
  Stack= IoGetCurrentIrpStackLocation (Irp);
  PowerType= Stack->Parameters.Power.Type;
  PowerState= Stack->Parameters.Power.State;

  if ((Stack->MinorFunction==IRP_MN_SET_POWER)&&
	  (PowerType==DevicePowerState)&&
	  (PowerState.DeviceState==PowerDeviceD0))
  {
   PPJOY_DBGPRINT (FILE_POWER|PPJOY_BABBLE, ("We got a device D0 power state request for our device. Set parallel port mode again") );

   /* Set the parallel port mode. */
   PPJoy_InitPortAndInterface (DeviceObject);
  }
 
  /* Must be called before IoSkipCurrentIrpStackLocation, else bugcheck */
  PoStartNextPowerIrp (Irp);
  /* Prepare IRP stack for next driver */
  IoSkipCurrentIrpStackLocation (Irp);

  /* We call PoCallDriver since this is a Power IRP */
  ntStatus=  PoCallDriver (GET_NEXT_DEVICE_OBJECT (DeviceObject),Irp);
 }
 else
 {
  /* Must be called even when the device is removed */
  PoStartNextPowerIrp (Irp);

  Irp->IoStatus.Information= 0;
  Irp->IoStatus.Status= ntStatus;
  IoCompleteRequest (Irp,IO_NO_INCREMENT);
 }

 PPJoy_LeaveRequest	(DeviceExtension);

 PPJOY_EXITPROC (FILE_POWER|PPJOY_FEXIT, "PPJOY_Power",ntStatus);
 return ntStatus;
}

/**************************************************************************/
/* Routine to handle Power IRPs for control devices. We are rock bottom   */
/* so we cannot simply pass the buck.
/**************************************************************************/
NTSTATUS PPJoy_Ctl_Power (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS           ntStatus;
 POWER_STATE		PowerState;
 POWER_STATE_TYPE	PowerType;
 PIO_STACK_LOCATION	Stack;
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE ();

 /* If it is not for one of our control devices - pass on to HID.sys */
 if (GET_NEXT_DEVICE_OBJECT(DeviceObject))
 {
  PPJOY_DBGPRINT (FILE_POWER|PPJOY_HIDHACK, ("Not a control device - passing request on to HID.sys (PPJoy_Ctl_Power)") );
  return HIDMajorFunctions[IoGetCurrentIrpStackLocation(Irp)->MajorFunction](DeviceObject,Irp);
 }

 PPJOY_DBGPRINT (FILE_POWER | PPJOY_FENTRY, ("Enter PPJoy_Ctl_Power(DeviceObject=0x%p,Irp=0x%p)",DeviceObject,Irp) );

 DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

 ntStatus= PPJoy_EnterRequest (DeviceExtension);
 if (NT_SUCCESS(ntStatus))
 {
  Stack= IoGetCurrentIrpStackLocation (Irp);
  PowerType= Stack->Parameters.Power.Type;
  PowerState= Stack->Parameters.Power.State;
  PPJOY_DBGPRINT (FILE_POWER|PPJOY_BABBLE, ("Received Power IRP %s",PowerMinorFunctionString(Stack->MinorFunction)) );

  switch (Stack->MinorFunction)
  {
   case IRP_MN_SET_POWER:
		PPJOY_DBGPRINT (FILE_POWER|PPJOY_BABBLE, ("Request to set %s state to %s",					\
					((PowerType==SystemPowerState)?"System":"Device"),								\
                     ((PowerType==SystemPowerState)?DbgSystemPowerString(PowerState.SystemState):	\
					 DbgDevicePowerString(PowerState.DeviceState))) );

		if ((PowerType==DevicePowerState)||(PowerType==SystemPowerState))
         ntStatus= STATUS_SUCCESS;
		break;

   case IRP_MN_QUERY_POWER:
		ntStatus= STATUS_SUCCESS;
		break;

   case IRP_MN_WAIT_WAKE:
   case IRP_MN_POWER_SEQUENCE:
   default:
		ntStatus= STATUS_NOT_SUPPORTED;
        break;
  }
 }

 /* Must be before IoCompleteRequest(), else bugcheck! */
 PoStartNextPowerIrp (Irp);

 /* We are the lowest level driver for this device, complete IRP */
 if (ntStatus!=STATUS_NOT_SUPPORTED)
  Irp->IoStatus.Status= ntStatus;
 IoCompleteRequest (Irp,IO_NO_INCREMENT);
 
 PPJoy_LeaveRequest	(DeviceExtension);

 PPJOY_EXITPROC (FILE_POWER|PPJOY_FEXIT, "PPJOY_Ctl_Power",ntStatus);
 return ntStatus;
}



#if DBG
/**************************************************************************/
/* Routines to print the symbolic names of Power IRP constants.           */
/**************************************************************************/
PCHAR PowerMinorFunctionString (UCHAR MinorFunction)
{
 switch (MinorFunction)
 {
  case IRP_MN_SET_POWER:
		return "IRP_MN_SET_POWER";
  case IRP_MN_QUERY_POWER:
		return "IRP_MN_QUERY_POWER";
  case IRP_MN_POWER_SEQUENCE:
		return "IRP_MN_POWER_SEQUENCE";
  case IRP_MN_WAIT_WAKE:
		return "IRP_MN_WAIT_WAKE";
            
  default:
	return "IRP_MN_?????";
 }
}

PCHAR DbgSystemPowerString (IN SYSTEM_POWER_STATE Type)
{  
 switch (Type)
 {
  case PowerSystemUnspecified:
			return "PowerSystemUnspecified";
  case PowerSystemWorking:
			return "PowerSystemWorking";
  case PowerSystemSleeping1:
			return "PowerSystemSleeping1";
  case PowerSystemSleeping2:
			return "PowerSystemSleeping2";
  case PowerSystemSleeping3:
			return "PowerSystemSleeping3";
  case PowerSystemHibernate:
			return "PowerSystemHibernate";
  case PowerSystemShutdown:
			return "PowerSystemShutdown";
  case PowerSystemMaximum:
			return "PowerSystemMaximum";

  default:
			return "UnKnown System Power State";
 }
}

PCHAR DbgDevicePowerString (IN DEVICE_POWER_STATE Type)
{
 switch (Type)
 {
  case PowerDeviceUnspecified:
			return "PowerDeviceUnspecified";
  case PowerDeviceD0:
			return "PowerDeviceD0";
  case PowerDeviceD1:
			return "PowerDeviceD1";
  case PowerDeviceD2:
			return "PowerDeviceD2";
  case PowerDeviceD3:
			return "PowerDeviceD3";
  case PowerDeviceMaximum:
			return "PowerDeviceMaximum";

  default:
			return "UnKnown Device Power State";
 }
}

#endif
