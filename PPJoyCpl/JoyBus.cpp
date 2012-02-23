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
#include <winioctl.h>

#include <initguid.h>
#include <devguid.h>
#include "PPJGUID.h"

#include "Debug.h"
#include "JoyBus.h"

struct DEVTYPE_DATA
{
 TCHAR	*Name;
};

struct IFTYPE_DATA
{
 TCHAR	*Name;
 UCHAR	DevType;
 UCHAR	MaxUnits;
 UCHAR	MaxAxes;
 UCHAR	MaxButtons;
};

HANDLE				PPJoyBusDriver= INVALID_HANDLE_VALUE;
PJOYSTICK_ENUM_DATA	pEnumData= NULL;

DEVTYPE_DATA	DeviceTable[PPJOY_MAX_DEVTYPE+1]=
				{
					{_T("NULL")},
					{_T(DEVTYPE_JOYSTICK_NAME)},
					{_T(DEVTYPE_GENESIS_NAME)},
					{_T(DEVTYPE_SNESPAD_NAME)},
					{_T(DEVTYPE_PSXPAD_NAME)},
					{_T(DEVTYPE_IOCTL_NAME)},
					{_T(DEVTYPE_RETIRED_NAME)},
					{_T(DEVTYPE_RADIOTX_NAME)}
				};

#if (!(	(DEVTYPE_JOYSTICK==1) && (DEVTYPE_GENESIS==2) &&			\
		(DEVTYPE_SNESPAD==3) && (DEVTYPE_PSXPAD==4)	&&				\
		(DEVTYPE_IOCTL==5) && (DEVTYPE_RETIRED==6) && 				\
		(DEVTYPE_RADIOTX==7)						 				\
	))
#error Please update the order of the entries in the IFDefs table to match #define values
#endif

/* Define table with textual names, maximum units, buttons and axes for */
/* each of the supported joystick types. Sequence must match the values */
/* of the #define values for each joystick type                         */
IFTYPE_DATA		JoystickTable[PPJOY_MAX_IFTYPE+1]=
				{
/*					Joystick interface name		DevType				Units	Axes	Buttons */
					{_T("NULL"),					0,					0,		0,		0},			/* 0 */
					{_T(IF_THEMAZE_NAME),			IF_THEMAZE_DEV,		1,		2,		3},			/* 1 */
					{_T(IF_IANHARRIES_NAME),		IF_IANHARRIES_DEV,	1,		2,		1},			/* 2 */
					{_T(IF_TURBOGRAFX_NAME),		IF_TURBOGRAFX_DEV,	7,		2,		5},			/* 3 */
					{_T(IF_LINUX0802_NAME),			IF_LINUX0802_DEV,	2,		2,		1},			/* 4 */
					{_T(IF_LINUXDB9C_NAME),			IF_LINUXDB9C_DEV,	1,		2,		8},			/* 5 */
					{_T(IF_TORMOD_NAME),			IF_TORMOD_DEV,		1,		2,		4},			/* 6 */
					{_T(IF_DIRECTPRO_NAME),			IF_DIRECTPRO_DEV,	1,		2,		2},			/* 7 */
					{_T(IF_TURBOGFX45_NAME),		IF_TURBOGFX45_DEV,	7,		2,		5},			/* 8 */
					{_T(IF_LPTJOYSTICK_NAME),		IF_LPTJOYSTICK_DEV,	1,		2,		1},			/* 9 */
					{_T(IF_CHAMPGAMES_NAME),		IF_CHAMPGAMES_DEV,	1,		2,		1},			/* 10 */
					{_T(IF_STFORMAT_NAME),			IF_STFORMAT_DEV,	2,		2,		1},			/* 11 */
					{_T(IF_SNESKEY2600_NAME),		IF_SNESKEY2600_DEV,	1,		2,		1},			/* 12 */
					{_T(IF_AMIGA4PLAYER_NAME),		IF_AMIGA4PLAYER_DEV,2,		2,		2},			/* 13 */
					{_T(IF_PCAE_NAME),				IF_PCAE_DEV,		2,		2,		1},			/* 14 */
					{_T(IF_GENESISLIN_NAME),		IF_GENESISLIN_DEV,	1,		2,		8},			/* 15 */
					{_T(IF_GENESISDPP_NAME),		IF_GENESISDPP_DEV,	1,		2,		8},			/* 16 */
					{_T(IF_GENESISNTP_NAME),		IF_GENESISNTP_DEV,	2,		2,		8},			/* 17 */
					{_T(IF_SNESPADLIN_NAME),		IF_SNESPADLIN_DEV,	5,		2,		12},		/* 18 */
					{_T(IF_SNESPADDPP_NAME),		IF_SNESPADDPP_DEV,	5,		2,		12},		/* 19 */
					{_T(IF_GENESISCC_NAME),			IF_GENESISCC_DEV,	2,		2,		8},			/* 20 */
					{_T(IF_GENESISSNES_NAME),		IF_GENESISSNES_DEV,	1,		2,		8},			/* 21 */
					{_T(IF_PSXPBLIB_NAME),			IF_PSXPBLIB_DEV,	8,		4,		16},		/* 22 */
					{_T(IF_PSXDPADPRO_NAME),		IF_PSXDPADPRO_DEV,	2,		4,		16},		/* 23 */
					{_T(IF_PSXLINUX_NAME),			IF_PSXLINUX_DEV,	5,		4,		16},		/* 24 */
					{_T(IF_PSXNTPADXP_NAME),		IF_PSXNTPADXP_DEV,	6,		4,		16},		/* 25 */
					{_T(IF_PSXMEGATAP_NAME),		IF_PSXMEGATAP_DEV,	5,		4,		16},		/* 26 */
					{_T(IF_IOCTL_NAME),				IF_IOCTL_DEV,		16,		8,		16},		/* 27 */
					{_T(IF_LINUXGAMECON_NAME),		IF_LINUXGAMECON_DEV,5,		2,		2},			/* 28 */
					{_T(IF_LPTSWITCH_NAME),			IF_LPTSWITCH_DEV,	2,		2,		30},		/* 29 */
					{_T(IF_FMSBUDDYBOX_NAME),		IF_FMSBUDDYBOX_DEV,	1,		0,		0},			/* 30 */
					{_T(IF_POWERPADLIN_NAME),		IF_POWERPADLIN_DEV,	2,		0,		0},			/* 31 */
					{_T(IF_GENESISDPP6_NAME),		IF_GENESISDPP6_DEV,	1,		2,		8}			/* 32 */
				};

