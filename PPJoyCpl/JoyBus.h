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


#ifndef __JOYBUS_H__
#define __JOYBUS_H__

#include <TCHAR.h>

#include "PPJIoctl.h"
#include "PPJType.h"

#define	ENUM_ALLOC_SIZE		(sizeof(JOYSTICK_ENUM_DATA))
/* Define the maximum Joystick Type number - that we know of. */
#define	PPJOY_MAX_IFTYPE	IF_GENESISDPP6
#define PPJOY_MAX_DEVTYPE	DEVTYPE_RADIOTX

#define	MAX_PARPORTS		16
#define	MAX_JOYUNITS		16

extern PJOYSTICK_ENUM_DATA		pEnumData;

UCHAR GetDevTypeFromJoyType(UCHAR JoyType);
TCHAR *GetDevTypeName(UCHAR DevType);

TCHAR *GetJoyTypeName (UCHAR JoyType);
UCHAR GetJoyTypeMaxUnits (UCHAR JoyType);

int OpenPPJoyBusDriver (void);
void ClosePPJoyBusDriver (void);

int RescanJoysticks (void);
int RescanParallelPorts (ULONG *PortArray, UCHAR ArraySize);

// LPTNumber starts at 1 for LPT1.
int DeleteJoystick (UCHAR LPTNumber, UCHAR JoyType, UCHAR UnitNumber);
int AddJoystick (UCHAR LPTNumber, UCHAR JoyType, UCHAR UnitNumber, UCHAR JoySubType, USHORT VendorID, USHORT ProductID);

ULONG GetLptNumber (TCHAR *LptName);

int SetPPJoyOption (ULONG Option, ULONG Value);
int GetPPJoyOption (ULONG Option, PULONG Value);

#endif
