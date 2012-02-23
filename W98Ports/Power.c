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
/**  Power.c    Handler for Power Management IRPs.                        **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "W98Ports.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE, W98Ports_Power)
#endif

/**************************************************************************/
/* Routine to handle Power IRP requests. Simply succeed everything.       */
/**************************************************************************/
NTSTATUS W98Ports_DoPower (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 NTSTATUS			ntStatus;
 POWER_STATE		PowerState;
 POWER_STATE_TYPE	PowerType;
 PIO_STACK_LOCATION	Stack;
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_POWER|PPJOY_FENTRY, ("Enter W98Ports_Power_Bus (DeviceObject=0x%x,Irp=0x%x)",DeviceObject,Irp) );

 Stack= IoGetCurrentIrpStackLocation (Irp);
 PowerType= Stack->Parameters.Power.Type;
 PowerState= Stack->Parameters.Power.State;
 PPJOY_DBGPRINT (FILE_POWER|PPJOY_BABBLE, ("Received Power IRP %s",PowerMinorFunctionString(Stack->MinorFunction)) );
 if (Stack->MinorFunction==IRP_MN_SET_POWER)
 {
  PPJOY_DBGPRINT (FILE_POWER|PPJOY_BABBLE, ("Request to set %s state to %s",						\
					((PowerType==SystemPowerState)?"System":"Device"),								\
                     ((PowerType==SystemPowerState)?DbgSystemPowerString(PowerState.SystemState):	\
					 DbgDevicePowerString(PowerState.DeviceState))) );
 }

 /* Must be before IoSkipCurrentIrpStackLocation(), else bugcheck! */
 PoStartNextPowerIrp (Irp);

 /* Pass power IRP on to the next lower driver in the chain. */
 IoSkipCurrentIrpStackLocation (Irp);

 /* Get a pointer to device data */
 DeviceExtension= (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
 ntStatus =  PoCallDriver (DeviceExtension->NextLowerDriver,Irp);

 PPJOY_EXITPROC (FILE_POWER|PPJOY_FEXIT, "W98Ports_Power_Bus",ntStatus );
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
