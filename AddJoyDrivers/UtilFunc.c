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


#include "UtilFunc.h"

#include <windows.h>

#include "Debug.h"


typedef DWORD (WINAPI *GetModuleFileNameA_PTR) (
  __in_opt  HMODULE hModule,
  __out     LPTSTR lpFilename,
  __in      DWORD nSize
);

// Get directory from where file was loaded
char *GetMyDirectory (char *MyModuleName, char *Buffer, int BufSize)
{
 HMODULE				hModule;
 char					*TempPtr;
 GetModuleFileNameA_PTR	GetModuleFileNameA_Ptr;

 *Buffer= 0;
 GetModuleFileNameA_Ptr= (GetModuleFileNameA_PTR) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"GetModuleFileNameA");
 if(!GetModuleFileNameA_Ptr)
 {
  DebugPrintf (("GetModuleFileNameA entry point not found %d\n",GetLastError()))
  return Buffer;
 }

 hModule= GetModuleHandle(MyModuleName);
 if (!hModule)
 {
  DebugPrintf (("GetModuleHandle() error %d trying to get handle for '%s'\n",GetLastError(),MyModuleName))
  return Buffer;
 }

 if (!GetModuleFileNameA_Ptr(hModule,Buffer,BufSize))
 {
  DebugPrintf (("GetModuleFileNameA_Ptr error %d\n",GetLastError()))
 }
 
 TempPtr= strrchr(Buffer,'\\');			// Get last backslash in the path name
 if (*TempPtr) *(TempPtr+1)= 0;			// And terminate string just after it

 DebugPrintf (("Directory for AddJoyDrivers.dll is  %s\n",Buffer))
 return Buffer;
}


typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

char IsWOW64 (void)
{
 BOOL	bIsWow64;

 // Attempt to get entry point for IsWow64Process() from KERNEL32
 fnIsWow64Process= (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

 // Return FALSE if we could not find the symbol - can't be a 64 bit system
 if (!fnIsWow64Process)
 {
  DebugPrintf (("IsWow64Process entry point not found %d\n",GetLastError()))
  return 0;
 }

 // Call IsWow64Process() to determine whether we are running on a 64 bit OS
 if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
 {
  DebugPrintf (("IsWow64Process error %d\n",GetLastError()))
  return 0;						// Assume 32 bit process on error
 }

 DebugPrintf (("IsWow64Process result %d\n",bIsWow64))
 return (char) bIsWow64;
}


// Executes a command and return its exit code a return value. A single
// parameter is passed to the executable as argv[0]; and is enclosed in
// quotes. Returns 0 in case of failure.
int RunHelper (char *MyModuleName, char *HelperName, char *Param)
{
 STARTUPINFO			si;
 PROCESS_INFORMATION	pi;
 DWORD					ExitCode;
 char					HelperFQN[_MAX_PATH+1];
 char					Parameters[4096];

 ZeroMemory (&si,sizeof(si));
 si.cb= sizeof(si);
 ZeroMemory (&pi,sizeof(pi));

 // Build string with (self) path and helper name, surrounded by quotes.
 GetMyDirectory (MyModuleName,HelperFQN,sizeof(HelperFQN));
 strcat (HelperFQN,HelperName);

 Parameters[0]= '\"';
 strcpy (Parameters+1,Param);
 strcat (Parameters,"\"");

 DebugPrintf (("Helper for 64bit operation is '%s', parameters '%s'\n",HelperFQN,Parameters))

 // NB: The Unicode version of CreateProcessW expects the commandline parameter to
 // to be writable. We will use the source path buffer. (Only pass source dir)
 if (!CreateProcess(HelperFQN,Parameters,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi))
 {
  DebugPrintf (("CreateProcess() error code %d trying to start helper %s\n",GetLastError(),HelperFQN))
  return 0;		// Create process error - install failed.
 }

 // Wait until child process exits and get the return code
 WaitForSingleObject (pi.hProcess,INFINITE);
 if (!GetExitCodeProcess(pi.hProcess,&ExitCode))
 {
  DebugPrintf (("GetExitCodeProcess() error code %d trying to get helper exit code\n",GetLastError()))
  ExitCode= 0;		// Return a zero on failure
 }

 // Close process and thread handles. 
 CloseHandle (pi.hProcess);
 CloseHandle (pi.hThread);

 DebugPrintf (("Helper process exit code was %d\n",ExitCode))

 return ExitCode; // Return exit code from the called process
}
