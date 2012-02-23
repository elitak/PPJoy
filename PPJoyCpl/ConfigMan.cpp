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
#include <setupapi.h>
#include <cfgmgr32.h>

#include <stdio.h>

#include "ConfigMan.h"
#include "DeviceSetup.h"

#include "Debug.h"

#include <branding.h>

#define	MAX_DRIVERS	128

int AddDeviceContext (HDEVINFO DeviceInfoList, DEVINST DevInst, PSP_DEVINFO_DATA DriverContext);
int GetDevices (HDEVINFO DeviceInfoList, DEVINST DevInst, PSP_DEVINFO_DATA DriverContexts, int MaxDrivers, int *NumDrivers);

int AddDeviceContext (HDEVINFO DeviceInfoList, DEVINST DevInst, PSP_DEVINFO_DATA DriverContext)
{
 TCHAR	Buffer[256*sizeof(TCHAR)];
 int	rc;

 rc= CM_Get_Device_ID(DevInst,Buffer,sizeof(Buffer)/sizeof(TCHAR),0);
 if (rc!=CR_SUCCESS)
 {
  DebugPrintf ((_T("Error %d getting DeviceID for DevInst %X"),rc,DevInst))
  return 0;
 }
 
 DebugPrintf ((_T("Found DeviceID %s (%d)\n"),Buffer,DevInst))
 DriverContext->cbSize= sizeof(*DriverContext);
  
 DriverContext->cbSize= sizeof(SP_DEVINFO_DATA);

 if (!SetupDiOpenDeviceInfo(DeviceInfoList,Buffer,NULL,0,DriverContext))
 {
  /* Looks like we can get here in success? [rc= 0 then] */
  rc= GetLastError();
  if (rc)
  {
   DebugPrintf ((_T("SetupDiOpenDeviceInfo error %d\n"),rc))
   return 0;
  }
 }
 return 1;
}

int GetDevices (HDEVINFO DeviceInfoList, DEVINST DevInst, PSP_DEVINFO_DATA DriverContexts, int MaxDrivers, int *NumDrivers)
{
 int		rc;
 DEVINST	ChildDev;
 int		myrc;

 if ((*NumDrivers)>=MaxDrivers)
  return 0;

 myrc= 1;

 if (AddDeviceContext(DeviceInfoList,DevInst,DriverContexts))
 {
  DriverContexts++;
  (*NumDrivers)++;
  myrc= 0;
 }

 rc= CM_Get_Child(&ChildDev,DevInst,0);
 while (rc==CR_SUCCESS)
 {
  rc= GetDevices (DeviceInfoList,ChildDev,DriverContexts,MaxDrivers,NumDrivers);
  if (!rc)
   myrc= 0;
  rc= CM_Get_Sibling(&ChildDev,ChildDev,0);
 }
 if (rc!=CR_NO_SUCH_DEVNODE)
 {
  DebugPrintf ((_T("CM_Get_Child/CM_Get_Sibling error %d\n"),rc))
  myrc= 0;
 }

 return myrc;
}

int DeleteDeviceID (TCHAR *DeviceID, int Recursive)
{
 SP_DEVINFO_DATA	DriverContexts[MAX_DRIVERS];
 HDEVINFO			DeviceInfoList;
 DEVINST			RootDev;
 int				rc;
 int				NumDrivers;

 DeviceInfoList= SetupDiCreateDeviceInfoList(NULL,NULL);
 if (DeviceInfoList==INVALID_HANDLE_VALUE)
 {
  DebugPrintf ((_T("SetupDiCreateDeviceInfoList error %d\n"),GetLastError()))
  return 0;
 }

 rc= CM_Locate_DevNode(&RootDev,DeviceID,CM_LOCATE_DEVNODE_PHANTOM);
 if (rc!=CR_SUCCESS)
 {
  DebugPrintf ((_T("CM_Locate_DevNode failed, error %d\n"),rc))
  return 0;
 }

 if (Recursive)
 {
  NumDrivers= 0;
  rc= GetDevices (DeviceInfoList,RootDev,DriverContexts,MAX_DRIVERS,&NumDrivers);
 }
 else
 {
  NumDrivers= 1;
  rc= AddDeviceContext (DeviceInfoList,RootDev,DriverContexts);
 }

 DebugPrintf ((_T("Found %d drivers\n"),NumDrivers))

 while (NumDrivers--)
 {
  DebugPrintf ((_T("Atempting to remove DevInst %d"),DriverContexts[NumDrivers].DevInst))
  if (!SetupDiCallClassInstaller(DIF_REMOVE,DeviceInfoList,DriverContexts+NumDrivers))
  {
   DebugPrintf ((_T(" SetupDiCallClassInstaller error %d\n"),GetLastError()))
   rc= 0;
  }
  else
   DebugPrintf ((_T(", done\n")))
 }

 SetupDiDestroyDeviceInfoList (DeviceInfoList);
 return rc;
}

