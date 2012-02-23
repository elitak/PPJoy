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


#ifndef	__CONFIGMAN_H__
#define	__CONFIGMAN_H__

#include <basetyps.h>
#include <wtypes.h>

void MakeDeviceID (char *DeviceID, USHORT VendorID, USHORT ProductID);

int DeleteDeviceID (char *DeviceID, int Recursive);
int SetFriendlyName (char *DeviceID, char *FriendlyName);
int RestartDevice (char *DeviceID);

int FindDeviceID (char *HardwareID, char *DeviceID, int	DevIDSize);

int GetDeviceIRQ (char *DeviceID);

int GetInterruptAssignment(char *DeviceName);
int GetConnectInterruptSetting(char *DeviceName);
int SetConnectInterruptSetting(char *DeviceName, DWORD Connect);

#endif