#if (!(	(IF_THEMAZE==1) && (IF_IANHARRIES==2) &&					\
		(IF_TURBOGRAFX==3) && (IF_LINUX0802==4) &&					\
		(IF_LINUXDB9C==5) && (IF_TORMOD==6) &&						\
		(IF_DIRECTPRO==7) && (IF_TURBOGFX45==8) &&					\
		(IF_LPTJOYSTICK==9) && (IF_CHAMPGAMES==10) &&				\
		(IF_STFORMAT==11) && (IF_SNESKEY2600==12) &&				\
		(IF_AMIGA4PLAYER==13) && (IF_PCAE==14) &&					\
		(IF_GENESISLIN==15) && (IF_GENESISDPP==16) &&				\
		(IF_GENESISNTP==17) && (IF_SNESPADLIN==18) &&				\
		(IF_SNESPADDPP==19) && (IF_GENESISCC==20) &&				\
		(IF_GENESISSNES==21) && (IF_PSXPBLIB==22) &&				\
		(IF_PSXDPADPRO==23) && (IF_PSXLINUX==24) &&					\
		(IF_PSXNTPADXP==25) && (IF_PSXMEGATAP==26) &&				\
		(IF_IOCTL==27) && (IF_LINUXGAMECON==28) &&					\
		(IF_LPTSWITCH==29) && (IF_FMSBUDDYBOX==30) &&				\
		(IF_POWERPADLIN==31) && (IF_GENESISDPP6==32)				\
	))
#error Please update the order of the entries in the JoystickTable table to match #define values
#endif

UCHAR GetDevTypeFromJoyType(UCHAR JoyType)
{
 if (JoyType>PPJOY_MAX_IFTYPE)
  return 0;
 return JoystickTable[JoyType].DevType;
}

TCHAR *GetDevTypeName(UCHAR DevType)
{
 if (DevType>PPJOY_MAX_DEVTYPE)
  return (_T("UnknownDev"));
 return DeviceTable[DevType].Name;
}

TCHAR *GetJoyTypeName (UCHAR JoyType)
{
 if (JoyType>PPJOY_MAX_IFTYPE)
  return (_T("UnknownIF"));
 return JoystickTable[JoyType].Name;
}

UCHAR GetJoyTypeMaxUnits (UCHAR JoyType)
{
 if (JoyType>PPJOY_MAX_IFTYPE)
  return 0;
 return JoystickTable[JoyType].MaxUnits;
}

//UCHAR GetJoyTypeMaxButtons (UCHAR JoyType)
//{
// if (JoyType>PPJOY_MAX_IFTYPE)
//  return 0;
// return JoystickTable[JoyType].MaxButtons;
//}

