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


#include "Install.h"

#include <windows.h>  
//#include <cfgmgr32.h>
#include <string.h>
#include <stdio.h>

#include <branding.h>
#include "Debug.h"

#ifndef	MAX_CLASS_NAME_LEN
#define	MAX_CLASS_NAME_LEN	32
#endif

typedef BOOL (WINAPI *SetupCopyOEMInfA_PTR) 
	(
	IN  PCSTR   SourceInfFileName,
    IN  PCSTR   OEMSourceMediaLocation,         OPTIONAL
    IN  DWORD   OEMSourceMediaType,
    IN  DWORD   CopyStyle,
    OUT PSTR    DestinationInfFileName,         OPTIONAL
    IN  DWORD   DestinationInfFileNameSize,
    OUT PDWORD  RequiredSize,                   OPTIONAL
    OUT PSTR   *DestinationInfFileNameComponent OPTIONAL
    );

typedef BOOL (WINAPI *UpdateDriverForPlugAndPlayDevicesA_PTR)
	(
    HWND hwndParent,
    LPCSTR HardwareId,
    LPCSTR FullInfPath,
    DWORD InstallFlags,
    PBOOL bRebootRequired OPTIONAL
    );

BOOL WINAPI SetupCopyOEMInf_Wrap
	(
	IN  PCSTR   SourceInfFileName,
    IN  PCSTR   OEMSourceMediaLocation,         OPTIONAL
    IN  DWORD   OEMSourceMediaType,
    IN  DWORD   CopyStyle,
    OUT PSTR    DestinationInfFileName,         OPTIONAL
    IN  DWORD   DestinationInfFileNameSize,
    OUT PDWORD  RequiredSize,                   OPTIONAL
    OUT PSTR   *DestinationInfFileNameComponent OPTIONAL
    )
{
 HMODULE				hModule;
 SetupCopyOEMInfA_PTR	SetupCopyOEMInfA_Ptr;

 hModule= GetModuleHandle(TEXT("SETUPAPI.DLL"));
 if (!hModule)
 {
  // Should never happen since we're linked to SetupAPI, but...
  DebugPrintf (("Could not find handle to SETUPAPI.DLL\n"))
  return FALSE;
 }

 SetupCopyOEMInfA_Ptr= (SetupCopyOEMInfA_PTR) GetProcAddress(hModule,"SetupCopyOEMInfA");
 if(SetupCopyOEMInfA_Ptr)
  return SetupCopyOEMInfA_Ptr(SourceInfFileName,OEMSourceMediaLocation,OEMSourceMediaType,CopyStyle,DestinationInfFileName,DestinationInfFileNameSize,RequiredSize,DestinationInfFileNameComponent);
 return FALSE;
}

BOOL WINAPI UpdateDriverForPlugAndPlayDevices_Wrap
	(
	HWND hwndParent,
	LPCSTR HardwareId,
    LPCSTR FullInfPath,
    DWORD InstallFlags,
    PBOOL bRebootRequired OPTIONAL
    )
{
 HINSTANCE								hModule;
 UpdateDriverForPlugAndPlayDevicesA_PTR	UpdateDriverForPlugAndPlayDevicesA_Ptr;
 BOOL									TempResult;

 hModule= LoadLibrary("NEWDEV.DLL");
 if (hModule)
 {
  UpdateDriverForPlugAndPlayDevicesA_Ptr= (UpdateDriverForPlugAndPlayDevicesA_PTR) GetProcAddress(hModule,"UpdateDriverForPlugAndPlayDevicesA");
  if(UpdateDriverForPlugAndPlayDevicesA_Ptr)
  {
   TempResult= UpdateDriverForPlugAndPlayDevicesA_Ptr (hwndParent,HardwareId,FullInfPath,InstallFlags,bRebootRequired);
   FreeLibrary (hModule);
   return TempResult;
  }

  FreeLibrary (hModule);
  DebugPrintf (("Cannot find UpdateDriverForPlugAndPlayDevicesA in newdev.dll!\n"))
 }

 return FALSE;
}


