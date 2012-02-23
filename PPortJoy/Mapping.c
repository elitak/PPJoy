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
#include "hidusage.h"
#include "JoyDefs.h"
#include <wchar.h>

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE,PPJOY_ComputeMappingSize)
    #pragma alloc_text (PAGE,PPJoy_OpenDeviceRegKey)
    #pragma alloc_text (PAGE,PPJoy_OpenDriverRegKey)
    #pragma alloc_text (PAGE,PPJoy_DeleteMappingFromReg)
    #pragma alloc_text (PAGE,PPJoy_DeleteDeviceMapping)
    #pragma alloc_text (PAGE,PPJoy_DeleteDriverMapping)
    #pragma alloc_text (PAGE,PPJoy_WriteMappingToReg)
    #pragma alloc_text (PAGE,PPJoy_WriteDeviceMapping)
    #pragma alloc_text (PAGE,PPJoy_WriteDriverMapping)
    #pragma alloc_text (PAGE,PPJoy_ReadMappingFromReg)
    #pragma alloc_text (PAGE,PPJoy_ReadDeviceMapping)
    #pragma alloc_text (PAGE,PPJoy_ReadDriverMapping)
    #pragma alloc_text (PAGE,PPJoy_BuildDefaultMapping)
    #pragma alloc_text (PAGE,PPJoy_LoadJoystickMapping)
#endif

ULONG PPJOY_ComputeMappingSize (PJOYSTICK_MAP Mapping)
{

 PAGED_CODE();

 return sizeof(Mapping->NumAxes)+sizeof(Mapping->NumButtons)+
		sizeof(Mapping->NumHats)+sizeof(Mapping->NumMaps)+
		(Mapping->NumAxes+Mapping->NumButtons+Mapping->NumHats)*2 +
		(Mapping->NumAxes*2+Mapping->NumButtons*1+Mapping->NumHats*4)*Mapping->NumMaps;
}

NTSTATUS PPJoy_DeleteMappingFromReg (UCHAR JoyType, HANDLE RegKey)
{
 NTSTATUS			ntStatus;
 UNICODE_STRING		ValueName;
 WCHAR				ValueNameBuffer[128];

 PAGED_CODE();

 swprintf (ValueNameBuffer,L"MapV1_%02X",JoyType);
 RtlInitUnicodeString(&ValueName,ValueNameBuffer);

 ntStatus= ZwSetValueKey (RegKey,&ValueName,0,REG_BINARY,ValueNameBuffer,0);
 return ntStatus;
}


NTSTATUS PPJoy_DeleteDeviceMapping (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType)
{
 HANDLE		RegKey;
 NTSTATUS	ntStatus;

 RegKey= PPJoy_OpenDeviceRegKey(DeviceObject);
 if (!RegKey)
  return STATUS_UNSUCCESSFUL;

 ntStatus= PPJoy_DeleteMappingFromReg (JoyType,RegKey);
 ZwClose(RegKey);

 return ntStatus;
}

NTSTATUS PPJoy_DeleteDriverMapping (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType)
{
 HANDLE		RegKey;
 NTSTATUS	ntStatus;

 RegKey= PPJoy_OpenDriverRegKey();
 if (!RegKey)
  return STATUS_UNSUCCESSFUL;

 ntStatus= PPJoy_DeleteMappingFromReg (JoyType,RegKey);
 ZwClose(RegKey);

 return ntStatus;
}

NTSTATUS PPJoy_WriteMappingToReg (PJOYSTICK_MAP Mapping, UCHAR JoyType, HANDLE RegKey)
{
 NTSTATUS			ntStatus;
 UNICODE_STRING		ValueName;
 WCHAR				ValueNameBuffer[128];
 ULONG				MapSize;

 PAGED_CODE();

 swprintf (ValueNameBuffer,L"MapV1_%02X",JoyType);
 RtlInitUnicodeString(&ValueName,ValueNameBuffer);

 MapSize= PPJOY_ComputeMappingSize(Mapping);	 
 
 ntStatus= ZwSetValueKey (RegKey,&ValueName,0,REG_BINARY,Mapping,MapSize);
 return ntStatus;
}