//UCHAR GetJoyTypeMaxAxes (UCHAR JoyType)
//{
// if (JoyType>PPJOY_MAX_IFTYPE)
//  return 0;
// return JoystickTable[JoyType].MaxAxes;
//}

int RescanParallelPorts (ULONG *PortArray, UCHAR ArraySize)
{
 HDEVINFO			DeviceInfoSet;
 SP_DEVINFO_DATA	DeviceInfoData;
 HKEY				DeviceKey;

 DWORD				Index;
 int				rc;
 TCHAR				Buffer[256*sizeof(TCHAR)];
 DWORD				BufSize;
 DWORD				ValueType;
 ULONG				LPTNumber;

 memset (PortArray,0,sizeof(*PortArray)*ArraySize);
#if 0
// // Use this GUID to enumerate parallel ports only
// DeviceInfoSet= SetupDiGetClassDevs ((LPGUID)&GUID_PARALLEL_DEVICE,NULL,NULL,DIGCF_DEVICEINTERFACE);
#endif
 // Use this GUID to get all Port (COM and LPT) type devices. Means we don't
 // need the DDK to build this application
 DeviceInfoSet= SetupDiGetClassDevs ((LPGUID)&GUID_DEVCLASS_PORTS,NULL,NULL,0);
 
 if(DeviceInfoSet==INVALID_HANDLE_VALUE)
 {
  DebugPrintf ((_T("SetupDiGetClassDevs failed: %d\n"), GetLastError()))
  return 0;
 }

 Index= 0;
 while (1)
 {
  /* Initialise buffer so that SetupDiGetDeviceInterfaceDetail will accept it. */
  DeviceInfoData.cbSize= sizeof(SP_DEVINFO_DATA);
  if (!SetupDiEnumDeviceInfo (DeviceInfoSet,Index++,&DeviceInfoData))
  {
   rc= GetLastError();
   if (rc!=ERROR_NO_MORE_ITEMS)
	DebugPrintf ((_T("SetupDiEnumDeviceInfo failed %d\n"),rc))
   break;
  }

  DeviceKey= SetupDiOpenDevRegKey (DeviceInfoSet,&DeviceInfoData,DICS_FLAG_GLOBAL,0,DIREG_DEV,KEY_READ);
  if (DeviceKey==INVALID_HANDLE_VALUE)
  {
   DebugPrintf ((_T("SetupDiOpenDevRegKey failed %d\n"),GetLastError()))
   continue;
  }

  BufSize= sizeof(Buffer);
  rc= RegQueryValueEx (DeviceKey,_T("PortName"),NULL,&ValueType,(LPBYTE)Buffer,&BufSize);
  RegCloseKey (DeviceKey);

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

  LPTNumber= GetLptNumber(Buffer);
  if (!LPTNumber)
  {
   DebugPrintf ((_T("Port ignored - not a valid LPT name\n")))
   continue;
  }
  if (LPTNumber>ArraySize)
  {
   DebugPrintf ((_T("Port ignored - LPT number (%d) is larger then array size (%d)\n"),LPTNumber,ArraySize))
   continue;
  }
  PortArray[LPTNumber-1]= LPTNumber;
 }

 SetupDiDestroyDeviceInfoList (DeviceInfoSet);
 return 1;	/* Need to relook return codes at a later stage... */
}

