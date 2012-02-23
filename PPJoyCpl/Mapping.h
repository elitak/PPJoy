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


#ifndef	__MAPPING_H__
#define	__MAPPING_H__

#include <windows.h>
#include <hidusage.h>
#include <TCHAR.h>

#include <PPJIoctl.h>

#define	SCANTYPE_AXISMIN	1
#define	SCANTYPE_AXISMAX	2
#define	SCANTYPE_BUTTON		3
#define	SCANTYPE_HATDIAL	4

#define	ANALOG_MIN_TRIP		((PPJOY_AXIS_MAX+PPJOY_AXIS_MIN)/2-(PPJOY_AXIS_MAX-PPJOY_AXIS_MIN)/2/2)
#define	ANALOG_MAX_TRIP		((PPJOY_AXIS_MAX+PPJOY_AXIS_MIN)/2+(PPJOY_AXIS_MAX-PPJOY_AXIS_MIN)/2/2)

struct HIDUSAGE
{
 BYTE	Page;
 BYTE	Usage;
};

struct SAxisMapRec
{
 BYTE	SourceMin;
 BYTE	SourceMax;
};

struct SButtonMapRec
{
 BYTE	Source;
};

struct SPOVHatMapRec
{
 BYTE	SourceU;
 BYTE	SourceD;
 BYTE	SourceL;
 BYTE	SourceR;
};

struct SAxisDefines
{
 BYTE	Page;
 BYTE	Usage;
 TCHAR	*Description;
};

extern SAxisDefines	AxisUsageList[];

HANDLE OpenJoystickDevice (int Port, int SubAddr);

int	ReadJoystickMappings (int Port, int JoyType, int SubAddr, JOYSTICK_MAP **DefaultMapping, JOYSTICK_MAP **DriverMapping, JOYSTICK_MAP **DeviceMapping);
int	WriteJoystickMapping (int Port, int JoyType, int SubAddr, JOYSTICK_MAP *Mapping, int MapSize, int Scope);
int DeleteJoystickMapping (int Port, int JoyType, int SubAddr, int Scope);

int	ScanJoystickInput (HANDLE hJoy, int ScanType);

int	ReadJoystickTimings (int Port, int JoyType, int SubAddr, PJOYTIMING *DefaultTiming, PJOYTIMING *DriverTiming, PJOYTIMING *DeviceTiming);
int	WriteJoystickTiming (int Port, int JoyType, int SubAddr, PJOYTIMING Timing, int TimingSize, int Scope);
int DeleteJoystickTiming (int Port, int JoyType, int SubAddr, int Scope);
int ActivateJoystickTiming (int Port, int JoyType, int SubAddr);

#endif
