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
/**  Mapping.c  Routines to handle remapping of joystick axes and buttons **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**  Portions of this file derived from the Windows 2000 DDK              **/
/**                                                                       **/
/***************************************************************************/

#include "PPortJoy.h"
//#include "JoyDefs.h"
#include <wchar.h>

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE,PPJOY_ComputeTimingSize)
    #pragma alloc_text (PAGE,PPJoy_InitTimingValues)
    #pragma alloc_text (PAGE,PPJoy_DeleteTimingFromReg)
    #pragma alloc_text (PAGE,PPJoy_DeleteDeviceTiming)
    #pragma alloc_text (PAGE,PPJoy_DeleteDriverTiming)
    #pragma alloc_text (PAGE,PPJoy_WriteTimingToReg)
    #pragma alloc_text (PAGE,PPJoy_WriteDeviceTiming)
    #pragma alloc_text (PAGE,PPJoy_WriteDriverTiming)
    #pragma alloc_text (PAGE,PPJoy_ReadTimingFromReg)
    #pragma alloc_text (PAGE,PPJoy_ReadDeviceTiming)
    #pragma alloc_text (PAGE,PPJoy_ReadDriverTiming)
    #pragma alloc_text (PAGE,PPJoy_BuildDefaultTiming)
    #pragma alloc_text (PAGE,PPJoy_LoadJoystickTiming)
#endif


ULONG PPJOY_ComputeTimingSize (UCHAR JoyType)
{

 PAGED_CODE();

 switch (JoyType)
 {
  case IF_GENESISLIN:
  case IF_GENESISDPP:
  case IF_GENESISNTP:
  case IF_GENESISCC:
  case IF_GENESISSNES:
  case IF_GENESISDPP6:
		return sizeof(TIMING_GENESIS);

  case IF_SNESPADLIN:
  case IF_SNESPADDPP:
  case IF_POWERPADLIN:
		return sizeof(TIMING_SNES);

  case IF_PSXPBLIB:
  case IF_PSXDPADPRO:
  case IF_PSXLINUX:
  case IF_PSXNTPADXP:
  case IF_PSXMEGATAP:
		return sizeof(TIMING_PSX);

  case IF_LPTSWITCH:
		return sizeof(TIMING_LPTSWITCH);

  case IF_FMSBUDDYBOX:
		return sizeof(TIMING_FMSPPM);

  default:
		return 0;
 }
}

void PPJoy_InitTimingValues (IN PDEVICE_OBJECT DeviceObject)
{
 PDEVICE_EXTENSION	DeviceExtension;
 LARGE_INTEGER		TickCount;

 PAGED_CODE();

 DeviceExtension= GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

 if (DeviceExtension->Config.JoyType==IF_FMSBUDDYBOX)
 {
  PTIMING_FMSPPM	PPMTiming;
  PPPMBUDDYBOX		PPMParams;

  PPMTiming= (PTIMING_FMSPPM) DeviceExtension->Timing;
  PPMParams= &(DeviceExtension->Param.PPMBuddyBox);

  KeQueryPerformanceCounter(&TickCount);
  PPMParams->MaxPulseTicks.QuadPart= (TickCount.QuadPart*(PPMTiming->MaxPulseWidth/*us*/)/1000000);
  PPMParams->MinPulseTicks.QuadPart= (TickCount.QuadPart*(PPMTiming->MinPulseWidth/*us*/)/1000000);
  PPMParams->MinSyncTicks.QuadPart= (TickCount.QuadPart*(PPMTiming->MinSyncWidth/*us*/)/1000000);
 }
}

NTSTATUS PPJoy_DeleteTimingFromReg (UCHAR JoyType, HANDLE RegKey)
{
 NTSTATUS			ntStatus;
 UNICODE_STRING		ValueName;
 WCHAR				ValueNameBuffer[128];

 PAGED_CODE();

 swprintf (ValueNameBuffer,L"TimingV1_%02X",JoyType);
 RtlInitUnicodeString(&ValueName,ValueNameBuffer);

 ntStatus= ZwSetValueKey (RegKey,&ValueName,0,REG_BINARY,ValueNameBuffer,0);
 return ntStatus;
}


NTSTATUS PPJoy_DeleteDeviceTiming (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType)
{
 HANDLE		RegKey;
 NTSTATUS	ntStatus;

 RegKey= PPJoy_OpenDeviceRegKey(DeviceObject);
 if (!RegKey)
  return STATUS_UNSUCCESSFUL;

 ntStatus= PPJoy_DeleteTimingFromReg (JoyType,RegKey);
 ZwClose(RegKey);

 return ntStatus;
}

NTSTATUS PPJoy_DeleteDriverTiming (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType)
{
 HANDLE		RegKey;
 NTSTATUS	ntStatus;

 RegKey= PPJoy_OpenDriverRegKey();
 if (!RegKey)
  return STATUS_UNSUCCESSFUL;

 ntStatus= PPJoy_DeleteTimingFromReg (JoyType,RegKey);
 ZwClose(RegKey);

 return ntStatus;
}

