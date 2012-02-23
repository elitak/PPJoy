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


#include <time.h>
#include <stdio.h>
#include <string.h>
#include "branding.h"

struct tm *TM;

void CreateBusINF (FILE	*File)
{
 fputs (
"[Version]\n"
"Signature=\"$CHICAGO$\"\n"
"Class=MEDIA\n"
"ClassGUID={4d36e96c-e325-11ce-bfc1-08002be10318}\n"
"Provider=%VENDOR%\n"
"CatalogFile=" BUS_DRIVER_BASENAME ".cat\n"
"LayoutFile=layout.inf\n"
"DriverVer="
 ,File);
 fprintf (File,"%02d/%02d/%04d",TM->tm_mon+1,TM->tm_mday,TM->tm_year+1900);
 fputs (
"," VER_PRODUCTVERSION_STR "\n"
"\n"
 ,File);

 fputs (
"[SourceDisksNames]\n"
"99=%DiskId%,,\n"
"\n"
"[SourceDisksFiles]\n"
BUS_DRIVER_FILENAME " = 99\n"
#ifdef	ENABLE_64BIT_DRIVER
BUS_DRIVER64_FILENAME " = 99\n"
#endif
"\n"
"[DestinationDirs]\n"
"CopyFilesSYS = 12                   ;%SystemRoot%\\system32\\drivers\n"
#ifdef	ENABLE_64BIT_DRIVER
"CopyFilesSYSAMD64 = 12              ;%SystemRoot%\\system32\\drivers\n"
#endif
"CopyFilesDLL = 11                   ;%SystemRoot%\\system or system32 - 98 or Win2000\n"
"DefaultDestDir = 12\n"
"\n"
"[Manufacturer]\n"
"%VENDOR%=Vendor"
#ifdef	ENABLE_64BIT_DRIVER
",NTAMD64"
#endif
"\n"
"\n"
"[Vendor]\n"
";Device Name   Install Section     PnP ID\n"
"%JoystickBus%= JoyBus.Inst,        root\\" BUS_DRIVER_DEVICE_ID "\n"
"\n"
#ifdef	ENABLE_64BIT_DRIVER
"[Vendor.NTAMD64]\n"
"%JoystickBus%= JoyBus.Inst.NTAMD64, root\\" BUS_DRIVER_DEVICE_ID "\n"
"\n"
#endif
"[JoyBus.Inst]\n"
"CopyFiles = CopyFilesSYS, CopyFilesDLL\n"
"AddReg    = JoyBus.AddReg, AddReg.Vendor_Driver\n"
"\n"
"[JoyBus.Inst.NT]\n"
#ifdef	ENABLE_64BIT_DRIVER
"\n"
"[JoyBus.Inst.NTAMD64]\n"
"CopyFiles = CopyFilesSYSAMD64, CopyFilesDLL\n"
"AddReg    = JoyBus.AddReg\n"
"\n"
"[JoyBus.Inst.NTx86]\n"
#endif
"CopyFiles = CopyFilesSYS, CopyFilesDLL\n"
"AddReg    = JoyBus.AddReg\n"
"\n"
 ,File);

 fputs (
"[JoyBus.Inst.NT.Services]\n"		/* Do we need to suppress this line for 64 bit compatible INFs? */
#ifdef	ENABLE_64BIT_DRIVER
"\n"
"[JoyBus.Inst.NTAMD64.Services]\n"
"AddService = " BUS_DRIVER_SERVICENAME ",%SPSVCINST_ASSOCSERVICE%,Vendor_Service_InstAMD64,JoyBus_EventLog_InstAMD64\n"
"\n"
"[JoyBus.Inst.NTx86.Services]\n"
#endif
"AddService = " BUS_DRIVER_SERVICENAME ",%SPSVCINST_ASSOCSERVICE%,Vendor_Service_Inst,JoyBus_EventLog_Inst\n"
"\n"
"[CopyFilesSYS]\n"
BUS_DRIVER_FILENAME "\n"
"\n"
#ifdef	ENABLE_64BIT_DRIVER
"[CopyFilesSYSAMD64]\n"
BUS_DRIVER64_FILENAME "\n"
"\n"
#endif
"[CopyFilesDLL]\n"
"\n"
"[AddReg.Vendor_Driver]\n"
"HKR,,DevLoader,,*ntkern\n"
"HKR,,NTMPDriver,,\"" BUS_DRIVER_FILENAME "\"\n"
"\n"
"[Vendor_Service_Inst]\n"
"DisplayName     = %SvcDesc%\n"
"ServiceType     = 1                  ; SERVICE_KERNEL_DRIVER\n"
"StartType       = 3                  ; SERVICE_DEMAND_START\n"
"ErrorControl    = 0                  ; SERVICE_ERROR_IGNORE\n"
"ServiceBinary   = %12%\\" BUS_DRIVER_FILENAME "\n"
"LoadOrderGroup  = Extended Base\n"
"AddReg          = JoyBus_Service_AddReg\n"
"\n"
#ifdef	ENABLE_64BIT_DRIVER
"[Vendor_Service_InstAMD64]\n"
"DisplayName     = %SvcDesc%\n"
"ServiceType     = 1                  ; SERVICE_KERNEL_DRIVER\n"
"StartType       = 3                  ; SERVICE_DEMAND_START\n"
"ErrorControl    = 0                  ; SERVICE_ERROR_IGNORE\n"
"ServiceBinary   = %12%\\" BUS_DRIVER64_FILENAME "\n"
"LoadOrderGroup  = Extended Base\n"
"AddReg          = JoyBus_Service_AddReg\n"
"\n"
#endif
"[JoyBus_EventLog_Inst]\n"
"AddReg=JoyBus_EventLog_AddReg\n"
"\n"
#ifdef	ENABLE_64BIT_DRIVER
"[JoyBus_EventLog_InstAMD64]\n"
"AddReg=JoyBus_EventLog_AddRegAMD64\n"
"\n" 
#endif
"[JoyBus_EventLog_AddReg]\n"
"HKR,,EventMessageFile,0x00020000,\"\"%SystemRoot%\\System32\\Drivers\\" BUS_DRIVER_FILENAME "\"\"\n"
"HKR,,TypesSupported,0x0001001,7\n"
"\n"
#ifdef	ENABLE_64BIT_DRIVER
"[JoyBus_EventLog_AddRegAMD64]\n"
"HKR,,EventMessageFile,0x00020000,\"\"%SystemRoot%\\System32\\Drivers\\" BUS_DRIVER64_FILENAME "\"\"\n"
"HKR,,TypesSupported,0x0001001,7\n"
"\n"
#endif
 ,File);

 fputs (
"[JoyBus.AddReg]\n"
"\n"
"[JoyBus_Service_AddReg]\n"
BUS_DRIVER_ADDSVCREG
"\n"
"[Strings]\n"
"SPSVCINST_ASSOCSERVICE= 0x00000002\n"
"VENDOR          = \"" COMPANY_NAME "\"\n"
"DiskId          = \"" PRODUCT_NAME_LONG " Installation Disk\"\n"
"SvcDesc         = \"" BUS_DRIVER_DESCRIPTION "\"\n"
"JoystickBus     = \"" BUS_DRIVER_DESCRIPTION "\"\n"
 ,File);
}