int OpenPPJoyBusDriver (void)
{
 HANDLE						DriverHandle;
 HDEVINFO					DeviceInfoSet;
 SP_INTERFACE_DEVICE_DATA	DeviceInterfaceData;
 int						rc;

 DriverHandle= INVALID_HANDLE_VALUE;
 // Open a handle to the device interface information set of all present PPJoy bus enumerator interfaces.
 DeviceInfoSet= SetupDiGetClassDevs ((LPGUID)&GUID_PPJOY_BUS,NULL,NULL,(DIGCF_PRESENT|DIGCF_INTERFACEDEVICE));
 if(DeviceInfoSet==INVALID_HANDLE_VALUE)
 {
  DebugPrintf ((_T("SetupDiGetClassDevs failed: %x\n"), GetLastError()))
  return 0;
 }

 DeviceInterfaceData.cbSize= sizeof(SP_INTERFACE_DEVICE_DATA);
 if (!SetupDiEnumDeviceInterfaces (DeviceInfoSet,0,(LPGUID)&GUID_PPJOY_BUS,0,&DeviceInterfaceData))
 {
  rc= GetLastError();
  if (rc==ERROR_NO_MORE_ITEMS)
   DebugPrintf ((_T("Error:Interface GUID_PPJOY_BUS is not registered\n")))
  else
   DebugPrintf ((_T("SetupDiEnumDeviceInterfaces() error %d\n"),rc))
 }
 else
 {
  PSP_INTERFACE_DEVICE_DETAIL_DATA	DevIfDetailData;
  ULONG								Length;
  
  Length= 0;
  SetupDiGetInterfaceDeviceDetail (DeviceInfoSet,&DeviceInterfaceData,NULL,0,&Length,NULL);

  DevIfDetailData= (PSP_INTERFACE_DEVICE_DETAIL_DATA) malloc(Length);
  /* Initialise buffer so that SetupDiGetDeviceInterfaceDetail will accept it. */
  DevIfDetailData->cbSize= sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);

  if (!SetupDiGetDeviceInterfaceDetail(DeviceInfoSet,&DeviceInterfaceData,DevIfDetailData,Length,&Length,NULL))
  {
   DebugPrintf ((_T("SetupDiGetDeviceInterfaceDetail() error %d\n"),GetLastError()))
  }
  else
  {
   DebugPrintf ((_T("Opening %s\n"),DevIfDetailData->DevicePath))
   DriverHandle= CreateFile(DevIfDetailData->DevicePath,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
   
   if (DriverHandle==INVALID_HANDLE_VALUE)
    DebugPrintf ((_T("CreateFile() error %d\n"),GetLastError()))
   else 
    DebugPrintf ((_T("Bus interface opened!!!\n")))
  }
  
  free (DevIfDetailData);
 }

 SetupDiDestroyDeviceInfoList (DeviceInfoSet);

 PPJoyBusDriver= DriverHandle;
 return PPJoyBusDriver!=INVALID_HANDLE_VALUE;
}

void ClosePPJoyBusDriver (void)
{
 if (PPJoyBusDriver!=INVALID_HANDLE_VALUE)
  CloseHandle (PPJoyBusDriver);
 PPJoyBusDriver=INVALID_HANDLE_VALUE;

 if (pEnumData)
  delete pEnumData;
 pEnumData= NULL;
}

int RescanJoysticks (void)
{
 DWORD	AllocSize;
 DWORD	BytesOut;
 DWORD	Count;
 DWORD	rc;

 if (pEnumData)
  delete (pEnumData);
 pEnumData= NULL;

 if (PPJoyBusDriver==INVALID_HANDLE_VALUE)
  return 0;

 AllocSize= ENUM_ALLOC_SIZE;
 pEnumData= (PJOYSTICK_ENUM_DATA) new char[AllocSize];
 if (!pEnumData)
 {
  DebugPrintf ((_T("Error allocating initial enum buffer (%d bytes)\n")))
  goto ExitOnly;
 }

 if (!DeviceIoControl (PPJoyBusDriver,IOCTL_PPJOYBUS_ENUM_JOY,NULL,0,pEnumData,AllocSize,&BytesOut,NULL))
 {
  rc= GetLastError();
  if (rc!=ERROR_MORE_DATA)
  {
   DebugPrintf ((_T("DeviceIoControl() error %d\n"),rc,BytesOut))
   goto ExitAndDealloc;
  }
 }

 if (BytesOut<pEnumData->Size)
 {
  // Allocate a new, bigger buffer
  AllocSize= pEnumData->Size;
  DebugPrintf ((_T("Allocating new buffer of %d bytes for IOCTL_PPJOYBUS_ENUM_JOY\n"),AllocSize))
  delete (pEnumData);
  pEnumData= (PJOYSTICK_ENUM_DATA) new char[AllocSize];
  if (!pEnumData)
  {
   DebugPrintf ((_T("Error allocating new enum buffer (%d bytes)\n")))
   goto ExitOnly;
  }
 }
 // Repeat the call to DeviceIoControl
 if (!DeviceIoControl (PPJoyBusDriver,IOCTL_PPJOYBUS_ENUM_JOY,NULL,0,pEnumData,AllocSize,&BytesOut,NULL))
 {
  rc= GetLastError();
  DebugPrintf ((_T("DeviceIoControl() 2 error %d\n"),rc))
  goto ExitAndDealloc;
 }

 DebugPrintf ((_T("\nJoysticks connected to the bus:\n")))
 DebugPrintf ((_T("-------------------------------\n\n")))
 DebugPrintf ((_T(" Index     LPT#     JoyType,UnitNumber,JoySubType)\n")))
 
 
 for (Count=0;Count<pEnumData->Count;Count++)
 {
  DebugPrintf ((_T(" %5d     %04X     %d,%d,%d\n"),Count,pEnumData->Joysticks[Count].LPTNumber,
	  pEnumData->Joysticks[Count].JoyType,pEnumData->Joysticks[Count].UnitNumber+1,
	  pEnumData->Joysticks[Count].JoySubType))
 }
 goto ExitOnly;

ExitAndDealloc: 
 free (pEnumData);
 pEnumData= NULL;

ExitOnly:
 return pEnumData!=NULL;
}

