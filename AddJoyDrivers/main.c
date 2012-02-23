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


#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "Install.h"
#include "Debug.h"
#include "UtilFunc.h"

#include <branding.h>

// For Nullsoft installer plugins - include plugin functions and defines.
#include <pluginapi.c>
// Wrapper to declare a parameter passed on the nsis plugin stack. Pass the
// variable name to declare as parameter. All vars will be 1024 characters,
// same as nsis default. Param is popped during declaration!!!
#define	DECLARE_CHAR_PARAM(PARNAME)	char PARNAME[1024]; if (popstringn(PARNAME,sizeof(PARNAME))) *PARNAME=0;
#define	DECLARE_INT_PARAM(PARNAME)	int PARNAME= popint();
#define	SAVE_INT_RESULT(RESULTEXPR)	{ char TempResultStr[33]; setuservariable (INST_0,_itoa((RESULTEXPR),TempResultStr,10)); }
#define	SAVE_CHAR_RESULT(RESULTEXPR)	{ setuservariable (INST_0,(RESULTEXPR)); }
#define	DECLARE_NSIS_FUNC(NSISFUNC)	void __cdecl NSISFUNC (HWND hwndParent, int string_size, char *variables, stack_t **stacktop, void *extra)


/* Copy a file to the Windows INF directory. Used to copy INF files   */
/* on Windows 98 where we don't have the other install routines.      */
int CopyFileToWindowsInfDirectory (char *SourcePath)
{
 char					Buffer[1024];
 char					*INFName;

 if (!GetWindowsDirectory(Buffer,sizeof(Buffer)))
 {
  DebugPrintf (("GetWindowsDirectory error %d\n",GetLastError()))
  return 0;
 }
 strcat (Buffer,"\\INF\\");
 INFName= strrchr (SourcePath,'\\');
 if (!INFName)
 {
  DebugPrintf (("Cannot determine source .INF name\n"))
  return 0;
 }
 strcat (Buffer,INFName+1);
 
 DebugPrintf (("Copying INF File from %s to %s\n",SourcePath,Buffer))
 if (!CopyFile (SourcePath,Buffer,FALSE))
 {
  DebugPrintf (("CopyFile error %d\n",GetLastError()))
  return 0;
 }
 
 return 1;
}

/* Copy a file to the System32\Drivers directory. Used to copy driver */
/* INF files on Windows 98 where we don't have the install routines.  */
int CopyFileToWindowsDriversDirectory (char *SourcePath)
{
 char					Buffer[1024];
 char					*DriverName;

 if (!GetWindowsDirectory(Buffer,sizeof(Buffer)))
 {
  DebugPrintf (("GetWindowsDirectory error %d\n",GetLastError()))
  return 0;
 }
 strcat (Buffer,"\\System32\\Drivers\\");
 DriverName= strrchr (SourcePath,'\\');
 if (!DriverName)
 {
  DebugPrintf (("Cannot determine source .sys name\n"))
  return 0;
 }
 strcat (Buffer,DriverName+1);
 
 DebugPrintf (("Copying Driver File from %s to %s\n",SourcePath,Buffer))
 if (!CopyFile (SourcePath,Buffer,FALSE))
 {
  DebugPrintf (("CopyFile error %d\n",GetLastError()))
  return 0;
 }
 
 return 1;
}


/* Routine to add PPJoy bus enumerator to the system, attempt to copy */
/* the .inf file for the joystick driver to system32, and update the  */
/* drivers used for these devices.                                    */
LONG CALLBACK ViseEntry(LONG lParam1, LPCSTR lpParam2, DWORD dwReserved)
{
 char			FullSourcePath[4096];
 char			*Filename;
 DWORD			OSVersion;
 
 if (!lpParam2)
 {
  DebugPrintf (("Hey, InstallVISE passed us a NULL parameter!\n"))
  return 0;
 }

 DebugPrintf (("Source directory for installation files is: '%s'\n",lpParam2))
 if (!*lpParam2)
 {
  DebugPrintf (("Empty install directory passed; exiting\n"))
  return 0;
 }

 OSVersion= GetVersion();
 DebugPrintf (("OS Version is 0x%X\n",OSVersion))

 Filename= PrepareSourcePath (lpParam2,FullSourcePath);

 if (OSVersion&0x80000000)
 {
  // Copy all the files for Windows 98. Very simple!
  DebugPrintf (("Doing Windows 98 simple installation\n"))

  // Install ports helper
  strcpy (Filename,W98_DRIVER_INFNAME);
  DebugPrintf (("Installation path for port helper inf is: '%s'\n",FullSourcePath))
  CopyFileToWindowsInfDirectory (FullSourcePath);

  // Install Joystick Function driver
  strcpy (Filename,JOY_DRIVER_INFNAME);
  DebugPrintf (("Installation path for joystick driver inf is: '%s'\n",FullSourcePath))
  CopyFileToWindowsInfDirectory (FullSourcePath);

  // Install Joystick Bus enumerator
  strcpy (Filename,BUS_DRIVER_INFNAME);
  DebugPrintf (("Installation path for bus inf is: '%s'\n",FullSourcePath))
  CopyFileToWindowsInfDirectory (FullSourcePath);

  // Install ports helper driver
  strcpy (Filename,W98_DRIVER_FILENAME);
  DebugPrintf (("Installation path for port helper driver is: '%s'\n",FullSourcePath))
  CopyFileToWindowsInfDirectory (FullSourcePath);


  // Install Joystick Bus driver
  strcpy (Filename,BUS_DRIVER_FILENAME);
  DebugPrintf (("Installation path for bus driver is: '%s'\n",FullSourcePath))
  CopyFileToWindowsDriversDirectory (FullSourcePath);

  // Install Joystick driver
  strcpy (Filename,JOY_DRIVER_FILENAME);
  DebugPrintf (("Installation path for joystick driver is: '%s'\n",FullSourcePath))
  CopyFileToWindowsDriversDirectory (FullSourcePath);

  return 1;
 }
 
 // OK, we are on a real operating system - detect if it is a 64 bit operating
 // system. OIf it is a 32 bit OS we use our internal (compiled for 32 bit) routines
 // else we execute the 64 bit compiled exe version of these routines.
 if (IsWOW64())
  return RunHelper ("AddJoyDrivers.dll","HELPER64.EXE",FullSourcePath);
 else
  return InstallPPJoyDrivers(lpParam2);
}

DECLARE_NSIS_FUNC (nsis_Install)
{
 EXDLL_INIT();
 {	// Need to start a new block for this to work in C (instead of C++
  DECLARE_CHAR_PARAM(INFPath)
  SAVE_INT_RESULT(ViseEntry(0,INFPath,0));
 }
}