NTSTATUS PPJoy_WriteDeviceMapping (IN PDEVICE_OBJECT DeviceObject, PJOYSTICK_MAP Mapping, UCHAR JoyType)
{
 HANDLE		RegKey;
 NTSTATUS	ntStatus;

 RegKey= PPJoy_OpenDeviceRegKey(DeviceObject);
 if (!RegKey)
  return STATUS_UNSUCCESSFUL;

 ntStatus= PPJoy_WriteMappingToReg (Mapping,JoyType,RegKey);
 ZwClose(RegKey);

 return ntStatus;
}

NTSTATUS PPJoy_WriteDriverMapping (IN PDEVICE_OBJECT DeviceObject, PJOYSTICK_MAP Mapping, UCHAR JoyType)
{
 HANDLE		RegKey;
 NTSTATUS	ntStatus;

 RegKey= PPJoy_OpenDriverRegKey();
 if (!RegKey)
  return STATUS_UNSUCCESSFUL;

 ntStatus= PPJoy_WriteMappingToReg (Mapping,JoyType,RegKey);
 ZwClose(RegKey);

 return ntStatus;
}

PJOYSTICK_MAP PPJoy_ReadMappingFromReg (UCHAR JoyType, HANDLE RegKey)
{
 NTSTATUS			ntStatus;
 ULONG				NumRead;
 UNICODE_STRING		ValueName;
 WCHAR				ValueNameBuffer[128];
 ULONG				MapSize;
 UCHAR				Buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+1024];
 PJOYSTICK_MAP		Mapping;

 PAGED_CODE();

 Mapping= NULL;

 swprintf (ValueNameBuffer,L"MapV1_%02X",JoyType);
 RtlInitUnicodeString(&ValueName,ValueNameBuffer);

 NumRead= 0;
 ntStatus= ZwQueryValueKey(RegKey,&ValueName,KeyValuePartialInformation,Buffer,sizeof(Buffer),&NumRead);
 if(!NT_SUCCESS(ntStatus))
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_WARN, ("Error 0x%X reading value %S. NumRead= %d",ntStatus,ValueName.Buffer,NumRead) );
  goto Exit;
 }

 if (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Type!=REG_BINARY)
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_WARN, ("Wrong registry datatype %d reading value %S",((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Type,ValueName.Buffer) );
  goto Exit;
 }

 if (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength==0)
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE, ("Empty registry key, ignoring") );
  goto Exit;
 }

 MapSize= (ULONG)-1;
 if (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength>4)
  MapSize= PPJOY_ComputeMappingSize((PJOYSTICK_MAP)(((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data));

 if (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength!=MapSize)
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_WARN, ("Invalid registry value size. Value size= %d expected %d",((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength,MapSize) );
  goto Exit;
 }

 Mapping= ExAllocatePoolWithTag (NonPagedPool,MapSize,PPJOY_POOL_TAG);
 if (!Mapping)
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_ERROR, ("Cannot allocate nonpaged pool to store joystick mapping. Help!") );
  goto Exit;
 }

 RtlCopyMemory (Mapping,((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data,MapSize);

Exit:
 return Mapping;
}

PJOYSTICK_MAP PPJoy_ReadDeviceMapping (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType)
{
 HANDLE			RegKey;
 PJOYSTICK_MAP	Mapping;

 RegKey= PPJoy_OpenDeviceRegKey(DeviceObject);
 if (!RegKey)
  return NULL;

 Mapping= PPJoy_ReadMappingFromReg(JoyType,RegKey);
 ZwClose(RegKey);

 return Mapping;
}

PJOYSTICK_MAP PPJoy_ReadDriverMapping (IN PDEVICE_OBJECT DeviceObject, UCHAR JoyType)
{
 HANDLE			RegKey;
 PJOYSTICK_MAP	Mapping;

 RegKey= PPJoy_OpenDriverRegKey();
 if (!RegKey)
  return NULL;

 Mapping= PPJoy_ReadMappingFromReg(JoyType,RegKey);
 ZwClose(RegKey);

 return Mapping;
}

PJOYSTICK_MAP PPJoy_BuildDefaultMapping (UCHAR JoyType)
{
 NTSTATUS		ntStatus;
 PJOYSTICK_MAP	Mapping;
 PJOYSTICK_MAP	SrcMapping;
 int			MapSize;

 PAGED_CODE();

 Mapping= NULL;

 if (JoyType>PPJOY_MAX_JOYTYPE)
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_WARN, ("Warning, mapping request for invalid joystick type 0x%02X!",JoyType) );
  goto Exit;
 }

 SrcMapping= JoyDefs[JoyType].DefaultMap;
 if (!SrcMapping)
  goto Exit;
 
 MapSize= PPJOY_ComputeMappingSize(SrcMapping);
 Mapping= ExAllocatePoolWithTag (NonPagedPool,MapSize,PPJOY_POOL_TAG);
 if (!Mapping)
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_ERROR, ("Cannot allocate nonpaged pool to store joystick mapping. Help!") );
  goto Exit;
 }

 RtlCopyMemory (Mapping,SrcMapping,MapSize);

