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


#include "stdafx.h"

#include "DriverSigning.h"
#include "Testsign.h"
#include "PPJoyBus50.h"
#include "DirectInput.h"
#include "ConfigMan.h"
#include "JoyBus.h"

#include "Debug.h"

// For Nullsoft installer plugins - include plugin functions and defines.
#include <pluginapi.c>
// Wrapper to declare a parameter passed on the nsis plugin stack. Pass the
// variable name to declare as parameter. All vars will be 1024 characters,
// same as nsis default. Param is popped during declaration!!!
#define	DECLARE_CHAR_PARAM(PARNAME)	char PARNAME[1024]; if (popstringn(PARNAME,sizeof(PARNAME))) *PARNAME=0;
#define	DECLARE_INT_PARAM(PARNAME)	int PARNAME= popint();
#define	SAVE_INT_RESULT(RESULTEXPR)	{ char TempResultStr[33]; setuservariable (INST_0,_itoa((RESULTEXPR),TempResultStr,10)); }
#define	SAVE_CHAR_RESULT(RESULTEXPR)	{ setuservariable (INST_0,(RESULTEXPR)); }
#define	DECLARE_NSIS_FUNC(NSISFUNC)	void __cdecl NSISFUNC (HWND hwndParent, int string_size, char *variables, stack_t **stacktop, void *extra)


// If current install is <= PPJoy v0.50, delete all the configured joysticks
// and the joystick drivers.
int DeletePPJoyDevices50 (void)
{
 HANDLE					PPJoyDriver;
 PJOYSTICK_ENUM_DATA50	EnumData;
 unsigned int			Count;
 char					DeviceID[128];

 PPJoyDriver= OpenPPJoyBusDriver50();
 if (!PPJoyDriver)
  return 0;

 EnumJoystick50 (PPJoyDriver,&EnumData);
 if (!EnumData)
 {
  CloseHandle (PPJoyDriver);
  return 0;
 }

 /// delete joysticks here
 for (Count=0;Count<EnumData->Count;Count++)
 {
  DebugPrintf (("Found joystick #%d: port %4X typeaslong %4X\n",Count,EnumData->Joys[Count].Port,EnumData->Joys[Count].TypeAsLong))

  // Delete device nodes before actually removing hardware. Else we cannot delete the dependent HID driver node
  sprintf (DeviceID,"PPJOYBUS\\VID_DEAD&PID_BEEF\\PPJoy%08X%08X",EnumData->Joys[Count].Port,EnumData->Joys[Count].SubAddr);
  DeleteDeviceID (DeviceID,1);

#if 0	/* We don't want to migrate old config to new post v0.50 install */
  if (Count<16)
   AddJoystickToRegNew (0xDEAD,0xBEF0+Count,EnumData->Joys[Count].Port,Port,EnumData->Joys[Count].JoyType,Port,EnumData->Joys[Count].SubAddr,Port,EnumData->Joys[Count].NumButtons);
#endif

  DelJoystick50 (PPJoyDriver,EnumData->Joys[Count].Port,EnumData->Joys[Count].TypeAsLong);
 }

 CloseHandle (PPJoyDriver);

#if 1	/* Do we delete the old bus driver? Hmm... yes lets do it */
 if (FindDeviceID ("ROOT\\PPJOYBUS",DeviceID,sizeof(DeviceID)))
  DeleteDeviceID (DeviceID,1);
 else
 {
  DebugPrintf (("Cannot find DeviceID for PPJoyBus driver...\n"))
 }
#endif

 free (EnumData);
 return 1;
}

// Return the number of installed parallel ports (starting from LPT1 until we find a gap)
int CountParallelPorts (void)
{
 ULONG	PortArray[16];
 int	Count;

 if (!RescanParallelPorts(PortArray,sizeof(PortArray)/sizeof(*PortArray)))
  return -1;

 PortArray[15]= 0;
 Count= 0;
 while (PortArray[Count])
  Count++;

 return Count;
}

// Return number of joysticks configured in PPJoy.
int CountInstalledPPJoysticks (void)
{
 int	Count;

 if (!OpenPPJoyBusDriver())
  return -1;

 Count= -1;
 if (RescanJoysticks())
  Count= pEnumData->Count; 
	 
 ClosePPJoyBusDriver();
 return Count;
}

int AddNewJoystick (int LPTNumber, int JoyType, int UnitNumber, int JoySubType, int VendorID, int ProductID)
{
 int	rc;

 if (!OpenPPJoyBusDriver())
  return -1;

 rc= AddJoystick (LPTNumber,JoyType,UnitNumber,JoySubType,VendorID,ProductID);
	 
 ClosePPJoyBusDriver();
 return rc;
}

/* Entry point for utility functions. Will return 0 for unknown       */
/* functions, else whatever the called function returned.             */

