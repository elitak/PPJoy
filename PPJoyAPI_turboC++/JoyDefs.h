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


#ifndef __JOYDEFS_H__
#define __JOYDEFS_H__

#include "PPJType.h"

struct SUBTYPE_DATA
{
 UCHAR	DevType;
 UCHAR	SubType;
 char	*Name;
};

extern SUBTYPE_DATA	SubTypeTable[];

char* GetSubTypeName (UCHAR DevType, UCHAR JoySubType);
void PrintJoystickLongName (char *Buffer, UCHAR LPTNumber, UCHAR JoyType, UCHAR UnitNumber, UCHAR JoySubType);
void PrintJoystickShortName (char *Buffer, UCHAR LPTNumber, UCHAR JoyType, UCHAR UnitNumber, UCHAR JoySubType);

#endif