Exit:
 return Mapping;
}

NTSTATUS PPJoy_LoadJoystickMapping (IN PDEVICE_OBJECT DeviceObject)
{
 NTSTATUS			ntStatus;
 PDEVICE_EXTENSION	DeviceExtension;

 PAGED_CODE();

 DeviceExtension= GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

 if (DeviceExtension->Mapping)
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_WARN, ("JoystickMapping already set - attempting to free memory") );
  ExFreePool(DeviceExtension->Mapping);
  DeviceExtension->Mapping= NULL;
 }
	 
 PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE2, ("Attempting to read joystick mapping from device registry key") );
 DeviceExtension->Mapping= PPJoy_ReadDeviceMapping(DeviceObject,DeviceExtension->Config.JoyType);
 if(DeviceExtension->Mapping)
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE, ("Joystick mapping successfully read from the device registry") );
  return STATUS_SUCCESS;
 }

 PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE2, ("Attempting to read joystick mapping from driver registry key") );
 DeviceExtension->Mapping= PPJoy_ReadDriverMapping(DeviceObject,DeviceExtension->Config.JoyType);
 if(DeviceExtension->Mapping)
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE, ("Joystick mapping successfully read from the driver registry") );
  return STATUS_SUCCESS;
 }
    
 PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE2, ("Building default joystick mapping for device") );
 DeviceExtension->Mapping= PPJoy_BuildDefaultMapping(DeviceExtension->Config.JoyType);
 if(DeviceExtension->Mapping)
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE, ("Joystick mapping successfully built manually") );
  return STATUS_SUCCESS;
 }

 return STATUS_UNSUCCESSFUL;
}

/* Return values from the raw input buffer pointed to by RawInput */
#define	ANALOG_RAW(INDEX)	(RawInput->Analog[(INDEX)&0x3F])
#define	DIGITAL_RAW(INDEX)	(RawInput->Digital[(INDEX)&0x7F])

#define	MAP_ANALOG_SOURCE(SRC,SRC2)															\
	( ((SRC)==UNUSED_MAPPING)?(PPJOY_AXIS_CENTRE):											\
	  ( ((SRC)&0x80)?( ((SRC)&0x40)?(PPJOY_AXIS_REV(ANALOG_RAW(SRC))):(ANALOG_RAW(SRC)) ):	\
	    ( (PPJOY_AXIS_CENTRE)+(DIGITAL_RAW(SRC2)-DIGITAL_RAW(SRC))*(PPJOY_AXIS_HALFSWING) )\
	  )																						\
	)
#define	MAP_DIGTAL_SOURCE(SRC)																\
	( ((SRC)==UNUSED_MAPPING)?0:															\
	  ( ((SRC)&0x80)?( ((SRC)&0x40)?( ((ANALOG_RAW(SRC))>(PPJOY_AXIS_HITRIP))?1:0 ):		\
									( ((ANALOG_RAW(SRC))<(PPJOY_AXIS_LOTRIP))?1:0 )			\
					 ):																		\
	    (DIGITAL_RAW(SRC))																	\
	  )																						\
	)

ULONG POVDirLookup[16]=	{PPJOY_HAT_NUL,				/* ---- */
						 PPJOY_HAT_90DEG,			/* ---R */
						 PPJOY_HAT_270DEG,			/* --L- */
						 PPJOY_HAT_NUL,				/* --LR */
						 PPJOY_HAT_180DEG,			/* -D-- */
						 PPJOY_HAT_135DEG,			/* -D-R */
						 PPJOY_HAT_225DEG,			/* -DL- */
						 PPJOY_HAT_180DEG,			/* -DLR */
						 PPJOY_HAT_0DEG,			/* U--- */
						 PPJOY_HAT_45DEG,			/* U--R */
						 PPJOY_HAT_315DEG,			/* U-L- */
						 PPJOY_HAT_0DEG,			/* U-LR */
						 PPJOY_HAT_NUL,				/* UD-- */
						 PPJOY_HAT_90DEG,			/* UD-R */
						 PPJOY_HAT_270DEG,			/* UDL- */
						 PPJOY_HAT_NUL};			/* UDLR */

