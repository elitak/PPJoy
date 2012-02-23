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


/***************************************************************************/
/**                                                                       **/
/**  Parallel port joystick driver, (C) Deon van der Westhuysen 2002      **/
/**                                                                       **/
/**  Config.c   Routines to read and write configuration values from the  **/
/**             registry.                                                 **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/***************************************************************************/

#include "PPortJoy.h"

#include <wchar.h>

/**************************************************************************/
/* Declare the code segments into which each routine will be placed       */
/* Used to declare pagable and initialisation code.                       */
/**************************************************************************/

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE,PPJoy_OpenDeviceRegKey)
    #pragma alloc_text (PAGE,PPJoy_OpenDriverRegKey)
    #pragma alloc_text (PAGE,PPJoy_ReadRegULONG)
    #pragma alloc_text (PAGE,PPJoy_WriteRegULONG)
    #pragma alloc_text (PAGE,PPJoy_ReadDriverConfig)
    #pragma alloc_text (PAGE,PPJoy_WriteDriverConfig)
#endif



HANDLE PPJoy_OpenDeviceRegKey (IN PDEVICE_OBJECT DeviceObject)
{
 WCHAR				RegPathBuffer[256];
 UNICODE_STRING		RegPath;
 PDEVICE_EXTENSION	DeviceExtension;
 NTSTATUS			ntStatus;
 HANDLE				RegKey;
 OBJECT_ATTRIBUTES	oa;

 PAGED_CODE();

 DeviceExtension= GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
 swprintf (RegPathBuffer,L"%s\\Device%02X:%02X",Globals.ParamRegistryPath.Buffer,DeviceExtension->Config.LPTNumber,DeviceExtension->Config.UnitNumber+1);
 RtlInitUnicodeString(&RegPath,RegPathBuffer);

 InitializeObjectAttributes(&oa,&RegPath,0,NULL,NULL);
 ntStatus= ZwCreateKey(&RegKey,KEY_READ|KEY_WRITE,&oa,0,NULL,REG_OPTION_NON_VOLATILE,NULL);
 if(!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_WARN, ("Error opening device parameter registry key (%S) 0x%X",RegPath.Buffer,ntStatus) );
  RegKey= NULL;
 }

 return RegKey;
}

HANDLE PPJoy_OpenDriverRegKey (void)
{
 NTSTATUS			ntStatus;
 HANDLE				RegKey;
 OBJECT_ATTRIBUTES	oa;

 PAGED_CODE();

 InitializeObjectAttributes(&oa,&Globals.ParamRegistryPath,0,NULL,NULL);
 ntStatus= ZwCreateKey(&RegKey,KEY_READ|KEY_WRITE,&oa,0,NULL,REG_OPTION_NON_VOLATILE,NULL);
 if(!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_WARN, ("Error opening driver parameter registry key (%S) 0x%X",Globals.ParamRegistryPath.Buffer,ntStatus) );
  RegKey= NULL;
 }

 return RegKey;
}

NTSTATUS PPJoy_ReadRegULONG (HANDLE RegKey, WCHAR *ValueName, PULONG Result, ULONG DefaultValue)
{
 NTSTATUS			ntStatus;
 ULONG				NumRead;
 UNICODE_STRING		ValueNameStr;
 ULONG				MapSize;
 UCHAR				Buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+1024];
 PJOYSTICK_MAP		Mapping;

 PAGED_CODE();

 RtlInitUnicodeString(&ValueNameStr,ValueName);
 *Result= DefaultValue;

 NumRead= 0;
 ntStatus= ZwQueryValueKey(RegKey,&ValueNameStr,KeyValuePartialInformation,Buffer,sizeof(Buffer),&NumRead);
 if(!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_CONFIG|PPJOY_WARN, ("Error 0x%X reading value %S. NumRead= %d",ntStatus,ValueNameStr.Buffer,NumRead) );
  goto Exit;
 }

 if (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Type!=REG_DWORD)
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_WARN, ("Wrong registry datatype %d reading value %S",((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Type,ValueNameStr.Buffer) );
  goto Exit;
 }

 RtlCopyMemory (Result,((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data,sizeof(ULONG));

Exit:
 return ntStatus;
}

NTSTATUS PPJoy_WriteRegULONG (HANDLE RegKey, WCHAR *ValueName, ULONG Value)
{
 NTSTATUS			ntStatus;
 UNICODE_STRING		ValueNameStr;

 PAGED_CODE();

 RtlInitUnicodeString(&ValueNameStr,ValueName); 
 ntStatus= ZwSetValueKey (RegKey,&ValueNameStr,0,REG_DWORD,&Value,sizeof(Value));
 return ntStatus;
}

NTSTATUS PPJoy_ReadDriverConfig (void)
{
 HANDLE		RegKey;
 NTSTATUS	ntStatus;

 PPJoy_RuntimeDebug= RT_DEBUG_DEFAULT;

 RegKey= PPJoy_OpenDriverRegKey();
 if (!RegKey)
 {
  PPJOY_DBGPRINT (FILE_CONFIG|PPJOY_ERROR, ("Cannot open driver configuration registry key") );
  ntStatus= STATUS_UNSUCCESSFUL;
  goto Exit;
 }

 ntStatus= PPJoy_ReadRegULONG (RegKey,L"RuntimeDebug",&PPJoy_RuntimeDebug,RT_DEBUG_DEFAULT);
 if(!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_CONFIG|PPJOY_WARN, ("Error reading runtime debugging value from registry") );
 }

Exit:
 if (RegKey)
  ZwClose(RegKey);

 return ntStatus;
}

NTSTATUS PPJoy_WriteDriverConfig (void)
{
 HANDLE		RegKey;
 NTSTATUS	ntStatus;

 RegKey= PPJoy_OpenDriverRegKey();
 if (!RegKey)
 {
  PPJOY_DBGPRINT (FILE_CONFIG|PPJOY_ERROR, ("Cannot open driver configuration registry key") );
  ntStatus= STATUS_UNSUCCESSFUL;
  goto Exit;
 }

 ntStatus= PPJoy_WriteRegULONG (RegKey,L"RuntimeDebug",PPJoy_RuntimeDebug);
 if(!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_CONFIG|PPJOY_WARN, ("Error writing runtime debugging value to registry") );
 }

Exit:
 if (RegKey)
  ZwClose(RegKey);

 return ntStatus;
}
