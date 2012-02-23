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

#include "PPJoyBus50.h"

#include "Debug.h"

#include <stdio.h>

#include <setupapi.h>
#include <initguid.h>
#include <winioctl.h>

/* Do not use branded GUID value */
DEFINE_GUID(GUID_PPJOY_BUS,0x64c3b4c4,0xcdcc,0x49aa,0x99,0xa0,0x5b,0x4a,0xe4,0xb5,0xb1,0xbb);
/* 64c3b4c4-cdcc-49aa-99a0-5b4ae4b5b1bb */

#define FILE_DEVICE_PPJOYBUS			FILE_DEVICE_BUS_EXTENDER

#define PPJOYBUS_IOCTL(_index_)	\
	CTL_CODE (FILE_DEVICE_PPJOYBUS, _index_, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PPJOYBUS_DEL_JOY50		PPJOYBUS_IOCTL (0x1)
#define IOCTL_PPJOYBUS_ENUM_JOY50		PPJOYBUS_IOCTL (0x2)

HANDLE OpenPPJoyBusDriver50 (void)
{
 HANDLE						DriverHandle;
 HDEVINFO					HardwareDeviceInfo;
 SP_INTERFACE_DEVICE_DATA	DeviceInterfaceData;
 int						rc;

 DriverHandle= INVALID_HANDLE_VALUE;

 // Open a handle to the device interface information set of all present PPJoy bus enumerator interfaces.
 HardwareDeviceInfo= SetupDiGetClassDevs ((LPGUID)&GUID_PPJOY_BUS,NULL,NULL,(DIGCF_PRESENT|DIGCF_INTERFACEDEVICE));
 if(HardwareDeviceInfo==INVALID_HANDLE_VALUE)
 {
  DebugPrintf (("SetupDiGetClassDevs failed: %x\n", GetLastError()))
  return DriverHandle;
 }

 DeviceInterfaceData.cbSize= sizeof(SP_INTERFACE_DEVICE_DATA);
 if (!SetupDiEnumDeviceInterfaces (HardwareDeviceInfo,0,(LPGUID)&GUID_PPJOY_BUS,0,&DeviceInterfaceData))
 {
  rc= GetLastError();
  if (rc==ERROR_NO_MORE_ITEMS)
   DebugPrintf (("Error:Interface GUID_PPJOY_BUS is not registered\n"))
  else
   DebugPrintf (("SetupDiEnumDeviceInterfaces() error %d\n",rc))
 }
 else
 {
  PSP_INTERFACE_DEVICE_DETAIL_DATA	DevIfDetailData;
  ULONG								Length;
  
  Length= 0;
  SetupDiGetInterfaceDeviceDetail (HardwareDeviceInfo,&DeviceInterfaceData,NULL,0,&Length,NULL);

  DevIfDetailData= (PSP_INTERFACE_DEVICE_DETAIL_DATA) malloc(Length);
  DevIfDetailData->cbSize= sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);
  if (!SetupDiGetDeviceInterfaceDetail(HardwareDeviceInfo,&DeviceInterfaceData,DevIfDetailData,Length,&Length,NULL))
  {
   DebugPrintf (("SetupDiGetInterfaceDeviceDetail() error %d\n",GetLastError()))
  }
  else
  {
   DebugPrintf (("Opening %s\n",DevIfDetailData->DevicePath))
   DriverHandle= CreateFile(DevIfDetailData->DevicePath,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
   
   if (DriverHandle==INVALID_HANDLE_VALUE)
    DebugPrintf (("CreateFile() error %d\n",GetLastError()))
   else 
    DebugPrintf (("Bus interface opened!!!\n"))
  }
  
  free (DevIfDetailData);
 }

 SetupDiDestroyDeviceInfoList (HardwareDeviceInfo);
 return DriverHandle;
}

void EnumJoystick50 (HANDLE Driver, PJOYSTICK_ENUM_DATA50 *EnumData)
{
 DWORD					AllocSize;
 DWORD					BytesOut;
 DWORD					Count;
 DWORD					rc;

 AllocSize= 1024;												// Initial alloc size;
// AllocSize= sizeof (JOYSTICK_ENUM_DATA50);						// Initial alloc size;
 *EnumData= (PJOYSTICK_ENUM_DATA50) malloc (AllocSize);
 if (!*EnumData)
 {
  DebugPrintf (("Error allocating initial enum buffer (%d bytes)\n",AllocSize))
  return;
 }

 rc= 0;
 if (!DeviceIoControl (Driver,IOCTL_PPJOYBUS_ENUM_JOY50,NULL,0,*EnumData,AllocSize,&BytesOut,NULL))
 {
  rc= GetLastError();
  if (rc!=ERROR_MORE_DATA)
  {
   DebugPrintf (("DeviceIoControl() error %d\n",rc))
   free (*EnumData);
   *EnumData= NULL;
   return;
  }
 }

 DebugPrintf (("BytesOut= %d, rc= %d\n",BytesOut,rc))
 if (BytesOut==0)
 {
  DebugPrintf (("Nothing to do here... returning error\n"))
  free (*EnumData);
  *EnumData= NULL;
  return;
 }

 if (BytesOut<(*EnumData)->Size)
 {
  // Allocate a new, bigger buffer
  AllocSize= (*EnumData)->Size;
  DebugPrintf (("Allocating new buffer of %d bytes for IOCTL_PPJOYBUS_ENUM_JOY\n",AllocSize))
  *EnumData= (PJOYSTICK_ENUM_DATA50) realloc (*EnumData,AllocSize);
  if (!*EnumData)
  {
   DebugPrintf (("Error allocating new enum buffer (%d bytes)\n",AllocSize))
   return;
  }
 }

 // Repeat the call to DeviceIoControl
 if (!DeviceIoControl (Driver,IOCTL_PPJOYBUS_ENUM_JOY50,NULL,0,*EnumData,AllocSize,&BytesOut,NULL))
 {
  rc= GetLastError();
  DebugPrintf (("DeviceIoControl() [2] error %d\n",rc))
  free (*EnumData);
  *EnumData= NULL;
  return;
 }

 printf ("\nJoysticks connected to the bus:\n");
 printf ("-------------------------------\n\n");
 printf (" Index     Port     Type (JoyType,SubUnit,Axes,Buttons)\n");
 
 
 for (Count=0;Count<(*EnumData)->Count;Count++)
 {
  printf (" %5d     %04X     %4X (%d,%d,%d,%d)\n",Count,(*EnumData)->Joys[Count].Port,(*EnumData)->Joys[Count].TypeAsLong,
		  (*EnumData)->Joys[Count].JoyType,(*EnumData)->Joys[Count].SubAddr,(*EnumData)->Joys[Count].NumAxes,
		  (*EnumData)->Joys[Count].NumButtons);
 }

 return;
}

void DelJoystick50 (HANDLE Driver, unsigned int Port, unsigned int TypeAsLong)
{
 JOYSTICK_DEL_DATA50	DelData;
 DWORD					BytesOut;

 DelData.JoyData.Port= Port;
 DelData.JoyData.TypeAsLong= TypeAsLong;

 DebugPrintf (("Deleting joystick type 0x%X on port 0x%X\n",DelData.JoyData.TypeAsLong,DelData.JoyData.Port))

 if (!DeviceIoControl (Driver,IOCTL_PPJOYBUS_DEL_JOY50,&DelData,sizeof(DelData),NULL,0,&BytesOut,NULL))
  DebugPrintf (("DeviceIoControl error %d\n",GetLastError()))
 else
  DebugPrintf (("Successfully deleted joystick from bus\n"))
}
