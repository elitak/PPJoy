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

#include "Debug.h"

#include "DirectInput.h"

#define	DIRECTINPUT_VERSION	0x0700
#include <initguid.h>
#include <dinput.h>
#include <dinputd.h>

/* Entry point for function to delete joystick type definitions from  */
/* DirectInput. Returns 0 on failure or 1 on success.                 */
int DeleteJoystickType (const char *TypeName)
{
 LPDIRECTINPUT				IDirectInput;
 LPDIRECTINPUTJOYCONFIG		IDirectInputJoyConfig;
 HRESULT					hr;
 int						Count;
 DIJOYCONFIG				JoyConfig;
 WCHAR						wTypeName[MAX_JOYSTRING];

 CFrameWnd					DummyWindow;
 
 DummyWindow.Create(NULL,"DummyWindow",0,CFrameWnd::rectDefault ,NULL,NULL,0,NULL);

 mbstowcs (wTypeName,TypeName,MAX_JOYSTRING);
 wTypeName[MAX_JOYSTRING-1]= 0;

 hr= DirectInputCreate(GetModuleHandle(NULL),DIRECTINPUT_VERSION,&IDirectInput,NULL);
 if (FAILED(hr))
 {
  DebugPrintf (("Error 0x%X creating IDirectInput inferface.\n",hr))
  return 0;
 }
 
 DebugPrintf (("IDirectInput inferface created successfully\n"))

 hr= IDirectInput->QueryInterface(IID_IDirectInputJoyConfig,(LPVOID*)&IDirectInputJoyConfig);
 if (FAILED(hr))
 {
  DebugPrintf (("Error 0x%X creating IDirectInputJoyConfig inferface.\n",hr))
  IDirectInput->Release();
  return 0;
 }

 DebugPrintf (("IDirectInputJoyConfig inferface created successfully\n"))
 DebugPrintf (("Window handle is 0x%08X\n",DummyWindow.m_hWnd))

 hr= IDirectInputJoyConfig->SetCooperativeLevel(DummyWindow.m_hWnd,DISCL_EXCLUSIVE|DISCL_BACKGROUND);
 if (FAILED(hr))
 {
  DebugPrintf (("Error 0x%X calling IDirectInputJoyConfig::SetCooperativeLevel.\n",hr))
  IDirectInputJoyConfig->Release();
  IDirectInput->Release();
  return 0;
 } 

 hr= IDirectInputJoyConfig->Acquire();
 if (FAILED(hr))
 {
  DebugPrintf (("Error 0x%X calling IDirectInputJoyConfig::Acquire.\n",hr))
  IDirectInputJoyConfig->Release();
  IDirectInput->Release();
  return 0;
 } 

 DebugPrintf (("IDirectInputJoyConfig acquired\n"))

 for (Count= 0;Count<64;Count++)
 {
  JoyConfig.dwSize= sizeof(JoyConfig);
  hr= IDirectInputJoyConfig->GetConfig(Count,&JoyConfig,DIJC_REGHWCONFIGTYPE);

  if (hr==DIERR_NOMOREITEMS)
   break;

  if (FAILED(hr))
  {
   DebugPrintf (("GetConfig error 0x%X for uiJoy=%d\n",hr,Count))
   continue;
  }

  if (!wcscmp(wTypeName,JoyConfig.wszType))
  {
   DebugPrintf (("Deleting joystick config with index %d\n",Count))
   hr= IDirectInputJoyConfig->DeleteConfig(Count);
   if (FAILED(hr))
    DebugPrintf (("DeleteConfig error 0x%X for uiJoy=%d\n",hr,Count))
  }
 }

 hr= IDirectInputJoyConfig->DeleteType(wTypeName);
 if (FAILED(hr))
 {
  DebugPrintf (("DeleteType error 0x%X for Type=%S\n",hr,wTypeName))
 }

 hr= IDirectInputJoyConfig->SendNotify();
 if (FAILED(hr))
 {
  DebugPrintf (("SendNotify error 0x%X\n",hr))
 }

 hr= IDirectInputJoyConfig->Unacquire();
 if (FAILED(hr))
 {
  DebugPrintf (("Error 0x%X calling IDirectInputJoyConfig::Unacquire.\n",hr))
 } 

 IDirectInputJoyConfig->Release();
 IDirectInput->Release();
 return 1;
}