NTSTATUS PPJoy_WriteTimingToReg (PJOYTIMING Timing, UCHAR JoyType, HANDLE RegKey)
{
 NTSTATUS			ntStatus;
 UNICODE_STRING		ValueName;
 WCHAR				ValueNameBuffer[128];
 ULONG				TimingSize;

 PAGED_CODE();

 swprintf (ValueNameBuffer,L"TimingV1_%02X",JoyType);
 RtlInitUnicodeString(&ValueName,ValueNameBuffer);

 TimingSize= PPJOY_ComputeTimingSize(JoyType);

 if (!TimingSize)
  return STATUS_SUCCESS;
 
 ntStatus= ZwSetValueKey (RegKey,&ValueName,0,REG_BINARY,Timing,TimingSize);
 return ntStatus;
}

NTSTATUS PPJoy_WriteDeviceTiming (IN PDEVICE_OBJECT DeviceObject, PJOYTIMING Timing, UCHAR JoyType)
{
 HANDLE		RegKey;
 NTSTATUS	ntStatus;

 RegKey= PPJoy_OpenDeviceRegKey(DeviceObject);
 if (!RegKey)
  return STATUS_UNSUCCESSFUL;

 ntStatus= PPJoy_WriteTimingToReg (Timing,JoyType,RegKey);
 ZwClose(RegKey);

 return ntStatus;
}

NTSTATUS PPJoy_WriteDriverTiming (IN PDEVICE_OBJECT DeviceObject, PJOYTIMING Timing, UCHAR JoyType)
{
 HANDLE		RegKey;
 NTSTATUS	ntStatus;

 RegKey= PPJoy_OpenDriverRegKey();
 if (!RegKey)
  return STATUS_UNSUCCESSFUL;

 ntStatus= PPJoy_WriteTimingToReg (Timing,JoyType,RegKey);
 ZwClose(RegKey);

 return ntStatus;
}

