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

#include "UtilFunc.h"
#include "Debug.h"

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

// Function to call the ViseEntry entry point in PPortJoy.CPL to uninstall the
// bus driver. Return 1 on success, 0 on error.
int UninstallDrv (char *CPLFullFilename)
{
 typedef LONG (CALLBACK *FUNC_VISEENTRY)(int, char*,int );

 HINSTANCE		hInst;
 FUNC_VISEENTRY	viseentry;
 int			rc;

 // Load the PPJoy control panel into memory
 hInst= LoadLibrary(CPLFullFilename);
 if (!hInst)
 {
  DebugPrintf (("Error loading Control Panel to remove bus driver (%s)\n",CPLFullFilename))
  return 0;
 }

 // Get address of the ViseEntry function
 viseentry= (FUNC_VISEENTRY) GetProcAddress(hInst,("ViseEntry"));
 if (viseentry)
 {
  rc= viseentry(0,"",0);
 }
 else
 {
  DebugPrintf (("Cannot find ViseEntry function entry point in control panel (%s)\n",CPLFullFilename))
  rc= 0;
 }

 FreeLibrary (hInst);
 return rc;
}

// Get the major version number of the running operating system. We will
// map Windows 9x to major version 1 (no version 1 of NT family)
int GetOSMajorVersion (void)
{
 DWORD	OSVersionPack;

 OSVersionPack= GetVersion();
 if (OSVersionPack&0x80000000)
 {
  // DebugPrintf (("  OS family is Windows 9x\n"))
  return 1;						// Hardcoded value for the Windows 9x family
 }
 // DebugPrintf (("  OS family is Windows NT\n"))
 return LOBYTE(LOWORD(OSVersionPack));	// Extract and return major OS version
}

// Get the major version number of the running operating system. We will
// always return a 0 for the Windows 9x family.
int GetOSMinorVersion (void)
{
 DWORD	OSVersionPack;

 OSVersionPack= GetVersion();
 if (OSVersionPack&0x80000000)
 {
  DebugPrintf (("  OS family is Windows 9x\n"))
  return 0;						// Hardcoded value for the Windows 9x family
 }
 DebugPrintf (("  OS family is Windows NT\n"))
 return HIBYTE(LOWORD(OSVersionPack));	// Extract and return major OS version
}


// Test to see if the operating system is supported by PPJoy.
// Return 1 if supported - currently Windows 2000 and up
int IsSupportedOS (void)
{
 return GetOSMajorVersion()>4;			// Anything later than NT4
}

// Get Operating System version and return it as a text string:
// "Win7" "Vista", "WinXP", "Win2K", "WinNT", "Win9x", "Unknown"
char *GetOSName (void)
{
 int	MinorVersion= GetOSMinorVersion();
 int	MajorVersion= GetOSMajorVersion();

 if (MajorVersion>6)					// Unknown new family
  return "Newer";
 if (MajorVersion==6)					// Vista/Win7 family
 {
  if (MinorVersion==0) return "Vista";
  if (MinorVersion==1) return "Win7";
  return "Win7";						// Unknown member of the Vista/Win7 family
 }
 if (MajorVersion==5)					// Win2K or XP
 {
  if (MinorVersion==0) return "Win2K";
  return "WinXP";
 }
 if (MajorVersion>=3)					// First version of WinNT was version 3
  return "WinNT";
 if (MajorVersion>=1)					// Windows 9x family
  return "Win9x";
 return "Unknown";
}

int Need64bitSignedDrivers (void)
{
 if (GetOSMajorVersion()<6) return 0;	// WinXP or earlier - no signed drivers (or can disable)
 
 // Windows Vista or later. We will return the value of IsWow64 - that
 // will return 1 (we are a 32 bit process) if we run on a 64 bit OS.
 return IsWOW64();
}

/******************************************

- SetupeHelper::nsis_UninstallDrv <FilenameAndPathOfControlPanel>

  Call the ViseEntry helper in the PPJoy control panel applet to remove the
  PPJoy bus driver on uninstall

  Return value in $0:
   0: Failure
   1: Success

- SetupHelper::nsis_GetOSMajorVersion

  Return the major version of the currently running OS in $0. Windows 9x is
  mapped to a value of 1.

- SetupHelper::sis_GetOSMinorVersion

  Return the minor version of the currently running OS in $0. Windows 9x is
  mapped to a value of 0.

- SetupeHelper::nsis_IsSupportedOS

  Check to see if the current operating system is supported by PPJoy

  Return value in $0:
   0: Not supported (Win9x or NT4 or earlier)
   1: Supported (Win2K or later)

- SetupHelper::nsis_GetOSName

  Return a text string describing the version of Windows on which we are running

  Return value in $0:
   "Win7"
   "Vista"
   "WinXP"
   "Win2K"
   "WinNT"
   "Win98"
   "Unknown"

- SetupHelper::nsis_Need64bitSignedDrivers

  Checks whether we are running on a 64 bit version of Vista or Windows 7 (and
  later). These OSes needs to have test signing enabled to make allow our
  drivers to load.

  Return value in $0:
   0: Not a 64 bit OS or Windows XP (Server 2003) 64bit (no signing required)
   1: Vista or later 64 bit OS (must enable test signing)

 ******************************************/

DECLARE_NSIS_FUNC (nsis_UninstallDrv)
{
 int	Result;
 EXDLL_INIT();
 {
  DECLARE_CHAR_PARAM(CPLFullFilename)
  
  if (IsWOW64())
   Result= RunHelper ("SetupHelper.dll","UNINST64.EXE","uninstall");
  else
   Result= UninstallDrv(CPLFullFilename);
  SAVE_INT_RESULT(Result);
 }
}

DECLARE_NSIS_FUNC (nsis_GetOSMajorVersion)
{
 EXDLL_INIT();
 SAVE_INT_RESULT(GetOSMajorVersion());
}

DECLARE_NSIS_FUNC (nsis_GetOSMinorVersion)
{
 EXDLL_INIT();
 SAVE_INT_RESULT(GetOSMinorVersion());
}

DECLARE_NSIS_FUNC (nsis_IsSupportedOS)
{
 EXDLL_INIT();
 SAVE_INT_RESULT(IsSupportedOS());
}

DECLARE_NSIS_FUNC (nsis_GetOSName)
{
 EXDLL_INIT();
 SAVE_CHAR_RESULT(GetOSName());
}

DECLARE_NSIS_FUNC (nsis_Need64bitSignedDrivers)
{
 EXDLL_INIT();
 SAVE_INT_RESULT(Need64bitSignedDrivers());
}