int FindExistingDevice (char *HardwareID)
{
 HDEVINFO			DeviceInfoSet;
 SP_DEVINFO_DATA	DeviceInfoData;

 DWORD				Index;
 char				Buffer[4096];

 int				rc;
 char				*Search;
    
 /* Create a Device Information Set with all present devices. */
 DeviceInfoSet= SetupDiGetClassDevs(NULL,0,0,DIGCF_ALLCLASSES|DIGCF_PRESENT);
 if (DeviceInfoSet==INVALID_HANDLE_VALUE)
 {
  DebugPrintf (("SetupDiGetClassDevs error %d\n",GetLastError()))
  return 0;
 }
    
 /* Search through all the devices and look for a hardware ID match */
 Index= 0;
 DeviceInfoData.cbSize= sizeof(DeviceInfoData);
 while (SetupDiEnumDeviceInfo(DeviceInfoSet,Index++,&DeviceInfoData))
 {
  if (SetupDiGetDeviceRegistryProperty(DeviceInfoSet,&DeviceInfoData,SPDRP_DEVICEDESC,NULL,Buffer,sizeof(Buffer),NULL))
  {
   DebugPrintf (("Checking device %s\n",Buffer))
  }

  if (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,&DeviceInfoData,SPDRP_HARDWAREID,NULL,Buffer,sizeof(Buffer),NULL))
  {
   rc= GetLastError();
   if (rc==ERROR_INVALID_DATA) 
    continue;

   DebugPrintf (("SetupDiGetDeviceRegistryProperty error %d\n",rc))
   continue;
  }
  
  Buffer[sizeof(Buffer)-1]= 0;
  Buffer[sizeof(Buffer)-2]= 0;

  Search= Buffer;
  while (*Search)
  {
   if (!_stricmp(Search,HardwareID))
   {
    /* Found a match for the hardware ID */
	SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    return 1;
   }
   Search+= strlen(Search)+1;
  }
 }      

 SetupDiDestroyDeviceInfoList(DeviceInfoSet);
 return 0;
}

/* HardwareID must be a multi-sz string (i.e. terminated with 2 zeros.) */
int InstallRootEnumDevice (char *HardwareID, char *INF)
{
 BOOL				RebootRequired;
 HDEVINFO			DeviceInfoSet;
 SP_DEVINFO_DATA	DeviceInfoData;
 GUID				ClassGUID;
 TCHAR				ClassName[MAX_CLASS_NAME_LEN];
    
 /* Get Class GUID from the INF file */
 if (!SetupDiGetINFClass(INF,&ClassGUID,ClassName,sizeof(ClassName),0))
 {
  DebugPrintf (("SetupDiGetINFClass error %d\n",GetLastError()))
  return 0;
 }
    
 /* Create the container for the to-be-created Device Information Element*/
 DeviceInfoSet= SetupDiCreateDeviceInfoList(&ClassGUID,0);
 if(DeviceInfoSet==INVALID_HANDLE_VALUE)
 {
  DebugPrintf (("SetupDiCreateDeviceInfoList error %d\n",GetLastError()))
  return 0;
 }

 /* Now create the element. Use the Class GUID and Name from the INF file. */
 DeviceInfoData.cbSize= sizeof(SP_DEVINFO_DATA);
 if (!SetupDiCreateDeviceInfo(DeviceInfoSet,ClassName,&ClassGUID,NULL,0,DICD_GENERATE_ID,&DeviceInfoData))
 {
  DebugPrintf (("SetupDiCreateDeviceInfo error %d\n",GetLastError()))
  goto cleanup_DeviceInfo;
 }
    
 /* Add the HardwareID to the Device's HardwareID property. */
 /* NOTE: Visual Studio 2008 doc says we shouldn't set this property. We */
 /* keep it in because install [on XP at least] fails without this call. */
 /* We are setting a MULTI_SZ so we include 2 extra bytes for buffer len */ 
 if(!SetupDiSetDeviceRegistryProperty(DeviceInfoSet,&DeviceInfoData,SPDRP_HARDWAREID,(LPBYTE)HardwareID,(DWORD)strlen(HardwareID)+1+1))
 {
  DebugPrintf (("SetupDiSetDeviceRegistryProperty error %d\n",GetLastError()))
  goto cleanup_DeviceInfo;
 }

 if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE,DeviceInfoSet,&DeviceInfoData))
 {
  DebugPrintf (("SetupDiCallClassInstaller error %d\n",GetLastError()))
  goto cleanup_DeviceInfo;
 }
    
 /* The element is now registered. We must explicitly remove the device */
 /* using DIF_REMOVE, if we encounter any failure from now on.          */
    
 /* Install the Driver. */
 if (UpdateDriverForPlugAndPlayDevices_Wrap(0,HardwareID,INF,0,&RebootRequired))
	 return RebootRequired?1:2;	/* 1 - reboot req, 2 - no reboot */

 DebugPrintf (("UpdateDriverForPlugAndPlayDevices error %d\n",GetLastError()))
 
 if (!SetupDiCallClassInstaller(DIF_REMOVE,DeviceInfoSet,&DeviceInfoData))
 {
  DebugPrintf (("SetupDiCallClassInstaller error %d\n",GetLastError()))
 }

