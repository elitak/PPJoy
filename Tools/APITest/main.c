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


#include <PPJoyAPI.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define	MAX_JOYSTICKS	16		// DirectX can only handle 16 joysticks
								// PPJoy thus inherits this limitation

int ListJoysticks (void)
{
 JOYENTRY	JoystickList[MAX_JOYSTICKS];
 int		NumJoysticks;
 int		Count;

 NumJoysticks= PPJoy_GetJoystickList (JoystickList,MAX_JOYSTICKS);
 if (NumJoysticks==-1)
 {
  printf ("Error reading joystick list\n");
  return 1;
 }
 printf ("There are %d joysticks installed:\n",NumJoysticks);
 for (Count=0;Count<NumJoysticks;Count++)
 {
  printf ("%02d. %s%s on Port %d Unit %d\n",Count+1,PPJoy_GetJoyTypeName(JoystickList[Count].JoyType),
		  PPJoy_GetSubTypeName(JoystickList[Count].JoyType,JoystickList[Count].JoySubType),
		  JoystickList[Count].LPTNumber,JoystickList[Count].UnitNumber);
 }
 return 0;
}

int AddJoystick (char LPTNumber, char UnitNumber, char JoyType, char JoySubType)
{
 if (!PPJoy_AddJoystick(LPTNumber,UnitNumber,JoyType,JoySubType))
 {
  printf ("Error adding new joystick\n");
  return 1;
 }
 return 0;
}

int DelJoystick (char LPTNumber, char UnitNumber, char JoyType)
{
 if (!PPJoy_DeleteJoystick(LPTNumber,UnitNumber,JoyType))
 {
  printf ("Error removing joystick\n");
  return 1;
 }
 return 0;
}

int ComputeMappingSize (JOYMAPPING *Mapping)
{
 return sizeof(Mapping->NumAxes)+sizeof(Mapping->NumButtons)+
		sizeof(Mapping->NumHats)+sizeof(Mapping->NumMaps)+
		(Mapping->NumAxes+Mapping->NumButtons+Mapping->NumHats)*2 +
		(Mapping->NumAxes*2+Mapping->NumButtons*1+Mapping->NumHats*4)*Mapping->NumMaps;
}

void DumpHexBytes (char *Data, int Count)
{
 while ((Count--)>0)
  printf ("%02X",*((unsigned char*)Data++));
}

int ReadMapping (char LPTNumber, char UnitNumber, char JoyType)
{
 JOYMAPPING	*DefaultMapping;
 JOYMAPPING	*DriverMapping;
 JOYMAPPING	*DeviceMapping;

 if (!PPJoy_ReadMappings(LPTNumber,UnitNumber,JoyType,&DefaultMapping,&DriverMapping,&DeviceMapping))
 {
  printf ("Error reading joystick mappings\n");
  return 1;
 }

 printf ("Default mapping: ");
 if (DefaultMapping)
  DumpHexBytes ((char*)DefaultMapping,ComputeMappingSize(DefaultMapping));
 else
  printf ("not set");
 printf ("\n");

 printf ("Device type mapping: ");
 if (DriverMapping)
  DumpHexBytes ((char*)DriverMapping,ComputeMappingSize(DriverMapping));
 else
  printf ("not set");
 printf ("\n");
 
 printf ("Device specific mapping: ");
 if (DeviceMapping)
  DumpHexBytes ((char*)DeviceMapping,ComputeMappingSize(DeviceMapping));
 else
  printf ("not set");
 printf ("\n");

 if (DefaultMapping) PPJoy_FreeMapping (DefaultMapping);
 if (DriverMapping) PPJoy_FreeMapping (DriverMapping);
 if (DeviceMapping) PPJoy_FreeMapping (DeviceMapping);
 return 0;
}

int DeleteMapping (char LPTNumber, char UnitNumber, char JoyType, int Scope)
{
 if (PPJoy_DeleteMapping(LPTNumber,UnitNumber,JoyType,Scope))
 {
  printf ("Error removing mapping\n");
  return 1;
 }
 return 0;
}

