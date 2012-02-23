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

#include <string.h>
#include <stdlib.h>

#include <basetyps.h>
#include <wtypes.h>
#include <winreg.h>
#include <setupapi.h>

#include <devguid.h>

#include "Debug.h"
#include "DeviceSetup.h"
#include <cfgmgr32.h>


HKEY OpenDeviceRegKey (TCHAR *DeviceName)
{
 HDEVINFO			DeviceInfoSet;
 SP_DEVINFO_DATA	DeviceInfoData;
 HKEY				DeviceKey;

 int				Index;
 int				rc;
 TCHAR				Buffer[256*sizeof(TCHAR)];
 DWORD				BufSize;
 DWORD				ValueType;

 // Use GUID_DEVCLASS_PORTS to get all Port (COM and LPT) type devices.
 // GUID_PARALLEL_DEVICE is only defined in the DDK headers. Don't want to
 // include the DDK just for that!
 DeviceInfoSet= SetupDiGetClassDevs ((LPGUID)&GUID_DEVCLASS_PORTS,NULL,NULL,DIGCF_PRESENT);
 
 if(DeviceInfoSet==INVALID_HANDLE_VALUE)
 {
  DebugPrintf ((_T("SetupDiGetClassDevs failed: %d\n"), GetLastError()))
  return (HKEY) INVALID_HANDLE_VALUE;
 }

 Index= 0;
 while (1)
 {
  DeviceKey= (HKEY) INVALID_HANDLE_VALUE;

  /* Initialise buffer so that SetupDiGetDeviceInterfaceDetail will accept it. */
  DeviceInfoData.cbSize= sizeof(SP_DEVINFO_DATA);
  if (!SetupDiEnumDeviceInfo (DeviceInfoSet,Index++,&DeviceInfoData))
  {
   rc= GetLastError();
   if (rc!=ERROR_NO_MORE_ITEMS)
	DebugPrintf ((_T("SetupDiEnumDeviceInfo failed %d\n"),rc))
   break;
  }

  DeviceKey= SetupDiOpenDevRegKey (DeviceInfoSet,&DeviceInfoData,DICS_FLAG_GLOBAL,0,DIREG_DEV,KEY_READ|KEY_WRITE);
  if (DeviceKey==INVALID_HANDLE_VALUE)
  {
   DebugPrintf ((_T("SetupDiOpenDevRegKey failed %d\n"),GetLastError()))
   continue;
  }

  BufSize= sizeof(Buffer);
  rc= RegQueryValueEx (DeviceKey,_T("PortName"),NULL,&ValueType,(LPBYTE)Buffer,&BufSize);
  if (rc!=ERROR_SUCCESS)
  {
   DebugPrintf ((_T("RegQueryValueEx failed %d\n"),GetLastError()))
   continue;
  }
  if (ValueType!=REG_SZ)
  {
   DebugPrintf ((_T("Unexpected type for PortName value (%d)\n"),ValueType))
   continue;
  }

  DebugPrintf ((_T("Found port: %s\n"),Buffer))

  if (!_tcsicmp(Buffer,DeviceName))
   break;

  RegCloseKey (DeviceKey);
 }

 SetupDiDestroyDeviceInfoList (DeviceInfoSet);
 return DeviceKey;
}

int GetDeviceInstanceID (TCHAR *DeviceName, TCHAR *DevInstID, int DevInstIDSize)
{
 HDEVINFO			DeviceInfoSet;
 SP_DEVINFO_DATA	DeviceInfoData;
 HKEY				DeviceKey;

 int				Index;
 int				rc;
 int				result;
 TCHAR				Buffer[256*sizeof(TCHAR)];
 DWORD				BufSize;
 DWORD				ValueType;

 // Use GUID_DEVCLASS_PORTS to get all Port (COM and LPT) type devices.
 // GUID_PARALLEL_DEVICE is only defined in the DDK headers. Don't want to
 // include the DDK just for that!
 DeviceInfoSet= SetupDiGetClassDevs ((LPGUID)&GUID_DEVCLASS_PORTS,NULL,NULL,DIGCF_PRESENT);
 
 if(DeviceInfoSet==INVALID_HANDLE_VALUE)
 {
  DebugPrintf ((_T("SetupDiGetClassDevs failed: %d\n"), GetLastError()))
  return 0;
 }

 result= 0;

 Index= 0;
 while (1)
 {
  DeviceKey= (HKEY) INVALID_HANDLE_VALUE;

  /* Initialise buffer so that SetupDiGetDeviceInterfaceDetail will accept it. */
  DeviceInfoData.cbSize= sizeof(SP_DEVINFO_DATA);
  if (!SetupDiEnumDeviceInfo (DeviceInfoSet,Index++,&DeviceInfoData))
  {
   rc= GetLastError();
   if (rc!=ERROR_NO_MORE_ITEMS)
	DebugPrintf ((_T("SetupDiEnumDeviceInfo failed %d\n"),rc))
   break;
  }

  DeviceKey= SetupDiOpenDevRegKey (DeviceInfoSet,&DeviceInfoData,DICS_FLAG_GLOBAL,0,DIREG_DEV,KEY_READ|KEY_WRITE);
  if (DeviceKey==INVALID_HANDLE_VALUE)
  {
   DebugPrintf ((_T("SetupDiOpenDevRegKey failed %d\n"),GetLastError()))
   continue;
  }

  BufSize= sizeof(Buffer);
  rc= RegQueryValueEx (DeviceKey,_T("PortName"),NULL,&ValueType,(LPBYTE)Buffer,&BufSize);
  if (rc!=ERROR_SUCCESS)
  {
   DebugPrintf ((_T("RegQueryValueEx failed %d\n"),GetLastError()))
   continue;
  }
  if (ValueType!=REG_SZ)
  {
   DebugPrintf ((_T("Unexpected type for PortName value (%d)\n"),ValueType))
   continue;
  }

  RegCloseKey (DeviceKey);

  DebugPrintf ((_T("Found port: %s\n"),Buffer))

  if (_tcsicmp(Buffer,DeviceName))
   continue;

  rc= CM_Get_Device_ID(DeviceInfoData.DevInst,DevInstID,DevInstIDSize,0);
  if (rc!=CR_SUCCESS)
  {
   DebugPrintf ((_T("CM_Get_Device_ID error %d\n"),rc))
   break;
  }

  result= 1;
  break;
 }

 SetupDiDestroyDeviceInfoList (DeviceInfoSet);
 return result;
}