cleanup_DeviceInfo:
 SetupDiDestroyDeviceInfoList(DeviceInfoSet);
 return 0;
}


int	InstallBusEnumerator (char *INFPath, char *HardwareID)
{
 BOOL	RebootRequired;

 if (FindExistingDevice (HardwareID))
 {
  /* Device already defined on system. Only update driver.   */
  /* we need to check to see if this updates the INF as well */
  DebugPrintf (("Bus enumerator already installed, updating drivers for it\n"))

  if (UpdateDriverForPlugAndPlayDevices_Wrap(0,HardwareID,INFPath,0,&RebootRequired))
  {
   return RebootRequired?1:2;
  }
  else
  {
   DebugPrintf (("UpdateDriverForPlugAndPlayDevices error %d\n",GetLastError()))
   return 0;
  }
 }
 else
 {
  DebugPrintf (("Bus enumerator not installed, installing bus enumerator\n"))
  return InstallRootEnumDevice (HardwareID,INFPath);
 }
}

int	InstallJoystickINF (char *INFPath)
{
 if (!SetupCopyOEMInf_Wrap(INFPath,NULL,SPOST_PATH,0,NULL,0,NULL,NULL))
 {
  DebugPrintf (("SetupCopyOEMInf error %d\n",GetLastError()))
  return 0;
 }
 return 1;
}

int	InstallJoystickDriver (char *INFPath, char *HardwareID)
{
 BOOL	RebootRequired;

 RebootRequired= FALSE;
 if (FindExistingDevice (HardwareID))
 {
  DebugPrintf (("Joystick with HardwareID %d detected, updating drivers\n",HardwareID))
  UpdateDriverForPlugAndPlayDevices_Wrap(0,HardwareID,INFPath,0,&RebootRequired);
 }

 return RebootRequired?1:2;
}

char *PrepareSourcePath (const char *SourceDirectory, char *Buffer)
{
 char	*EndPtr;

 strcpy (Buffer,SourceDirectory);		// Make a local copy of the source path
 EndPtr= Buffer+strlen(Buffer);			// Find the end of the the source string

 EndPtr--;								// Point to last character
 if (*EndPtr=='/')						// Convert forward slash to backslash
  *EndPtr='\\';
 if (*EndPtr!='\\')						// If no backslash add one
  *(++EndPtr)='\\';
 *(++EndPtr)= 0;						// Add a terminating zero

 return EndPtr;							// EndPtr points to terminating zero
}

// Routines to do the actual install
int InstallPPJoyDrivers (const char *SourceDirectory)
{
 char	FullSourcePath[4096];
 char	*Filename;
 int	InstallRC;
 int	FunctionRC;
 int	Count;
 char	HardwareID[64];

 Filename= PrepareSourcePath (SourceDirectory,FullSourcePath);

 FunctionRC= 2;

 // Install Joystick Function driver
 strcpy (Filename,JOY_DRIVER_INFNAME);
 DebugPrintf (("Installation path for joystick driver inf is: '%s'\n",FullSourcePath))

 InstallJoystickINF (FullSourcePath);

 for (Count=0;Count<16;Count++)
 {
  sprintf (HardwareID,JOY_DEVICE_PNP_ID,JOY_VENDOR_ID,JOY_PRODUCT_ID_BASE+Count);
  DebugPrintf (("Updating joystick driver for hardware ID  '%s'\n",HardwareID))
 
  InstallRC= InstallJoystickDriver (FullSourcePath,HardwareID);
  if (InstallRC<FunctionRC)
   FunctionRC= InstallRC;
 }

 // Install Joystick Bus enumerator
 strcpy (Filename,BUS_DRIVER_INFNAME);
 DebugPrintf (("Installation path for bus inf is: '%s'\n",FullSourcePath))
 InstallRC= InstallBusEnumerator (FullSourcePath,"ROOT\\" BUS_DRIVER_DEVICE_ID "\0\0");	// Make sure HardwareID is terminated by two 0's

 if (InstallRC<FunctionRC)
  FunctionRC= InstallRC;

 // Return InstallRC value. 0= error, 1= reboot, 2=fine
 return FunctionRC;
}
