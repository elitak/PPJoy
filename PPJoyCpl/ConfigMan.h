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

#include <TCHAR.h>

#include <basetyps.h>
#include <wtypes.h>

void MakeDeviceID (TCHAR *DeviceID, USHORT VendorID, USHORT ProductID);

int DeleteDeviceID (TCHAR *DeviceID, int Recursive);
int SetFriendlyName (TCHAR *DeviceID, TCHAR *FriendlyName);
int RestartDevice (TCHAR *DeviceID);

int FindDeviceID (TCHAR *HardwareID, TCHAR *DeviceID, int DevIDSize);

int GetDeviceIRQ (TCHAR *DeviceID);

int GetInterruptAssignment(TCHAR *DeviceName);
int GetConnectInterruptSetting(TCHAR *DeviceName);
int SetConnectInterruptSetting(TCHAR *DeviceName, DWORD Connect);

void DeleteDevice (void);

#endif