int FindDeviceID (TCHAR *HardwareID, TCHAR *DeviceID, int DevIDSize)
{
 HDEVINFO			DeviceInfoSet;
 SP_DEVINFO_DATA	DeviceInfoData;

 DWORD				Index;
 TCHAR				Buffer[4096*sizeof(TCHAR)];

 int				rc;
 TCHAR				*Search;

 *DeviceID= 0;

 /* Are we sure we want to only get present devices here??? */
 /* Currently this is used by the remove device function only... */
 
 /* Create a Device Information Set with all present devices. */
 DeviceInfoSet= SetupDiGetClassDevs(NULL,0,0,DIGCF_ALLCLASSES);
 if (DeviceInfoSet==INVALID_HANDLE_VALUE)
 {
  DebugPrintf ((_T("SetupDiGetClassDevs error %d\n"),GetLastError()))
  return 0;
 }
    
 /* Search through all the devices and look for a hardware ID match */
 Index= 0;
 DeviceInfoData.cbSize= sizeof(DeviceInfoData);
 while (SetupDiEnumDeviceInfo(DeviceInfoSet,Index++,&DeviceInfoData))
 {
  if (SetupDiGetDeviceRegistryProperty(DeviceInfoSet,&DeviceInfoData,SPDRP_DEVICEDESC,NULL,(PBYTE)Buffer,sizeof(Buffer),NULL))
  {
   DebugPrintf ((_T("Checking device %s\n"),Buffer))
  }

  if (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,&DeviceInfoData,SPDRP_HARDWAREID,NULL,(PBYTE)Buffer,sizeof(Buffer),NULL))
  {
   rc= GetLastError();
   if (rc==ERROR_INVALID_DATA) 
    continue;

   DebugPrintf ((_T("SetupDiGetDeviceRegistryProperty error %d\n"),rc))
   continue;
  }
  
  Buffer[(sizeof(Buffer)/sizeof(TCHAR))-1]= 0;			// TODO: Verify this still works in UNICODE
  Buffer[(sizeof(Buffer)/sizeof(TCHAR))-2]= 0;

  Search= Buffer;
  while (*Search)
  {
   if (!_tcsicmp(Search,HardwareID))
   {
    /* Found a match for the hardware ID */
	if (!SetupDiGetDeviceInstanceId(DeviceInfoSet,&DeviceInfoData,DeviceID,DevIDSize,NULL))
	{
     DebugPrintf ((_T("SetupDiGetDeviceInstanceId error %d\n"),GetLastError()))
	 SetupDiDestroyDeviceInfoList(DeviceInfoSet);
     return 0;
	}

	SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    return 1;
   }
   Search+= _tcslen(Search)+1;
  }
 }      

 SetupDiDestroyDeviceInfoList(DeviceInfoSet);
 return 0;
}

int SetFriendlyName (TCHAR *DeviceID, TCHAR *FriendlyName)
{
 SP_DEVINFO_DATA	DriverContext;
 HDEVINFO			DeviceInfoList;
 DEVINST			DevInst;
 int				rc;

 DeviceInfoList= SetupDiCreateDeviceInfoList(NULL,NULL);
 if (DeviceInfoList==INVALID_HANDLE_VALUE)
 {
  DebugPrintf ((_T("SetupDiCreateDeviceInfoList error %d\n"),GetLastError()))
  return 0;
 }

 rc= CM_Locate_DevNode(&DevInst,DeviceID,CM_LOCATE_DEVNODE_PHANTOM);
 if (rc!=CR_SUCCESS)
 {
  DebugPrintf ((_T("CM_Locate_DevNode failed, error %d\n"),rc))
  return 0;
 }

 if (!AddDeviceContext (DeviceInfoList,DevInst,&DriverContext))
 {
  DebugPrintf ((_T("AddDeviceContext() failed\n")))
  return 0;
 }

 // TODO: Check that this function call still works with UNICODE
 if (!SetupDiSetDeviceRegistryProperty(DeviceInfoList,&DriverContext,SPDRP_FRIENDLYNAME,(UCHAR*) FriendlyName,((DWORD)_tcslen(FriendlyName)+1))*sizeof(TCHAR))
 {
  DebugPrintf ((_T("SetupDiSetDeviceRegistryProperty() error %d\n"),GetLastError()))
  return 0;
 }

 SetupDiDestroyDeviceInfoList (DeviceInfoList);
 return 1;
}

