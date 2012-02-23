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

#include <windows.h>
#include <stdio.h>

#include "Debug.h"
#include "Registry.h"

HKEY OpenOEMKey (USHORT VendorID, USHORT ProductID)
{
 DWORD	rc;
 HKEY	RegKey;
 DWORD	CreateFlag;
 TCHAR	KeyName[128*sizeof(TCHAR)];

 /* Open HKLM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_DEAD&PID_BEF0 */
 _stprintf (KeyName,_T("SYSTEM\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_%04X&PID_%04X"),VendorID,ProductID);

 DebugPrintf ((_T("Opening OEM key %s\n"),KeyName))

 rc= RegCreateKeyEx(HKEY_LOCAL_MACHINE,KeyName,0,_T(""),REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&RegKey,&CreateFlag);
// rc= RegOpenKeyEx (HKEY_LOCAL_MACHINE,KeyName,0,KEY_WRITE,&RegKey);
 if (rc)
 {
  DebugPrintf ((_T("RegOpenKeyEx returned %d for key %s\n"),rc,KeyName))
  return NULL;
 }

 return RegKey;
}

int SetJoystickOEMName (TCHAR *OEMName, USHORT VendorID, USHORT ProductID)
{
 DWORD	rc;
 HKEY	RegKey;

 DebugPrintf ((_T("Setting joystick name to %s\n"),OEMName))

 RegKey= OpenOEMKey (VendorID,ProductID);
 if (!RegKey)
 {
  DebugPrintf ((_T("Cannot open registry key\n")))
  return 0;
 }

 rc= RegSetValueEx (RegKey,_T("OEMName"),0,REG_SZ,(BYTE *)OEMName,(_tcslen(OEMName)+1)*sizeof(TCHAR));
 RegCloseKey(RegKey);

 if (rc)
 {
  DebugPrintf ((_T("RegSetValueEx returned %d\n"),rc))
  return 0;
 }

 DebugPrintf ((_T("Done setting joystick name\n")))
 return 1;
}

int DeleteJoystickOEMData (USHORT VendorID, USHORT ProductID)
{
 DWORD	rc;
 HKEY	RegKey;

 DebugPrintf ((_T("Deleting OEMDATA for joystick\n")))

 RegKey= OpenOEMKey (VendorID,ProductID);
 if (!RegKey)
 {
  DebugPrintf ((_T("Cannot open registry key\n")))
  return 0;
 }

 rc= RegDeleteValue (RegKey,_T("OEMData"));
 RegCloseKey(RegKey);

 if (rc)
 {
  DebugPrintf ((_T("RegDeleteValue returned %d\n"),rc))
  return 0;
 }

 DebugPrintf ((_T("Done removng OEMData for joystick\n")))
 return 1;
}

