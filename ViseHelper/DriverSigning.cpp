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

#include "DriverSigning.h"

#include "Debug.h"

/* Check the status of driver signing. Will return the following      */
/* values: 1: install allowed or 2: install not allowed or error      */
int	CheckSigningStatus (void)
{
 int	rc;
 HKEY	hKey;
 DWORD	ValueType;
 DWORD	ValueData;
 DWORD	DataSize;

 DebugPrintf (("Checking to see if unsigned drivers can be installed.\n"))

 rc= RegOpenKeyEx (HKEY_CURRENT_USER,"Software\\Microsoft\\Driver Signing",0,KEY_READ,&hKey);
 if (rc!=ERROR_SUCCESS)
 {
  DebugPrintf (("RegOpenKeyEx (HKCU\\Software\\Microsoft\\Driver Signing) failed %d\n",rc))
  if (rc==2) /* Not found */
   goto CheckLocalMachine;

  return 2; /* Fatal error reading data */
 }
 
 DataSize= sizeof (ValueData);
 rc= RegQueryValueEx (hKey,"Policy",NULL,&ValueType,(LPBYTE)&ValueData,&DataSize);
 RegCloseKey (hKey);

 if (rc!=ERROR_SUCCESS)
 {
  DebugPrintf (("RegQueryValueEx (Policy) failed %d\n",rc))
  if (rc==2) /* Not found */
   goto CheckLocalMachine;

  return 2; /* Fatal error reading data */
 }

 if (ValueType!=REG_DWORD_LITTLE_ENDIAN)
 {
  DebugPrintf (("Unknown value type %d\n",ValueType))
  return 2; /* Fatal error reading data */
 }

 DebugPrintf (("Value for HKCU\\Software\\Microsoft\\Driver Signing\\Policy is %d\n",ValueData))

 if ((ValueData==0)||(ValueData==1))
  return 1;	/* Allowed */
 return 2; /* Driver signing policy not Warn or Ignore */

CheckLocalMachine:
 rc= RegOpenKeyEx (HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Driver Signing",0,KEY_READ,&hKey);
 if (rc!=ERROR_SUCCESS)
 {
  DebugPrintf (("RegOpenKeyEx (HKLM\\Software\\Microsoft\\Driver Signing) failed %d\n",rc))
  if (rc==2) /* Not found */
   return 1; /* No driver signing information - allowed */

  return 2; /* Fatal error reading data */
 }
 
 DataSize= sizeof (ValueData);
 rc= RegQueryValueEx (hKey,"Policy",NULL,&ValueType,(LPBYTE)&ValueData,&DataSize);
 RegCloseKey (hKey);

 if (rc!=ERROR_SUCCESS)
 {
  DebugPrintf (("RegQueryValueEx (Policy) failed %d\n",rc))
  if (rc==2) /* Not found */
   return 1; /* No driver signing information - allowed */

  return 2; /* Fatal error reading data */
 }

 if (ValueType!=REG_BINARY)
 {
  DebugPrintf (("Unknown value type %d\n",ValueType))
  return 2; /* Fatal error reading data */
 }

 ValueData&= 0xFF;
 DebugPrintf (("Value for HKLM\\Software\\Microsoft\\Driver Signing\\Policy is %d\n",ValueData))

 if ((ValueData==0)||(ValueData==1))
  return 1;	/* Allowed */
 return 2; /* Driver signing policy not Warn or Ignore */
}
