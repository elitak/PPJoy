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

#define	DIRECTINPUT_VERSION	0x0700

#include "Debug.h"
#include "DXUtil.h"

#include <objbase.h>
#include <initguid.h>
#include <dinput.h>
#include <dinputd.h>

class DIRECTINPUT
{
 public:
  LPDIRECTINPUT				IDirectInput;
  LPDIRECTINPUTJOYCONFIG	IDirectInputJoyConfig;

  DIRECTINPUT (void);
  ~DIRECTINPUT (void);
  int AcquireConfig (void);
  void UnacquireConfig (void);
  int GetJoystickIndex (int VendorID, int ProductID);
  int DeleteConfig (int JoystickIndex);
  int DeleteType (int VendorID, int ProductID);
};

DIRECTINPUT::DIRECTINPUT (void)
{
 HRESULT	hr;

 IDirectInput= NULL;
 IDirectInputJoyConfig= NULL;
	
 hr= DirectInputCreate(AfxGetInstanceHandle(),DIRECTINPUT_VERSION,&IDirectInput,NULL);
 if (FAILED(hr))
 {
  DebugPrintf ((_T("Error 0x%X creating IDirectInput inferface.\n"),hr))
  IDirectInput= NULL;
  return;
 }
 
 hr= IDirectInput->QueryInterface(IID_IDirectInputJoyConfig,(LPVOID*)&IDirectInputJoyConfig);
 if (FAILED(hr))
 {
  DebugPrintf ((_T("Error 0x%X creating IDirectInputJoyConfig inferface.\n"),hr))

  IDirectInput->Release();
  IDirectInputJoyConfig= NULL;
  IDirectInput= NULL;
  return;
 }
}

DIRECTINPUT::~DIRECTINPUT (void)
{
 if (IDirectInputJoyConfig)
  IDirectInputJoyConfig->Release();

 if (IDirectInput)
  IDirectInput->Release();

 IDirectInputJoyConfig= NULL;
 IDirectInput= NULL;
}

int DIRECTINPUT::AcquireConfig (void)
{
 HRESULT		hr;

 hr= IDirectInputJoyConfig->SetCooperativeLevel(AfxGetMainWnd()->m_hWnd,DISCL_EXCLUSIVE|DISCL_BACKGROUND);
 if (FAILED(hr))
 {
  DebugPrintf ((_T("Error 0x%X calling IDirectInputJoyConfig::SetCooperativeLevel.\n"),hr))
  return 0;
 } 

 hr= IDirectInputJoyConfig->Acquire();
 if (FAILED(hr))
 {
  DebugPrintf ((_T("Error 0x%X calling IDirectInputJoyConfig::Acquire.\n"),hr))
  return 0;
 } 

 return 1;
}

void DIRECTINPUT::UnacquireConfig (void)
{
 HRESULT		hr;

 hr= IDirectInputJoyConfig->SendNotify();
 if (FAILED(hr))
  DebugPrintf ((_T("SendNotify error 0x%X\n"),hr))

 hr= IDirectInputJoyConfig->Unacquire();
 if (FAILED(hr))
  DebugPrintf ((_T("Error 0x%X calling IDirectInputJoyConfig::Unacquire.\n"),hr))
}


int DIRECTINPUT::GetJoystickIndex (int VendorID, int ProductID)
{
 HRESULT		hr;
 int			Count;
 DIJOYCONFIG	JoyConfig;
 WCHAR			TypeName[MAX_JOYSTRING];

 swprintf (TypeName,L"VID_%04X&PID_%04X",VendorID,ProductID);	// Apply branding here?

 for (Count= 0;Count<16;Count++)
 {
  JoyConfig.dwSize= sizeof(JoyConfig);
  hr= IDirectInputJoyConfig->GetConfig(Count,&JoyConfig,DIJC_REGHWCONFIGTYPE);

  if (hr==DIERR_NOMOREITEMS)
  {
   DebugPrintf ((_T("Joystick with VendorID %04X and ProductID %04X not found (DIERR_NOMOREITEMS).\n"),VendorID,ProductID))
   return -1;
  }

  if (FAILED(hr))
  {
   DebugPrintf ((_T("GetConfig error 0x%X for uiJoy=%d\n"),hr,Count))
   //break;
   continue;
  }

  if (!wcscmp(TypeName,JoyConfig.wszType))
   return Count;
 }
 DebugPrintf ((_T("Joystick with VendorID %04X and ProductID %04X not found (for loop exit).\n"),VendorID,ProductID))
 return -1;
}

int DIRECTINPUT::DeleteConfig (int JoystickIndex)
{
 HRESULT		hr;

 DebugPrintf ((_T("Attemping to delete joystick configuration for uiJoy=%d\n"),JoystickIndex))
 if (JoystickIndex<0)
  return JoystickIndex;

 if (!AcquireConfig())
  return -1;

 hr= IDirectInputJoyConfig->DeleteConfig(JoystickIndex);
 if (FAILED(hr))
  DebugPrintf ((_T("DeleteConfig error 0x%X for uiJoy=%d\n"),hr,JoystickIndex))

 UnacquireConfig();
 DebugPrintf ((_T("Deleted joystick configuration for uiJoy=%d\n"),JoystickIndex))
 return 0;
}

int DIRECTINPUT::DeleteType (int VendorID, int ProductID)
{
 HRESULT		hr;
 WCHAR			TypeName[MAX_JOYSTRING];

 swprintf (TypeName,L"VID_%04X&PID_%04X",VendorID,ProductID);	// Apply branding here?


 DebugPrintf ((_T("Attemping to delete joystick type %lS\n"),TypeName))

 if (!AcquireConfig())
  return -1;

 hr= IDirectInputJoyConfig->DeleteType(TypeName);
 if (FAILED(hr))
  DebugPrintf ((_T("DeleteType error 0x%X for type %lS\n"),hr,TypeName))

 UnacquireConfig();
 DebugPrintf ((_T("Deleted joystick type %lS\n"),TypeName))
 return 0;
}

int DXDeleteJoystick (unsigned int VendorID, unsigned int ProductID)
{
 DIRECTINPUT	di;
 int			result;
 
 DebugPrintf ((_T("Attemping to delete joystick configuration for VID=%04X PID= %04X\n"),VendorID,ProductID))
 result= di.DeleteConfig(di.GetJoystickIndex(VendorID,ProductID));
 if (result==0)
  result= di.DeleteType(VendorID,ProductID);

 return result;
}