void CreateDevINF (FILE	*File)
{
 int	Count;

fputs (
"[Version]\n"
"Signature=\"$CHICAGO$\"\n"
"Class=HIDClass\n"
"ClassGuid={745a17a0-74d3-11d0-b6fe-00a0c90f57da}\n"
"Provider=%VENDOR%\n"
"CatalogFile=" JOY_DRIVER_BASENAME ".cat\n"
"LayoutFile=layout.inf\n"
"DriverVer="
 ,File);
 fprintf (File,"%02d/%02d/%04d",TM->tm_mon+1,TM->tm_mday,TM->tm_year+1900);
 fputs (
"," VER_PRODUCTVERSION_STR "\n"
"\n"
 ,File);

 fputs (
"[SourceDisksNames]\n"
"99=%DiskId%,,\n"
"\n"
"[SourceDisksFiles]\n"
JOY_DRIVER_FILENAME " = 99\n"
#ifdef	ENABLE_64BIT_DRIVER
JOY_DRIVER64_FILENAME " = 99\n"
#endif
"\n"
"[DestinationDirs]\n"
"CopyFilesSYS = 12                   ;%SystemRoot%\\system32\\drivers\n"
#ifdef	ENABLE_64BIT_DRIVER
"CopyFilesSYSAMD64 = 12              ;%SystemRoot%\\system32\\drivers\n"
#endif
"CopyFilesDLL = 11                   ;%SystemRoot%\\system or system32 - 98 or Win2000\n"
"DefaultDestDir = 12\n"
"\n"
"[Manufacturer]\n"
"%VENDOR%=Vendor"
#ifdef	ENABLE_64BIT_DRIVER
",NTAMD64"
#endif
"\n"
"\n"
"[Vendor]\n"
";Device Name    Install Section	    PnP ID\n"
 ,File);

#ifdef JOY_DRIVER_LEGACYINF
  fputs ("%Joystick%=\tJoystick.Inst,\t\tPPJoyBus\\VID_DEAD&PID_BEEF\n",File);
#endif

 for (Count=0;Count<16;Count++)
  fprintf (File,"%%Joystick%d%%=\tJoystick.Inst,\t\t" BUS_DRIVER_DEVICE_ID PNPID_FORMAT "\n",Count,JOY_VENDOR_ID,JOY_PRODUCT_ID_BASE+Count);

#ifdef	ENABLE_64BIT_DRIVER
 fputs ("[Vendor.NTAMD64]\n",File);

#ifdef JOY_DRIVER_LEGACYINF
  fputs ("%Joystick%=\tJoystick.Inst.NTAMD64,\t\tPPJoyBus\\VID_DEAD&PID_BEEF\n",File);
#endif

 for (Count=0;Count<16;Count++)
  fprintf (File,"%%Joystick%d%%=\tJoystick.Inst.NTAMD64,\t\t" BUS_DRIVER_DEVICE_ID PNPID_FORMAT "\n",Count,JOY_VENDOR_ID,JOY_PRODUCT_ID_BASE+Count);
#endif

 fputs (
"\n"
"[Joystick.Inst]\n"
"CopyFiles = CopyFilesSYS, CopyFilesDLL\n"
"AddReg    = Joystick.AddReg, AddReg.Vendor_Driver\n"
"\n"
"[Joystick.Inst.NT]\n"
#ifdef	ENABLE_64BIT_DRIVER
"\n"
"[Joystick.Inst.NTAMD64]\n"
"CopyFiles = CopyFilesSYSAMD64, CopyFilesDLL\n"
"AddReg    = Joystick.AddReg\n"
"\n"
"[Joystick.Inst.NTx86]\n"
#endif
"CopyFiles = CopyFilesSYS, CopyFilesDLL\n"
"AddReg    = Joystick.AddReg\n"
"\n"
"[Joystick.Inst.NT.Services]\n"		/* Do we need to suppress this line for 64 bit compatible INFs? */
#ifdef	ENABLE_64BIT_DRIVER
"\n"
"[Joystick.Inst.NTAMD64.Services]\n"
"AddService = " JOY_DRIVER_SERVICENAME ",%SPSVCINST_ASSOCSERVICE%,Vendor_Service_InstAMD64,Joystick_EventLog_InstAMD64\n"
"\n"
"[Joystick.Inst.NTx86.Services]\n"
#endif
"AddService = " JOY_DRIVER_SERVICENAME ",%SPSVCINST_ASSOCSERVICE%,Vendor_Service_Inst,Joystick_EventLog_Inst\n"
"\n"
"[CopyFilesSYS]\n"
JOY_DRIVER_FILENAME "\n"
";;HIDGame.sys\n"
";;HidClass.sys\n"
";;HidParse.sys\n"
"\n"
#ifdef	ENABLE_64BIT_DRIVER
"[CopyFilesSYSAMD64]\n"
JOY_DRIVER64_FILENAME "\n"
";;HIDGame.sys\n"
";;HidClass.sys\n"
";;HidParse.sys\n"
"\n"
#endif
"[CopyFilesDLL]\n"
";;Hid.dll\n"
"\n"
 ,File);

 fputs (
"[AddReg.Vendor_Driver]\n"
"HKR,,DevLoader,,*ntkern\n"
"HKR,,NTMPDriver,,\"" JOY_DRIVER_FILENAME "\"\n"
"\n"
"[Vendor_Service_Inst]\n"
"DisplayName     = %SvcDesc%\n"
"ServiceType     = 1                  ; SERVICE_KERNEL_DRIVER\n"
"StartType       = 3                  ; SERVICE_DEMAND_START\n"
"ErrorControl    = 0                  ; SERVICE_ERROR_IGNORE\n"
"ServiceBinary   = %12%\\" JOY_DRIVER_FILENAME "\n"
"LoadOrderGroup  = Extended Base\n"
"AddReg          = JoyDev_Service_AddReg\n"
"\n"
#ifdef	ENABLE_64BIT_DRIVER
"[Vendor_Service_InstAMD64]\n"
"DisplayName     = %SvcDesc%\n"
"ServiceType     = 1                  ; SERVICE_KERNEL_DRIVER\n"
"StartType       = 3                  ; SERVICE_DEMAND_START\n"
"ErrorControl    = 0                  ; SERVICE_ERROR_IGNORE\n"
"ServiceBinary   = %12%\\" JOY_DRIVER64_FILENAME "\n"
"LoadOrderGroup  = Extended Base\n"
"AddReg          = JoyDev_Service_AddReg\n"
"\n"
#endif
"[Joystick_EventLog_Inst]\n"
"AddReg=Joystick_EventLog_AddReg\n"
"\n"
#ifdef	ENABLE_64BIT_DRIVER
"[Joystick_EventLog_InstAMD64]\n"
"AddReg=Joystick_EventLog_AddRegAMD64\n"
"\n"
#endif
"[Joystick_EventLog_AddReg]\n"
"HKR,,EventMessageFile,0x00020000,\"\"%SystemRoot%\\System32\\Drivers\\" JOY_DRIVER_FILENAME "\"\"\n"
"HKR,,TypesSupported,0x0001001,7\n"
"\n"
#ifdef	ENABLE_64BIT_DRIVER
"[Joystick_EventLog_AddRegAMD64]\n"
"HKR,,EventMessageFile,0x00020000,\"\"%SystemRoot%\\System32\\Drivers\\" JOY_DRIVER64_FILENAME "\"\"\n"
"HKR,,TypesSupported,0x0001001,7\n"
"\n"
#endif
 ,File);

 fputs (
"[Joystick.AddReg]\n"
"\n"
"[JoyDev_Service_AddReg]\n"
JOY_DRIVER_ADDSVCREG
"\n"
"[Strings]\n"
"SPSVCINST_ASSOCSERVICE= 0x00000002\n"
"KEY_OEM=\"SYSTEM\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\"\n"
"\n"
"VENDOR          = \"" COMPANY_NAME "\"\n"
"DiskId          = \"" PRODUCT_NAME_LONG " Installation Disk\"\n"
"SvcDesc         = \"" JOY_DRIVER_DESCRIPTION "\"\n"
"\n"
,File);

#ifdef JOY_DRIVER_LEGACYINF
  fputs ("Joystick\t= \"" BUS_DRIVER_CHILD_NAME "\n",File);
#endif
 for (Count=0;Count<16;Count++)
  fprintf (File,"Joystick%d\t= \"" BUS_DRIVER_CHILD_NAME " %d\"\n",Count,Count+1);
}