void HexToBytes (char *HexString, char *Bytes)
{
 while ((*HexString)&&(*(HexString+1)))
 {
  sscanf (HexString,"%02X",Bytes++);
  HexString+= 2;
 }
}

int WriteMapping (char LPTNumber, char UnitNumber, char JoyType, int Scope, char *HexBytes)
{
 char	Bytes[2048];
 int	MapSize;

 HexToBytes (HexBytes,Bytes);
 MapSize= ComputeMappingSize((JOYMAPPING*)Bytes);
 if (MapSize!=(strlen(HexBytes)/2))
 {
  printf ("Incomplete (or invalid) mapping specified\n");
  return 0;
 }

 if (PPJoy_WriteMapping(LPTNumber,UnitNumber,JoyType,(JOYMAPPING*)Bytes,MapSize,Scope))
 {
  printf ("Error writing mapping\n");
  return 1;
 }
 return 0;
}

void PrintUsage (char *AppName)
{
 printf ("Usage: %s <cmd> [parameters]\n",AppName);
 printf ("Commands are:\n");
 printf (" listjoy\n");
 printf (" addjoy <LPTNumber> <UnitNumber> <JoyType> <JoySubType>\n");
 printf (" deljoy <LPTNumber> <UnitNumber> <JoyType>\n");
 printf (" readmap <LPTNumber> <UnitNumber> <JoyType>\n");
 printf (" delmap <LPTNumber> <UnitNumber> <JoyType> <Scope>\n");
 printf (" writemap <LPTNumber> <UnitNumber> <JoyType> <Scope> <Bytes>\n");
}

int main (int argc, char **argv)
{
 int	rc;

 if (argc<2)
 {
  PrintUsage(argv[0]);
  return 1;
 }

 if (!PPJoy_OpenDriver())
 {
  printf ("Unable to open the PPJoy device driver. Terminating\n");
  return 1;
 }

 rc= 1;
 if (!_stricmp(argv[1],"listjoy"))
 {
  if (argc!=2)
  {
   printf ("Incorrect number of arguments\n");
   PrintUsage(argv[0]);
  }
  else
   rc= ListJoysticks();
 }
 else if (!_stricmp(argv[1],"addjoy"))
 {
  if (argc!=6)
  {
   printf ("Incorrect number of arguments\n");
   PrintUsage(argv[0]);
  }
  else
   rc= AddJoystick((char)atoi(argv[2]),(char)atoi(argv[3]),(char)atoi(argv[4]),(char)atoi(argv[5]));
 }
 else if (!_stricmp(argv[1],"deljoy"))
 {
  if (argc!=5)
  {
   printf ("Incorrect number of arguments\n");
   PrintUsage(argv[0]);
  }
  else
   rc= DelJoystick((char)atoi(argv[2]),(char)atoi(argv[3]),(char)atoi(argv[4]));
 }
 else if (!_stricmp(argv[1],"readmap"))
 {
  if (argc!=5)
  {
   printf ("Incorrect number of arguments\n");
   PrintUsage(argv[0]);
  }
  else
   rc= ReadMapping((char)atoi(argv[2]),(char)atoi(argv[3]),(char)atoi(argv[4]));
 }
 else if (!_stricmp(argv[1],"delmap"))
 {
  if (argc!=6)
  {
   printf ("Incorrect number of arguments\n");
   PrintUsage(argv[0]);
  }
  else
   rc= DeleteMapping((char)atoi(argv[2]),(char)atoi(argv[3]),(char)atoi(argv[4]),(char)atoi(argv[5]));
 }
 else if (!_stricmp(argv[1],"writemap"))
 {
  if (argc!=7)
  {
   printf ("Incorrect number of arguments\n");
   PrintUsage(argv[0]);
  }
  else
   rc= WriteMapping((char)atoi(argv[2]),(char)atoi(argv[3]),(char)atoi(argv[4]),(char)atoi(argv[5]),argv[6]);
 }
 else
 {
  printf ("Unknown command!\n");
  PrintUsage(argv[0]);
 }
 
 PPJoy_CloseDriver();
 return rc;
}