/* DON'T RETURN -1! This causes Installer Vise to terminate! */
LONG CALLBACK ViseEntry(LONG lParam1, LPCSTR lpParam2, DWORD dwReserved)
{
 AFX_MANAGE_STATE(AfxGetStaticModuleState());
		
 int	rc;

 DebugPrintf (("ViseEntry called, lParam1= 0x%X, lpParam= %s\n",lParam1,lpParam2?lpParam2:"(null)"))

 switch (lParam1)
 {
  case 1:	rc= CheckSigningStatus();
			break;

  case 2:	rc= DeleteJoystickType(lpParam2);
			break;

  case 3:	rc= DeletePPJoyDevices50();
			break;

  case 4:	rc= CountParallelPorts();
			break;

  case 5:	rc= CountInstalledPPJoysticks();
			break;

  case 6:	rc= GetInterruptAssignment((char*)lpParam2);
			break;
	 
  case 7:	rc= GetConnectInterruptSetting((char*)lpParam2);
			break;

  case 8:	rc= SetConnectInterruptSetting((char*)lpParam2,1);
			break;

  case 9:	{
			 int	LPTNumber;
			 int	JoyType;
			 int	UnitNumber;
			 int	JoySubType;
			 int	VendorID;
			 int	ProductID;

			 rc= -1;
			 if (sscanf((char*)lpParam2,"LPT%d,%d,%d,%d,%X,%X",&LPTNumber,&JoyType,&UnitNumber,&JoySubType,&VendorID,&ProductID)==6)
			  rc= AddNewJoystick(LPTNumber,JoyType,UnitNumber,JoySubType,VendorID,ProductID);
			}
			break;

  case 10:	rc= GetRegistryTestsigningFlag();
			break;

  case 11:	rc= GetBCDTestsigningFlag();
			break;

  case 12:	rc= SetBCDTestsigningFlag (atoi(lpParam2));
			break;

  case 13:	rc= RebootSystem();
			break;

  default:	rc= -9999;
			DebugPrintf (("Unknown helper function request\n"))
			break;
 }

 DebugPrintf (("Return code is %d\n",rc))

 /* DON'T RETURN -1! This causes Installer Vise to terminate! */
 /* We will politely translater all -1 return codes to -1000 */
 if (rc==-1)
  rc=-1000;

 return rc;
}


// note if you want parameters from the stack, pop them off in order.
// i.e. if you are called via exdll::myFunction file.dat poop.dat
// calling popstring() the first time would give you file.dat,
// and the second time would give you poop.dat. 
// you should empty the stack of your parameters, and ONLY your
// parameters.


/******** NSIS plugin functions *************

- ViseHelper::nsis_CheckSigningStatus

  Checks whether unsigned drivers may be installed on the system by reading
  the appropriate registy keys.

  Return value in $0
   1: Unsigned drivers allowed
   2: Unsigned drivers not allowed

- ViseHelper::nsis_DeleteJoystickType <JoystickType>

  Delete the DirectX configuration for a joystick type. Function returns
  success even if no matching function is found to delete.

  Return value in $0
   0: Failure
   1: Success

- ViseHelper::nsis_DeletePPJoyDevices50

  Delete any PPJoy devices created under PPJoy 0.50. Also deletes the PPJoy
  0.50 Bus Driver if was installed

  Return value in $0
   0: PPJoy 0.50 not installed/running
   1: PPJoy 0.50 devices deleted

- ViseHelper::nsis_CountParallelPorts

  Counts the number of installed parallel ports

  Return value in $0 is the number of installed parallel ports


- ViseHelper::nsis_CountInstalledPPJoysticks

  Counter the number of PPJoy joysticks currently defined

  Return value in $0
   -1: Error opening PPJoy driver (driver not running)
   0-n: Number of joysticks currently installed


- ViseHelper::nsis_GetInterruptAssignment <LPTPortName>

  Get the interrupt assigned to a specific LPT port.

  Return value in $0:
   -2: Cannot find the specified LPT port
   -5: No interrupt assigned to this LPT port
   other <0: error
   >=0: IRQ number assigned
	 
- ViseHelper::nsis_GetConnectInterruptSetting <LPTPortName>

  Get setting that specifies whether hardware IRQs are propagated from the
  Parallel Port driver to other drivers that request the interrupt.

  Return value in $0:
   -1: Cannot find the port or error reading setting
   0: Not enabled
   1: Enabled


- ViseHelper::nsis_SetConnectInterruptSetting <LPTPortName> <ConnectValue>

  Sets registery key to enable/disable other drivers to get LPT IRQ notifitions
  from the Parallel Port driver. Set <ConnectValue> to 1 to enable and 0 to
  disable.

  Return value in $0:
   -1: Cannot find the port or opening registry key
   0: Error writing new value to the registry
   1: New value successfully written to the registry

- ViseHelper::nsis_AddNewJoystick <LPTNumber> <JoyType> <UnitNumber> <JoySubType> <VendorID> <ProductID>

  Add a new joystick to the PPJoy bus driver. It is the callers responsibility
  to make sure the parameters are valid and makes sense.

    <LPTNumber>		Number of LPT port to use (1-x), 0 for virtual
	<JoyType>		Joystick type according to PPJoy enums
	<UnitNumber>	Unit number on the specific LPT port. 0 based.
	<JoySubType>	Sub type for joysticks with more than one model
	<VendorID>		PnP vendor ID for the new device
	<ProductID>		PnP product ID for the new device

  Return value in $0:
   -1: Error opening bus driver (not running)
   0: Error sending IOCTL request to bus driver
   1: Success

- ViseHelper::nsis_GetRegistryTestsigningFlag

  Check the registry to see if the currently running OS image has test signed
  drivers enabled (runtime value of the flag).

  Return value in $0:
   <0: Error reading the required registry key
   0:  Test signed drivers disabled 
   1:  Test signed drivers can be loaded

- ViseHelper::nsis_GetBCDTestsigningFlag

  Check the Boot Configuration Data store to see if the currently running OS
  image has test signed drivers enabled (as a boot-time configuration option).

  Return value in $0:
   <0: Error reading the BDC data store or flag not found
   0:  Flag found and test signing disabled 
   1:  Test signing enabled

- ViseHelper::nsis_SetBCDTestsigningFlag <NewTestsignFlag>

  Sets the Test Signing allowed flag in the BCD for the currently running OS
  image. Setting takes effect on next reboot.

    <LPTNumber>		New value for the Allow Test Signed Drivers flag, 0 or 1

  Return value in $0:
   -1: Error setting the BCD test signing flag
   0: (should not be returned)
   1: Success

- ViseHelper::nsis_RebootSystem

  Initiates a system reboot. (With reason code as planned software installation

  Return value in $0:
   0:  Failure
   1:  Success

 ********************************************/

