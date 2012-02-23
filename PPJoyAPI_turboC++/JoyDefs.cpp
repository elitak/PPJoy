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

#include <stdio.h>

#include <basetyps.h>
#include <wtypes.h>

#include "Debug.h"
#include "JoyDefs.h"

SUBTYPE_DATA	SubTypeTable[]=
				{
				{DEVTYPE_GENESIS,SUBTYPE_GENESYS3,SUBTYPE_GENESYS3_NAME},
				{DEVTYPE_GENESIS,SUBTYPE_GENESYS6,SUBTYPE_GENESYS6_NAME},
				{DEVTYPE_SNESPAD,SUBTYPE_SNES_SNES,SUBTYPE_SNES_SNES_NAME},
				{DEVTYPE_SNESPAD,SUBTYPE_SNES_NES,SUBTYPE_SNES_NES_NAME},
				{0,0,NULL}
				};

#include "JoyBus.h"		/* until we move all definition functions here */

char* GetSubTypeName (UCHAR DevType, UCHAR JoySubType)
{
 SUBTYPE_DATA	*Table;

 /* First iterate through the table and see if we have a specific subtype name */
 Table= SubTypeTable;
 while (Table->DevType)
 {
  if ((Table->DevType==DevType)&&(Table->SubType==JoySubType))
   return Table->Name;
  Table++;
 }

 /* If not, if the subtype is 0 return "" - means default type */
 if (!JoySubType)
  return "";

 /* OK we don't know what the answer is */
 return "unknown subtype";
}

void PrintJoystickLongName (char *Buffer, UCHAR LPTNumber, UCHAR JoyType, UCHAR UnitNumber, UCHAR JoySubType)
{
 UCHAR		DevType;
 char		*SubTypeName;

 if (!LPTNumber)
 {
  sprintf (Buffer,"PPJoy Virtual joystick %d",UnitNumber+1);
  return;
 }

 DevType= GetDevTypeFromJoyType(JoyType);
 SubTypeName= GetSubTypeName (DevType,JoySubType);

 if (*SubTypeName)
  sprintf (Buffer,"LPT%d: %s %s %d (%s)",LPTNumber,GetJoyTypeName(JoyType),GetDevTypeName(DevType),UnitNumber+1,SubTypeName);
 else
  sprintf (Buffer,"LPT%d: %s %s %d",LPTNumber,GetJoyTypeName(JoyType),GetDevTypeName(DevType),UnitNumber+1);
}

void PrintJoystickShortName (char *Buffer, UCHAR LPTNumber, UCHAR JoyType, UCHAR UnitNumber, UCHAR JoySubType)
{
 UCHAR		DevType;

 if (!LPTNumber)
 {
  sprintf (Buffer,"PPJoy Virtual joystick %d",UnitNumber+1);
  return;
 }

 DevType= GetDevTypeFromJoyType(JoyType);
 sprintf (Buffer,"LPT%d: %s %s %d",LPTNumber,GetJoyTypeName(JoyType),GetDevTypeName(DevType),UnitNumber+1);
}