void MakeDeviceID (TCHAR *DeviceID, USHORT VendorID, USHORT ProductID)
{
 _stprintf (DeviceID,TJOY_DEVICE_PNP_ID _T("\\") _T(JOY_DEVINSTANCE_NAME),VendorID,ProductID);
}

int RestartDevice (TCHAR *DeviceID)
{
 SP_PROPCHANGE_PARAMS	PropChangeParams;
 SP_DEVINFO_DATA		DriverContext;
 HDEVINFO				DeviceInfoList;
 int					rc;
 DEVINST				DeviceNode;
 TCHAR					DeviceInstanceID[128*sizeof(TCHAR)];

 DeviceInfoList= SetupDiCreateDeviceInfoList(NULL,NULL);
 if (DeviceInfoList==INVALID_HANDLE_VALUE)
 {
  DebugPrintf ((_T("SetupDiCreateDeviceInfoList error %d\n"),GetLastError()))
  return 0;
 }

 rc= CM_Locate_DevNode(&DeviceNode,DeviceID,CM_LOCATE_DEVNODE_PHANTOM);
 if (rc!=CR_SUCCESS)
 {
  DebugPrintf ((_T("CM_Locate_DevNode failed, error %d\n"),rc))
  goto Exit;
 }

 rc= CM_Get_Device_ID(DeviceNode,DeviceInstanceID,sizeof(DeviceInstanceID)/sizeof(TCHAR),0);
 if (rc!=CR_SUCCESS)
 {
  DebugPrintf ((_T("Error %d getting DeviceInstanceID for DeviceNode %X"),rc,DeviceNode))
  goto Exit;
 }
 
 DebugPrintf ((_T("Found DeviceID %s (%d)\n"),DeviceID,DeviceInstanceID))

 DriverContext.cbSize= sizeof(SP_DEVINFO_DATA);
 if (!SetupDiOpenDeviceInfo(DeviceInfoList,DeviceInstanceID,NULL,0,&DriverContext))
 {
  rc= GetLastError();
  DebugPrintf ((_T("SetupDiOpenDeviceInfo error %d\n"),rc))
  goto Exit;
 }

 PropChangeParams.ClassInstallHeader.cbSize= sizeof(SP_CLASSINSTALL_HEADER);
 PropChangeParams.ClassInstallHeader.InstallFunction= DIF_PROPERTYCHANGE;
 PropChangeParams.StateChange= DICS_PROPCHANGE;
 PropChangeParams.Scope= DICS_FLAG_GLOBAL;
 PropChangeParams.HwProfile= 0;

 if (!SetupDiSetClassInstallParams(DeviceInfoList,&DriverContext,&PropChangeParams.ClassInstallHeader,sizeof(PropChangeParams)))
 {
  rc= GetLastError();
  DebugPrintf ((_T(" SetupDiSetClassInstallParams error %d\n"),rc))
  goto Exit;
 }

 if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,DeviceInfoList,&DriverContext))
 {
  rc= GetLastError();
  DebugPrintf ((_T(" SetupDiCallClassInstaller - DIF_PROPERTYCHANGE error %d\n"),rc))
  goto Exit;
 }

 rc= 0;

Exit:
 SetupDiDestroyDeviceInfoList (DeviceInfoList);
 return rc;
}