DECLARE_NSIS_FUNC (nsis_CheckSigningStatus)
{
 EXDLL_INIT();
 SAVE_INT_RESULT(CheckSigningStatus());
}

DECLARE_NSIS_FUNC (nsis_DeleteJoystickType)
{
 EXDLL_INIT();

 DECLARE_CHAR_PARAM(JoystickType)
 SAVE_INT_RESULT(DeleteJoystickType(JoystickType));
}

DECLARE_NSIS_FUNC (nsis_DeletePPJoyDevices50)
{
 EXDLL_INIT();
 SAVE_INT_RESULT(DeletePPJoyDevices50());
}

DECLARE_NSIS_FUNC (nsis_CountParallelPorts)
{
 EXDLL_INIT();
 SAVE_INT_RESULT(CountParallelPorts());
}

DECLARE_NSIS_FUNC (nsis_CountInstalledPPJoysticks)
{
 EXDLL_INIT();
 SAVE_INT_RESULT(CountInstalledPPJoysticks());
}

DECLARE_NSIS_FUNC (nsis_GetInterruptAssignment)
{
 EXDLL_INIT();

 DECLARE_CHAR_PARAM(LPTName)
 SAVE_INT_RESULT(GetInterruptAssignment(LPTName));
}
	 
DECLARE_NSIS_FUNC (nsis_GetConnectInterruptSetting)
{
 EXDLL_INIT();

 DECLARE_CHAR_PARAM(LPTName)
 SAVE_INT_RESULT(GetConnectInterruptSetting(LPTName));
}

DECLARE_NSIS_FUNC (nsis_SetConnectInterruptSetting)
{
 EXDLL_INIT();

 DECLARE_CHAR_PARAM(LPTName)
 DECLARE_INT_PARAM(ConnectValue)

 SAVE_INT_RESULT(SetConnectInterruptSetting(LPTName,ConnectValue));
}

DECLARE_NSIS_FUNC (nsis_AddNewJoystick)
{
 EXDLL_INIT();

 DECLARE_INT_PARAM(LPTNumber)
 DECLARE_INT_PARAM(JoyType)
 DECLARE_INT_PARAM(UnitNumber)
 DECLARE_INT_PARAM(JoySubType)
 DECLARE_INT_PARAM(VendorID)
 DECLARE_INT_PARAM(ProductID)

 SAVE_INT_RESULT(AddNewJoystick(LPTNumber,JoyType,UnitNumber,JoySubType,VendorID,ProductID));
}

DECLARE_NSIS_FUNC (nsis_GetRegistryTestsigningFlag)
{
 EXDLL_INIT();
 SAVE_INT_RESULT(GetRegistryTestsigningFlag());
}

DECLARE_NSIS_FUNC (nsis_GetBCDTestsigningFlag)
{
 EXDLL_INIT();
 SAVE_INT_RESULT(GetBCDTestsigningFlag());
}

DECLARE_NSIS_FUNC (nsis_SetBCDTestsigningFlag)
{
 EXDLL_INIT();

 DECLARE_INT_PARAM(NewValue)

 SAVE_INT_RESULT(SetBCDTestsigningFlag(NewValue));
}

DECLARE_NSIS_FUNC (nsis_RebootSystem)
{
 EXDLL_INIT();
 SAVE_INT_RESULT(RebootSystem());
}
