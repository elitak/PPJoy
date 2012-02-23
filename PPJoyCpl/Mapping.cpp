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

#include "Mapping.h"

#include <Windows.h>
#include <winioctl.h>
#include <stdio.h>

#include <setupapi.h>
#include <cfgmgr32.h>

#include "Debug.h"

#include <branding.h>

//#define HID_USAGE_GENERIC_HATSWITCH                ((USAGE) 0x39)

SAxisDefines	AxisUsageList[]=
	{
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,_T("X Axis")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,_T("Y Axis")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Z,_T("Z Axis")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_RX,_T("X Rotation")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_RY,_T("Y Rotation")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_RZ,_T("Z Rotation")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_SLIDER,_T("Slider")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_DIAL,_T("Dial")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_WHEEL,_T("Wheel")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_VX,_T("VX Axis")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_VY,_T("VY Axis")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_VZ,_T("VZ Axis")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_VBRX,_T("VBRX Axis")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_VBRY,_T("VBRY Axis")},
	{HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_VBRZ,_T("VBRZ Axis")},
	{HID_USAGE_PAGE_SIMULATION,HID_USAGE_SIMULATION_RUDDER,_T("Rudder")},
	{HID_USAGE_PAGE_SIMULATION,HID_USAGE_SIMULATION_THROTTLE,_T("Throttle")},
	{0,0,NULL}
	};
//
//{HID_USAGE_PAGE_VR             ((USAGE) 0x03)
//{HID_USAGE_PAGE_SPORT          ((USAGE) 0x04)
//{HID_USAGE_PAGE_GAME           ((USAGE) 0x05)
//

#pragma pack(push,1)		/* All fields in structure must be byte aligned. */
typedef struct
{
 JOYSTICKMAP_HEADER	jh;
 JOYSTICK_MAP		jm;
 char			Buffer[4096];
}	JOYSTICKMAPPING;

typedef struct
{
 unsigned long	Signature;				/* Signature to identify packet to PPJoy IOCTL */
 char			Data[1024];
}	JOYSTICK_STATE;

typedef struct
{
 UCHAR	NumAnalog;
 ULONG	Analog[1];
}	*PANALOG_DATA;

typedef struct
{
 UCHAR	NumDigital;
 UCHAR	Digital[1];
}	*PDIGITAL_DATA;

typedef struct
{
 TIMING_HEADER	th;
 JOYTIMING		jt;
}	JOYSTICKTIMING;

#pragma pack(pop)


