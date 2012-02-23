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


#ifndef __PPJOYBUS50_H__
#define __PPJOYBUS50_H__

typedef struct
{
 unsigned long	Port;
 union
 {
  struct
  {
   unsigned char	JoyType;
   unsigned char	SubAddr;
   unsigned char	NumAxes;
   unsigned char	NumButtons;
  };
  unsigned long	TypeAsLong;
 };
} JOYSTICK_DATA50, *PJOYSTICK_DATA50;

typedef struct
{
 JOYSTICK_DATA50	JoyData;
} JOYSTICK_DEL_DATA50, *PJOYSTICK_DEL_DATA50;

typedef struct
{
 unsigned long		Size;
 unsigned long		Count;
 JOYSTICK_DATA50	Joys[1];
} JOYSTICK_ENUM_DATA50, *PJOYSTICK_ENUM_DATA50;

HANDLE OpenPPJoyBusDriver50 (void);
void DelJoystick50 (HANDLE Driver, unsigned int Port, unsigned int TypeAsLong);
void EnumJoystick50 (HANDLE Driver, PJOYSTICK_ENUM_DATA50 *EnumData);

#endif