void CreateW98INF (FILE	*File)
{
 fputs (
"[Version]\n"
"Signature=\"$CHICAGO$\"\n"
"Class=MEDIA\n"
"ClassGUID={4d36e96c-e325-11ce-bfc1-08002be10318}\n"
"Provider=%VENDOR%\n"
"LayoutFile=layout.inf\n"
"DriverVer="
 ,File);
 fprintf (File,"%02d/%02d/%04d",TM->tm_mon+1,TM->tm_mday,TM->tm_year+1900);
 fputs (
"," VER_PRODUCTVERSION_STR "\n"
"\n"
 ,File);

 fputs (
"[SourceDisksNames]\n"
"99=%DiskId%,,\n"
"\n"
"[SourceDisksFiles]\n"
W98_DRIVER_FILENAME " = 99\n"
"\n"
"[DestinationDirs]\n"
"CopyFilesSYS = 10,system32\\drivers  ;%SystemRoot%\\system32\\drivers\n"
"DefaultDestDir = 10,system32\\drivers\n"
"CopyFilesDLL = 11                   ;%SystemRoot%\\system or system32 - 98 or Win2000\n"
"\n"
"[ControlFlags]\n"
"ExcludeFromSelect.nt=*\n"
"\n"
"[Manufacturer]\n"
"%VENDOR%=Vendor\n"
"\n"
"[Vendor]\n"
";Device Name	Install Section		PnP ID\n"
"%W98PortsLPT1%=	W98PortsLPT1.Inst,	root\\W98Ports\n"
"%W98PortsLPT2%=	W98PortsLPT2.Inst,	root\\W98Ports\n"
"%W98PortsLPT3%=	W98PortsLPT3.Inst,	root\\W98Ports\n"
 ,File);

 fputs (
"[W98PortsLPT1.Inst]\n"
"CopyFiles = CopyFilesSYS, CopyFilesDLL\n"
"AddReg    = W98PortsLPT1.AddReg, AddReg.Vendor_Driver\n"
"\n"
"[W98PortsLPT2.Inst]\n"
"CopyFiles = CopyFilesSYS, CopyFilesDLL\n"
"AddReg    = W98PortsLPT2.AddReg, AddReg.Vendor_Driver\n"
"\n"
"[W98PortsLPT3.Inst]\n"
"CopyFiles = CopyFilesSYS, CopyFilesDLL\n"
"AddReg    = W98PortsLPT3.AddReg, AddReg.Vendor_Driver\n"
"\n"
"[W98PortsLPT1.AddReg]\n"
"HKR,,PortName,,\"LPT1\"\n"
"\n"
"[W98PortsLPT2.AddReg]\n"
"HKR,,PortName,,\"LPT2\"\n"
"\n"
"[W98PortsLPT3.AddReg]\n"
"HKR,,PortName,,\"LPT3\"\n"
"\n"
"[CopyFilesSYS]\n"
W98_DRIVER_FILENAME "\n"
"\n"
"[CopyFilesDLL]\n"
"\n"
"[AddReg.Vendor_Driver]\n"
"HKR,,DevLoader,,*ntkern\n"
"HKR,,NTMPDriver,,\"" W98_DRIVER_FILENAME "\"\n"
"\n"
"[Strings]\n"
"VENDOR          = \"Deon van der Westhuysen\"\n"
"DiskId          = \"" PRODUCT_NAME_LONG " Installation Disk\"\n"
"W98PortsLPT1    = \"Windows 98 Parallel Port Joystick Support for LPT1\"\n"
"W98PortsLPT2    = \"Windows 98 Parallel Port Joystick Support for LPT2\"\n"
"W98PortsLPT3    = \"Windows 98 Parallel Port Joystick Support for LPT3\"\n"
 ,File);
}