HANDLE OpenJoystickDevice (int Port, int SubAddr)
{
 TCHAR	DeviceName[128*sizeof(TCHAR)];
 HANDLE	hJoy;

 _stprintf (DeviceName,_T("\\\\.\\") _T(JOY_CONTROL_DEV_NAME),Port,SubAddr+1);

 /* Open a handle to the control device for the joystick. */
 hJoy= CreateFile(DeviceName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
// if (hJoy==INVALID_HANDLE_VALUE)
// {
//  printf ("Error %d opening joystick device %s.\n",GetLastError(),DeviceName);
//  return 0;
// }
 return hJoy;
}

int ReadMapping (HANDLE hJoy, int JoyType, JOYSTICK_MAP **Buffer, int MapScope)
{
 JOYSTICKMAPPING	Map;
 DWORD				RetSize;
 DWORD				rc;
 
 Map.jh.Version= JOYSTICK_MAP_V1;
 Map.jh.MapScope= MapScope;
 Map.jh.JoyType= JoyType;

 if (!DeviceIoControl(hJoy,IOCTL_PPORTJOY_GET_MAPPING,&Map,sizeof(Map),&Map,sizeof(Map),&RetSize,NULL))
 {
  rc= GetLastError();
  return 0;
 }

 *Buffer= (JOYSTICK_MAP*) malloc(Map.jh.MapSize);
 if (!*Buffer)
  return 0;

 memcpy (*Buffer,&(Map.jm),Map.jh.MapSize);
 return 1;
}

int	ReadJoystickMappings (int Port, int JoyType, int SubAddr, JOYSTICK_MAP **DefaultMapping, JOYSTICK_MAP **DriverMapping, JOYSTICK_MAP **DeviceMapping)
{
 HANDLE	hJoy;
 
 *DefaultMapping= *DriverMapping= *DeviceMapping= NULL;

 hJoy= OpenJoystickDevice (Port,SubAddr);
 if (!hJoy)
  return 0;

 ReadMapping (hJoy,JoyType,DefaultMapping,MAP_SCOPE_DEFAULT);
 ReadMapping (hJoy,JoyType,DriverMapping,MAP_SCOPE_DRIVER);
 ReadMapping (hJoy,JoyType,DeviceMapping,MAP_SCOPE_DEVICE);

 CloseHandle (hJoy);
 return 1;
}

int	WriteJoystickMapping (int Port, int JoyType, int SubAddr, JOYSTICK_MAP *Mapping, int MapSize, int Scope)
{
 HANDLE				hJoy;
 JOYSTICKMAPPING	*MapIOCTL;
 int				MapIOCTLSize;
 DWORD				RetSize;
 DWORD				rc;

 hJoy= OpenJoystickDevice (Port,SubAddr);
 if (!hJoy)
  return 2;

 MapIOCTLSize= sizeof(JOYSTICKMAP_HEADER)+MapSize;
 MapIOCTL= (JOYSTICKMAPPING*) malloc (MapIOCTLSize);
 if (!MapIOCTL)
 {
  CloseHandle (hJoy);
  return 8;
 }

 MapIOCTL->jh.Version= JOYSTICK_MAP_V1;
 MapIOCTL->jh.MapScope= Scope;
 MapIOCTL->jh.JoyType= JoyType;
 MapIOCTL->jh.MapSize= MapSize;
 memcpy (&(MapIOCTL->jm),Mapping,MapSize);

 rc= 0;
 if (!DeviceIoControl(hJoy,IOCTL_PPORTJOY_SET_MAPPING,MapIOCTL,MapIOCTLSize,NULL,0,&RetSize,NULL))
  rc= GetLastError();

 CloseHandle (hJoy);
 free (MapIOCTL);
 return rc;
}

int DeleteJoystickMapping (int Port, int JoyType, int SubAddr, int Scope)
{
 HANDLE				hJoy;
 JOYSTICKMAP_HEADER	MapIOCTL;
 DWORD				rc;
 DWORD				RetSize;

 hJoy= OpenJoystickDevice (Port,SubAddr);
 if (!hJoy)
  return 2;

 MapIOCTL.Version= JOYSTICK_MAP_V1;
 MapIOCTL.MapScope= Scope;
 MapIOCTL.JoyType= JoyType;

 rc= 0;
 if (!DeviceIoControl(hJoy,IOCTL_PPORTJOY_DEL_MAPPING,&MapIOCTL,sizeof(MapIOCTL),NULL,0,&RetSize,NULL))
  rc= GetLastError();

 CloseHandle (hJoy);
 return rc;
}

int	ScanJoystickInput (HANDLE hJoy, int ScanType)
{
 JOYSTICK_STATE	JoyState;
 DWORD			rc;
 DWORD			RetSize;
 DWORD			Count;

 PANALOG_DATA	pAnalog;
 PDIGITAL_DATA	pDigital;

 int			NumDigital;
 int			NumAnalog;

 if (!hJoy)
  return -2;

 if (!DeviceIoControl(hJoy,IOCTL_PPORTJOY_GET_STATE,NULL,0,&JoyState,sizeof(JoyState),&RetSize,NULL))
 {
  rc= GetLastError();
  return -3;
 }

 pAnalog= (PANALOG_DATA) JoyState.Data;
 pDigital= (PDIGITAL_DATA) (JoyState.Data+pAnalog->NumAnalog*sizeof(long)+1);
    
 NumAnalog= pAnalog->NumAnalog;
 if (NumAnalog>63)
  NumAnalog= 63;

 NumDigital= pDigital->NumDigital;
 if (NumDigital>127)
  NumDigital= 127;

 for (Count=0;Count<pAnalog->NumAnalog;Count++)
 {
  if (pAnalog->Analog[Count]>ANALOG_MAX_TRIP)
  {
   switch (ScanType)
   {
    case SCANTYPE_AXISMIN:	break;	/*return Count|0xC0;*/	// We don't allow analog axismin scans - because axismin only enabled if axismax is digital
    case SCANTYPE_AXISMAX:	return Count|0x80;
    case SCANTYPE_BUTTON:	return Count|0xC0;
    case SCANTYPE_HATDIAL:	return Count|0xC0;
    default:				return -1;
   }
  }
  if (pAnalog->Analog[Count]<ANALOG_MIN_TRIP)
  {
   switch (ScanType)
   {
    case SCANTYPE_AXISMIN:	break;	/*return Count|0x80;*/	// We don't allow analog axismin scans - because axismin only enabled if axismax is digital
    case SCANTYPE_AXISMAX:	return Count|0xC0;
    case SCANTYPE_BUTTON:	return Count|0x80;
    case SCANTYPE_HATDIAL:	return Count|0x80;
    default:				return -1;
   }
  }
 }

 if (ScanType==SCANTYPE_HATDIAL)
  return -4; /* No digital values valid for hat dial scan */

 for (Count=0;Count<pDigital->NumDigital;Count++)
  if (pDigital->Digital[Count])
   return Count;

 return -4;
}

int ReadTiming (HANDLE hJoy, int JoyType, PJOYTIMING *Buffer, int TimingScope)
{
 JOYSTICKTIMING		Timing;
 DWORD				RetSize;
 DWORD				rc;
 
 Timing.th.Version= TIMING_V1;
 Timing.th.TimingScope= TimingScope;
 Timing.th.JoyType= JoyType;

 if (!DeviceIoControl(hJoy,IOCTL_PPORTJOY_GET_TIMING,&Timing,sizeof(Timing),&Timing,sizeof(Timing),&RetSize,NULL))
 {
  rc= GetLastError();
  return 0;
 }

 *Buffer= (PJOYTIMING) malloc(Timing.th.TimingSize);
 if (!*Buffer)
  return 0;

 memcpy (*Buffer,&(Timing.jt),Timing.th.TimingSize);
 return 1;
}

int	ReadJoystickTimings (int Port, int JoyType, int SubAddr, PJOYTIMING *DefaultTiming, PJOYTIMING *DriverTiming, PJOYTIMING *DeviceTiming)
{
 HANDLE	hJoy;
 
 *DefaultTiming= *DriverTiming= *DeviceTiming= NULL;

 hJoy= OpenJoystickDevice (Port,SubAddr);
 if (!hJoy)
  return 0;

 ReadTiming (hJoy,JoyType,DefaultTiming,TIMING_SCOPE_DEFAULT);
 ReadTiming (hJoy,JoyType,DriverTiming,TIMING_SCOPE_DRIVER);
 ReadTiming (hJoy,JoyType,DeviceTiming,TIMING_SCOPE_DEVICE);

 CloseHandle (hJoy);
 return 1;
}

int	WriteJoystickTiming (int Port, int JoyType, int SubAddr, PJOYTIMING Timing, int TimingSize, int Scope)
{
 HANDLE				hJoy;
 JOYSTICKTIMING		TimingIOCTL;
 int				TimingIOCTLSize;
 DWORD				RetSize;
 DWORD				rc;

 hJoy= OpenJoystickDevice (Port,SubAddr);
 if (!hJoy)
  return 2;

 TimingIOCTLSize= sizeof(TIMING_HEADER)+TimingSize;
 memcpy (&(TimingIOCTL.jt),Timing,TimingSize);

 TimingIOCTL.th.Version= TIMING_V1;
 TimingIOCTL.th.TimingScope= Scope;
 TimingIOCTL.th.JoyType= JoyType;
 TimingIOCTL.th.TimingSize= TimingSize;

 rc= 0;
 if (!DeviceIoControl(hJoy,IOCTL_PPORTJOY_SET_TIMING,&TimingIOCTL,TimingIOCTLSize,NULL,0,&RetSize,NULL))
  rc= GetLastError();

 CloseHandle (hJoy);
 return rc;
}

int DeleteJoystickTiming (int Port, int JoyType, int SubAddr, int Scope)
{
 HANDLE				hJoy;
 TIMING_HEADER		TimingIOCTL;
 DWORD				rc;
 DWORD				RetSize;

 hJoy= OpenJoystickDevice (Port,SubAddr);
 if (!hJoy)
  return 2;

 TimingIOCTL.Version= TIMING_V1;
 TimingIOCTL.TimingScope= Scope;
 TimingIOCTL.JoyType= JoyType;

 rc= 0;
 if (!DeviceIoControl(hJoy,IOCTL_PPORTJOY_DEL_TIMING,&TimingIOCTL,sizeof(TimingIOCTL),NULL,0,&RetSize,NULL))
  rc= GetLastError();

 CloseHandle (hJoy);
 return rc;
}

int ActivateJoystickTiming (int Port, int JoyType, int SubAddr)
{
 HANDLE				hJoy;
 TIMING_HEADER		TimingIOCTL;
 DWORD				rc;
 DWORD				RetSize;

 hJoy= OpenJoystickDevice (Port,SubAddr);
 if (!hJoy)
  return 2;

 TimingIOCTL.Version= TIMING_V1;
 TimingIOCTL.TimingScope= TIMING_SCOPE_DEFAULT;
 TimingIOCTL.JoyType= JoyType;

 rc= 0;
 if (!DeviceIoControl(hJoy,IOCTL_PPORTJOY_ACT_TIMING,&TimingIOCTL,sizeof(TimingIOCTL),NULL,0,&RetSize,NULL))
  rc= GetLastError();

 CloseHandle (hJoy);
 return rc;
}
