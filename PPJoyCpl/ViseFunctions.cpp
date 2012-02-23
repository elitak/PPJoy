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

#include "ViseFunctions.h"
#include "ConfigMan.h"

#include "Debug.h"

#include <branding.h>

/////////////////////////////////////////////////////////////////////////////
// External entry point for VISE Uninstaller.
// Function 0: Remove the joystick FDO and bus enumerator PDO devices
// Function 1: Return number of installed parallel ports

LONG CALLBACK ViseEntry(LONG lParam1, LPCSTR lpParam2, DWORD dwReserved)
{
 // Must be included here???
 AFX_MANAGE_STATE(AfxGetStaticModuleState());

 switch (lParam1)
 {
  case 0:
	{
     DeleteDevice();
	 return 1;
	}
	// Function 1 never implemented? (Or was it removed?)
 }
 return -9999;
}