PJOYTIMING PPJoy_ReadTimingFromReg (UCHAR JoyType, HANDLE RegKey)
{
 NTSTATUS			ntStatus;
 ULONG				NumRead;
 UNICODE_STRING		ValueName;
 WCHAR				ValueNameBuffer[128];
 UCHAR				Buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+1024];
 ULONG				TimingSize;
 PJOYTIMING			Timing;

 PAGED_CODE();

 Timing= NULL;

 swprintf (ValueNameBuffer,L"TimingV1_%02X",JoyType);
 RtlInitUnicodeString(&ValueName,ValueNameBuffer);

 NumRead= 0;
 ntStatus= ZwQueryValueKey(RegKey,&ValueName,KeyValuePartialInformation,Buffer,sizeof(Buffer),&NumRead);
 if(!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_TIMING|PPJOY_WARN, ("Error 0x%X reading value %S. NumRead= %d",ntStatus,ValueName.Buffer,NumRead) );
  goto Exit;
 }

 if (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Type!=REG_BINARY)
 {
  PPJOY_DBGPRINT (FILE_TIMING|PPJOY_WARN, ("Wrong registry datatype %d reading value %S",((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Type,ValueName.Buffer) );
  goto Exit;
 }

 if (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength==0)
 {
  PPJOY_DBGPRINT (FILE_TIMING|PPJOY_BABBLE, ("Empty registry key, ignoring") );
  goto Exit;
 }

 TimingSize= PPJOY_ComputeTimingSize(JoyType);

 if (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength!=TimingSize)
 {
  PPJOY_DBGPRINT (FILE_TIMING|PPJOY_WARN, ("Invalid registry value size. Value size= %d expected %d",((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength,TimingSize) );
  goto Exit;
 }

 Timing= ExAllocatePoolWithTag (NonPagedPool,TimingSize,PPJOY_POOL_TAG);
 if (!Timing)
 {
  PPJOY_DBGPRINT (FILE_TIMING|PPJOY_ERROR, ("Cannot allocate nonpaged pool to store joystick timing. Help!") );
  goto Exit;
 }

 RtlCopyMemory (Timing,((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data,TimingSize);

Exit:
 return Timing;
}

PJOYTIMING PPJoy_ReadDeviceTiming (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType)
{
 HANDLE			RegKey;
 PJOYTIMING		Timing;

 RegKey= PPJoy_OpenDeviceRegKey(DeviceObject);
 if (!RegKey)
  return NULL;

 Timing= PPJoy_ReadTimingFromReg(JoyType,RegKey);
 ZwClose(RegKey);

 return Timing;
}

PJOYTIMING PPJoy_ReadDriverTiming (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType)
{
 HANDLE			RegKey;
 PJOYTIMING		Timing;

 RegKey= PPJoy_OpenDriverRegKey();
 if (!RegKey)
  return NULL;

 Timing= PPJoy_ReadTimingFromReg(JoyType,RegKey);
 ZwClose(RegKey);

 return Timing;
}

PJOYTIMING PPJoy_BuildDefaultTiming (UCHAR JoyType)
{
 NTSTATUS		ntStatus;
 PJOYTIMING		Timing;
// PJOYSTICK_MAP	SrcMapping;
 int			TimingSize;

 PAGED_CODE();

 TimingSize= PPJOY_ComputeTimingSize(JoyType);
 if (!TimingSize)
  return NULL;

 Timing= ExAllocatePoolWithTag (NonPagedPool,TimingSize,PPJOY_POOL_TAG);
 if (!Timing)
 {
  PPJOY_DBGPRINT (FILE_TIMING|PPJOY_ERROR, ("Cannot allocate nonpaged pool to store joystick timing. Help!") );
  goto Exit;
 }

 switch (JoyType)
 {
  case IF_GENESISLIN:
  case IF_GENESISDPP:
  case IF_GENESISNTP:
  case IF_GENESISCC:
  case IF_GENESISSNES:
  case IF_GENESISDPP6:
			((PTIMING_GENESIS)Timing)->BitDelay= GENESIS_DELAY;
			((PTIMING_GENESIS)Timing)->Bit6Delay= GENESIS6_DELAY;
			break;

  case IF_SNESPADLIN:
  case IF_SNESPADDPP:
  case IF_POWERPADLIN:
			((PTIMING_SNES)Timing)->BitDelay= SNES_BITDELAY;
			((PTIMING_SNES)Timing)->SetupDelay= SNES_SETUPDELAY;
			break;

  case IF_PSXPBLIB:
  case IF_PSXDPADPRO:
  case IF_PSXLINUX:
  case IF_PSXNTPADXP:
  case IF_PSXMEGATAP:
			((PTIMING_PSX)Timing)->Ack1Delay= PSXACK1_DELAY;
			((PTIMING_PSX)Timing)->AckXDelay= PSXACKX_DELAY;
			((PTIMING_PSX)Timing)->BitDelay= PSXBIT_DELAY;
			((PTIMING_PSX)Timing)->SelDelay= PSXSEL_DELAY;
			((PTIMING_PSX)Timing)->TailDelay= PSXTAIL_DELAY;
			break;

  case IF_LPTSWITCH:
			((PTIMING_LPTSWITCH)Timing)->RowDelay= LPTSWITCH_DELAY;
			break;

  case IF_FMSBUDDYBOX:
			((PTIMING_FMSPPM)Timing)->MaxPulseWidth= FMS_MAXPULSE;
			((PTIMING_FMSPPM)Timing)->MinPulseWidth= FMS_MINPULSE;
			((PTIMING_FMSPPM)Timing)->MinSyncWidth= FMS_MINSYNC;
			break;
 }

Exit:
 return Timing;
}

NTSTATUS PPJoy_LoadJoystickTiming (IN PDEVICE_OBJECT DeviceObject)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;
 PJOYTIMING			OldTiming;
 PJOYTIMING			NewTiming;

 PAGED_CODE();

 DeviceExtension= GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

 if (!PPJOY_ComputeTimingSize(DeviceExtension->Config.JoyType))
 {
  PPJOY_DBGPRINT (FILE_TIMING|PPJOY_BABBLE2, ("Joystick timing not required for this device") );
  return STATUS_SUCCESS;
 }

 PPJOY_DBGPRINT (FILE_TIMING|PPJOY_BABBLE2, ("Attempting to read joystick timing from device registry key") );
 NewTiming= PPJoy_ReadDeviceTiming(DeviceObject,DeviceExtension->Config.JoyType);
 if (NewTiming)
 {
  PPJOY_DBGPRINT (FILE_TIMING|PPJOY_BABBLE, ("Joystick timing successfully read from the device registry") );
  goto UpdateTiming;
 }

 PPJOY_DBGPRINT (FILE_TIMING|PPJOY_BABBLE2, ("Attempting to read joystick timing from driver registry key") );
 NewTiming= PPJoy_ReadDriverTiming(DeviceObject,DeviceExtension->Config.JoyType);
 if (NewTiming)
 {
  PPJOY_DBGPRINT (FILE_TIMING|PPJOY_BABBLE, ("Joystick timing successfully read from the driver registry") );
  goto UpdateTiming;
 }
    
 PPJOY_DBGPRINT (FILE_TIMING|PPJOY_BABBLE2, ("Building default joystick timing for device") );
 NewTiming= PPJoy_BuildDefaultTiming(DeviceExtension->Config.JoyType);
 if (NewTiming)
 {
  PPJOY_DBGPRINT (FILE_TIMING|PPJOY_BABBLE, ("Joystick timing successfully built manually") );
  goto UpdateTiming;
 }

 return STATUS_UNSUCCESSFUL;

UpdateTiming:

 OldTiming= DeviceExtension->Timing;
 DeviceExtension->Timing= NewTiming;

 PPJoy_InitTimingValues (DeviceObject);

 if (OldTiming)
  ExFreePool(OldTiming);

 return STATUS_SUCCESS;
}