int DeleteJoystick (UCHAR LPTNumber, UCHAR JoyType, UCHAR UnitNumber)
{
 JOYSTICK_DEL_DATA	DelData;
 DWORD				BytesOut;

 memset (&DelData,0,sizeof(DelData));
 DelData.JoyData.Size= sizeof (DelData.JoyData);
 DelData.JoyData.LPTNumber= LPTNumber;
 DelData.JoyData.JoyType= JoyType;
 DelData.JoyData.UnitNumber= UnitNumber;

 if (!DeviceIoControl (PPJoyBusDriver,IOCTL_PPJOYBUS_DEL_JOY,&DelData,sizeof(DelData),NULL,0,&BytesOut,NULL))
 {
  DebugPrintf ((_T("DeviceIoControl error %d\n"),GetLastError()))
  return 0;
 }
 else
 {
  DebugPrintf ((_T("Successfully deleted joystick from bus\n")))
  return 1;
 }
}

int AddJoystick (UCHAR LPTNumber, UCHAR JoyType, UCHAR UnitNumber, UCHAR JoySubType, USHORT VendorID, USHORT ProductID)
{
 JOYSTICK_ADD_DATA	AddData;
 DWORD				BytesOut;

 memset (&AddData,0,sizeof(AddData));
 AddData.JoyData.Size= sizeof (AddData.JoyData);
 AddData.JoyData.LPTNumber= LPTNumber;
 AddData.JoyData.JoyType= JoyType;
 AddData.JoyData.JoySubType= JoySubType;
 AddData.JoyData.UnitNumber= UnitNumber;
 AddData.JoyData.VendorID= VendorID;
 AddData.JoyData.ProductID= ProductID;

 AddData.Persistent= 1;

 DebugPrintf ((_T("Adding joystick type 0x%X on port 0x%X (%s persistent)\n"),AddData.JoyData.JoyType,AddData.JoyData.LPTNumber,AddData.Persistent?_T(""):_T("not ")))

 if (!DeviceIoControl (PPJoyBusDriver,IOCTL_PPJOYBUS_ADD_JOY,&AddData,sizeof(AddData),NULL,0,&BytesOut,NULL))
 {
  DebugPrintf ((_T("DeviceIoControl error %d\n"),GetLastError()))
  return 0;
 }

 return 1;
}

ULONG GetLptNumber (TCHAR *LptName)
{
 ULONG	Number;

 Number= 0;
 /* Make sure the name starts with LPT */
 if ((LptName[0]=='L')&&(LptName[1]=='P')&&(LptName[2]=='T'))
 {
  LptName+= 3;
  /* Convert all following digits into a number. */
  while ((*LptName>='0')&&(*LptName<='9'))
   Number= Number*10+*(LptName++)-'0';

  /* If string contains other non-digits we got an error. */
  if (*LptName)
   Number= 0;
 }
 return Number;
}

int SetPPJoyOption (ULONG Option, ULONG Value)
{
 PPJOY_OPTIONS	OptionRec;
 DWORD			BytesOut;

 OptionRec.Option= Option;
 OptionRec.Value= Value;

 if (!DeviceIoControl (PPJoyBusDriver,IOCTL_PPJOYBUS_SET_OPTIONS,&OptionRec,sizeof(OptionRec),NULL,0,&BytesOut,NULL))
 {
  DebugPrintf ((_T("DeviceIoControl error %d\n"),GetLastError()))
  return 0;
 }

 DebugPrintf ((_T("Successfully set joystick options\n")))
 return 1;
}

int GetPPJoyOption (ULONG Option, PULONG Value)
{
 PPJOY_OPTIONS	OptionRec;
 DWORD			BytesOut;

 OptionRec.Option= Option;

 if (!DeviceIoControl (PPJoyBusDriver,IOCTL_PPJOYBUS_GET_OPTIONS,&OptionRec,sizeof(OptionRec),&OptionRec,sizeof(OptionRec),&BytesOut,NULL))
 {
  DebugPrintf ((_T("DeviceIoControl error %d\n"),GetLastError()))
  return 0;
 }

 *Value= OptionRec.Value;
 DebugPrintf ((_T("Successfully set joystick options\n")))
 return 1;
}