void main (int argc, char **argv)
{
 char	FullPath[1024];
 char	*Filename;
 FILE	*File;
 time_t	Now;

 if (argc>2)
 {
  printf ("Usage: %s [<INF output directory>]\n");
  return;
 }

 Filename= FullPath;
 if (argc==2)
 {
  strcpy (FullPath,argv[1]);
  Filename= FullPath+strlen(FullPath);
  if (*(Filename-1)!='\\')
   *(Filename++)= '\\';
 }

 time (&Now);
 TM= localtime (&Now);

 strcpy (Filename,BUS_DRIVER_INFNAME);
 File= fopen (FullPath,"w");
 if (!File)
 {
  printf ("Error creating Bus driver INF file %s\n",FullPath);
  return;
 }
 CreateBusINF (File);
 fclose (File);

 strcpy (Filename,JOY_DRIVER_INFNAME);
 File= fopen (FullPath,"w");
 if (!File)
 {
  printf ("Error creating Device driver INF file %s\n",FullPath);
  return;
 }
 CreateDevINF (File);
 fclose (File);

 strcpy (Filename,W98_DRIVER_INFNAME);
 File= fopen (FullPath,"w");
 if (!File)
 {
  printf ("Error creating W98 support driver INF file %s\n",FullPath);
  return;
 }
 CreateW98INF (File);
 fclose (File);

 printf ("--- done ---\n");
}
