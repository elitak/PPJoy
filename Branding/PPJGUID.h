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


// Define an Interface Guid to access the proprietary PPJoyBus interface.
// This guid is used to identify a specific interface in IRP_MN_QUERY_INTERFACE handler.

DEFINE_GUID(GUID_PPJOY_BUS,0x64c3b4c4,0xcdcc,0x49aa,0x99,0xa0,0x5b,0x4a,0xe4,0xb5,0xb1,0xbb);
/* 64c3b4c4-cdcc-49aa-99a0-5b4ae4b5b1bb */