int GetDeviceIRQ (TCHAR *DeviceID)
{
 int			rc;
 DEVINST		DeviceNode;
 LOG_CONF		LogConf;
 RES_DES		ResDesc;
 int			IRQ;
 IRQ_RESOURCE	ResBuffer;

 rc= CM_Locate_DevNode(&DeviceNode,DeviceID,CM_LOCATE_DEVNODE_PHANTOM);
 if (rc!=CR_SUCCESS)
 {
  DebugPrintf ((_T("CM_Locate_DevNode failed, error %d\n"),rc))
  return -3;
 }
 
 rc= CM_Get_First_Log_Conf(&LogConf,DeviceNode,ALLOC_LOG_CONF);
 if (rc!=CR_SUCCESS)
 {
  DebugPrintf ((_T("CM_Get_First_Log_Conf failed, error %d\n"),rc))
  return -4;
 }

 rc= CM_Get_Next_Res_Des(&ResDesc,LogConf,ResType_IRQ,NULL,0);
 if (rc!=CR_SUCCESS)
 {
  if (rc==CR_NO_MORE_RES_DES)
  {
   CM_Free_Log_Conf_Handle (LogConf);
   return -5;
  }
  DebugPrintf ((_T("CM_Get_Next_Res_Des failed, error %d\n"),rc))
  CM_Free_Log_Conf_Handle (LogConf);
  return -6;
 }
 
 rc= CM_Get_Res_Des_Data(ResDesc,&ResBuffer,sizeof(ResBuffer),0);
 if (rc!=CR_SUCCESS)
 {
  DebugPrintf ((_T("CM_Get_Res_Des_Data failed, error %d\n"),rc))
  CM_Free_Res_Des_Handle (ResDesc);
  CM_Free_Log_Conf_Handle (LogConf);
  return -7;
 }

 IRQ= ResBuffer.IRQ_Header.IRQD_Alloc_Num;

 CM_Free_Res_Des_Handle (ResDesc);
 CM_Free_Log_Conf_Handle (LogConf);
 return IRQ;
}

int GetInterruptAssignment(TCHAR *DeviceName)
{
 TCHAR	DevInstID[128*sizeof(TCHAR)];

 if (!GetDeviceInstanceID (DeviceName,DevInstID,sizeof(DevInstID)/sizeof(TCHAR)))
  return -2;

 return GetDeviceIRQ (DevInstID);
}

int GetConnectInterruptSetting(TCHAR *DeviceName)
{
 DWORD	RegValue;
 DWORD	ValueType;
 DWORD	BufSize;
 HKEY	PortDeviceRegKey;
 int	rc;

 PortDeviceRegKey= OpenDeviceRegKey(DeviceName);

 if (PortDeviceRegKey==INVALID_HANDLE_VALUE)
 {
  DebugPrintf((_T("Cannot open parallel port device registry")))
  return -1;
 }

 BufSize= sizeof(RegValue);
 rc= RegQueryValueEx (PortDeviceRegKey,_T("EnableConnectInterruptIoctl"),NULL,&ValueType,(LPBYTE)&RegValue,&BufSize);
 CloseHandle (PortDeviceRegKey);

 if (rc==2)		// Reg entry not found. Same effect as value being 0
  return 0;

 if (rc!=ERROR_SUCCESS)
 {
  DebugPrintf((_T("Error %d reading EnableConnectInterruptIoctl value"),rc))
  return -1;
 }

 if (ValueType!=REG_DWORD)
 {
  DebugPrintf ((_T("Datatype for EnableConnectInterruptIoctl is not DWORD!!!"),ValueType))
  return -1;
 }

 return RegValue;
}

int SetConnectInterruptSetting(TCHAR *DeviceName, DWORD Connect)
{
 HKEY	PortDeviceRegKey;
 int	rc;

 PortDeviceRegKey= OpenDeviceRegKey(DeviceName);

 if (PortDeviceRegKey==INVALID_HANDLE_VALUE)
 {
  DebugPrintf((_T("Cannot open parallel port device registry")))
  return -1;
 }

 rc= RegSetValueEx (PortDeviceRegKey,_T("EnableConnectInterruptIoctl"),NULL,REG_DWORD,(LPBYTE)&Connect,sizeof(Connect));
 CloseHandle (PortDeviceRegKey);

 return rc==ERROR_SUCCESS;
}

void DeleteDevice (void)
{
 TCHAR	DeviceID[4096*sizeof(TCHAR)];

 FindDeviceID (_T("ROOT\\") _T(BUS_DRIVER_DEVICE_ID),DeviceID,sizeof(DeviceID)/sizeof(TCHAR));
 if (*DeviceID)
 {
  DebugPrintf ((_T("Found Device ID %s for ") _T("ROOT\\") _T(BUS_DRIVER_DEVICE_ID) _T("\n"),DeviceID));
  DeleteDeviceID (DeviceID,1);
 }
 else
 {
  DebugPrintf ((_T("Didn't find a Device ID for ") _T("ROOT\\") _T(BUS_DRIVER_DEVICE_ID) _T(" - not deleting joystick drivers\n")));
 }
}