void PPJoy_MapRawToReport (PHIDREPORT Report, PRAWINPUT RawInput, PJOYSTICK_MAP Mapping, UCHAR MapIndex)
{
 UCHAR	NumAxes;
 UCHAR	NumButtons;
 UCHAR	NumHats;
 UCHAR	NumMaps;

 int	Count;
 UCHAR	*Next;

 UCHAR	Source1;
 UCHAR	Source2;
 UCHAR	Source3;
 UCHAR	Source4;

/// ULONG	Axis;
 ULONG	POVHat;
 UCHAR	HatU;
 UCHAR	HatD;
 UCHAR	HatL;
 UCHAR	HatR;
 UCHAR	HatIndex;

 NumAxes= Mapping->NumAxes;
 NumButtons= Mapping->NumButtons;
 NumHats= Mapping->NumHats;
 NumMaps= Mapping->NumMaps;

 PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE2, ("Performing joystick mapping axes=%d buttons=%d hats=%d Report=0x%p RawInput=0x%p Mapping=0x%p",NumAxes,NumButtons,NumHats,Report,RawInput,Mapping) );

 if (MapIndex>=NumMaps)
 {
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_WARN, ("Not performing mapping. Device in map mode %d, only %d maps in mapping",MapIndex+1,NumMaps) );
  return;
 }

 /* Get start of first joystick mapping record */
 Next= Mapping->Data;
 Next+= (NumAxes+NumButtons+NumHats)*2;

 /* Skip to start of active config record */
 Next+= MapIndex*(NumAxes*2+NumButtons*1+NumHats*4);

 /* Loop through all the axis definitions */
 for (Count=0;Count<NumAxes;Count++)
 {
  Source1= *(Next++);
  Source2= *(Next++);

  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE2, ("Axis %d definition Source1=0x%02X Source2=0x%02X",Count,Source1,Source2) );
  Report->Axes[Count]= MAP_ANALOG_SOURCE(Source1,Source2);

  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE2, ("Report->Axes[Count] set to 0x%08X",Report->Axes[Count]) );
 }

 /* Loop through all the button definitions */
 for (Count=0;Count<NumButtons;Count++)
 {
  Source1= *(Next++);

  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE2, ("Button %d definition Source1=0x%02X",Count,Source1) );
  Report->Buttons[Count]= MAP_DIGTAL_SOURCE(Source1);

  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE2, ("Report->Buttons[Count] set to 0x%02X",Report->POVHats[Count]) );
 }

 /* Loop through all the hat definitions */
 for (Count=0;Count<NumHats;Count++)
 {
  Source1= *(Next++);
  Source2= *(Next++);
  Source3= *(Next++);
  Source4= *(Next++);

  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE2, ("Hat %d definition Source1=0x%02X Source2=0x%02X Source3=0x%02X Source4=0x%02X",Count,Source1,Source2,Source3,Source4) );

  if (Source1==UNUSED_MAPPING)									/* No hat source mapped */
  {
   POVHat= PPJOY_HAT_NUL;										/* NULL value */
  }
  else if ((Source1&0x80)&&(Source2==UNUSED_MAPPING))			/* Map hat using single analog axis */
  {
   POVHat= PPJOY_AXISTOHAT(MAP_ANALOG_SOURCE(Source1,Source2));
  }
  else															/* Map hat using four directions (analog or digital) */
  {
   HatU= MAP_DIGTAL_SOURCE(Source1);
   HatD= MAP_DIGTAL_SOURCE(Source2);
   HatL= MAP_DIGTAL_SOURCE(Source3);
   HatR= MAP_DIGTAL_SOURCE(Source4);

   HatIndex= (HatU<<3)+(HatD<<2)+(HatL<<1)+HatR;
   POVHat= POVDirLookup[HatIndex];
  }

  Report->POVHats[Count]= POVHat;
  PPJOY_DBGPRINT (FILE_MAPPING|PPJOY_BABBLE2, ("Report->POVHats[Count] set to 0x%08X",Report->POVHats[Count]) );
 }
}
