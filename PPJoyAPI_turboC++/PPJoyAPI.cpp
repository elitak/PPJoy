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


#include <vcl.h>
#pragma hdrstop
#define Library

#include <windows.h>

#include "PPJoyAPI.h"

#include <JoyBus.h>
#include <JoyDefs.h>
#include <Mapping.h>
#include <ConfigMan.h>

int PPJoy_OpenDriver (void)
{
 return OpenPPJoyBusDriver();
}

void PPJoy_CloseDriver (void)
{
 ClosePPJoyBusDriver();
}

int PPJoy_GetJoystickList (JOYENTRY *JoystickList, int ListSize)
{
 int	Count;

 RescanJoysticks();
 if (!pEnumData)
  return -1;
 
 if (ListSize>(int)pEnumData->Count)
  ListSize= (int)pEnumData->Count;

 for (Count=0;Count<ListSize;Count++)
 {
  JoystickList->JoyType= pEnumData->Joysticks[Count].JoyType;
  JoystickList->JoySubType= pEnumData->Joysticks[Count].JoySubType;
  JoystickList->LPTNumber= pEnumData->Joysticks[Count].LPTNumber;
  JoystickList->UnitNumber= pEnumData->Joysticks[Count].UnitNumber;
  JoystickList++;
 }
 return Count;
}

int PPJoy_AddJoystick (char LPTNumber, char UnitNumber, char JoyType, char JoySubType)
{
 USHORT VendorID;
 USHORT ProductID;
 ULONG	Count;

 RescanJoysticks();
 if (!pEnumData)
  return 0;

 VendorID= 0xDEAD;
 for (ProductID=0xBEF0;ProductID<0xBF00;ProductID++)
 {
  for (Count=0;Count<pEnumData->Count;Count++)
   if ((pEnumData->Joysticks[Count].VendorID==VendorID)&&(pEnumData->Joysticks[Count].ProductID==ProductID))
    break;
  if (Count==pEnumData->Count)
   break;
 }

 if (ProductID>=0xBF00)
  return 0;

 return AddJoystick (LPTNumber,JoyType,UnitNumber,JoySubType,VendorID,ProductID);
}

int PPJoy_DeleteJoystick (char LPTNumber, char UnitNumber, char JoyType)
{
 return DeleteJoystick (LPTNumber,JoyType,UnitNumber);
}

char* PPJoy_GetSubTypeName (char JoyType, char JoySubType)
{
 return GetSubTypeName(JoyType,JoySubType);
}

char* PPJoy_GetJoyTypeName (char JoyType)
{
 return GetJoyTypeName(JoyType);
}

int	PPJoy_ReadMappings (char LPTNumber, char UnitNumber, char JoyType, JOYMAPPING **DefaultMapping, JOYMAPPING **DriverMapping, JOYMAPPING **DeviceMapping)
{
 *DefaultMapping= NULL;
 *DriverMapping= NULL;
 *DeviceMapping= NULL;
 return ReadJoystickMappings (LPTNumber,JoyType,UnitNumber,(JOYSTICK_MAP**)DefaultMapping,(JOYSTICK_MAP**)DriverMapping,(JOYSTICK_MAP**)DeviceMapping);
}

void PPJoy_FreeMapping (JOYMAPPING *Mapping)
{
 if (Mapping)
  free (Mapping);
}

int	PPJoy_WriteMapping (char LPTNumber, char UnitNumber, char JoyType, JOYMAPPING *Mapping, int MapSize, int Scope)
{
 int	rc;
 DWORD	Count;
 char	DeviceID[256];

 rc= WriteJoystickMapping (LPTNumber,JoyType,UnitNumber,(JOYSTICK_MAP*)Mapping,MapSize,Scope);

 RescanJoysticks();
 if (!pEnumData)
  return rc;

 for (Count=0;Count<pEnumData->Count;Count++)
  if (((JoyType==pEnumData->Joysticks[Count].JoyType)&&(Scope==MAP_SCOPE_DRIVER))||
      (((DWORD)LPTNumber==pEnumData->Joysticks[Count].LPTNumber)&&(UnitNumber==pEnumData->Joysticks[Count].UnitNumber)))
  {
   MakeDeviceID (DeviceID,pEnumData->Joysticks[Count].VendorID,pEnumData->Joysticks[Count].ProductID);
   RestartDevice (DeviceID);
  }

 return rc;
}



int PPJoy_DeleteMapping (char LPTNumber, char UnitNumber, char JoyType, int Scope)
{
 return DeleteJoystickMapping (LPTNumber,JoyType,UnitNumber,Scope);
}
