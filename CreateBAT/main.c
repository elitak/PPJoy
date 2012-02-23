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

void CreateCopyProductsBat (FILE *File)
{
 fputs (
"copy /y PPJoyBus\\objfre_wxp_x86\\i386\\PPJoyBus.sys %1\\" BUS_DRIVER_FILENAME "\n"
"copy /y PPJoyBus\\objchk_wxp_x86\\i386\\PPJoyBus.sys %2\\" BUS_DRIVER_FILENAME "\n"
#ifdef	ENABLE_64BIT_DRIVER
"copy /y PPJoyBus\\objfre_wlh_amd64\\amd64\\PPJoyBus64.sys %1\\" BUS_DRIVER64_FILENAME "\n"
"copy /y PPJoyBus\\objchk_wlh_amd64\\amd64\\PPJoyBus64.sys %2\\" BUS_DRIVER64_FILENAME "\n"
#endif
"\n"
"copy /y PPortJoy\\objfre_wxp_x86\\i386\\PPortJoy.sys %1\\" JOY_DRIVER_FILENAME "\n"
"copy /y PPortJoy\\objchk_wxp_x86\\i386\\PPortJoy.sys %2\\" JOY_DRIVER_FILENAME "\n"
#ifdef	ENABLE_64BIT_DRIVER
"copy /y PPortJoy\\objfre_wlh_amd64\\amd64\\PPortJoy64.sys %1\\" JOY_DRIVER64_FILENAME "\n"
"copy /y PPortJoy\\objchk_wlh_amd64\\amd64\\PPortJoy64.sys %2\\" JOY_DRIVER64_FILENAME "\n"
#endif
"\n"
"copy /y W98Ports\\objfre_wxp_x86\\i386\\W98Ports.sys %1\\" W98_DRIVER_FILENAME "\n"
"copy /y W98Ports\\objchk_wxp_x86\\i386\\W98Ports.sys %2\\" W98_DRIVER_FILENAME "\n"
"\n"
"copy /y PPJoyCpl\\Release\\PPortJoy.cpl %1\\" CPL_APPLET_FILENAME "\n"
"copy /y PPJoyCpl\\Debug\\PPortJoy.cpl %2\\" CPL_APPLET_FILENAME "\n"
"\n"
 ,File);
}


void CreateSignDriversBat (FILE *File)
{
/*
%1 is the DDK root (based on Win 7.0.0 DDK dir structure)
%2 and %3 are the args to pass to SignTool - normally "/s StoreName" or "/f File.pfx"
%4 is the path leading up to the release product directory
%5 is the path leading up to the debug product directory
*/
 fputs (
"ren %4\\" W98_DRIVER_INFNAME " " W98_DRIVER_BASENAME ".TmpINF\n"
"ren %5\\" W98_DRIVER_INFNAME " " W98_DRIVER_BASENAME ".TmpINF\n"
"\n"
"%1\\bin\\selfsign\\inf2cat /driver:%4 /os:2000,XP_X86,Server2003_X86,Vista_X86,Server2008_X86,XP_X64,Server2003_X64,Vista_X64,Server2008_X64,7_X86,7_X64,Server2008R2_X64\n"
"%1\\bin\\selfsign\\inf2cat /driver:%5 /os:2000,XP_X86,Server2003_X86,Vista_X86,Server2008_X86,XP_X64,Server2003_X64,Vista_X64,Server2008_X64,7_X86,7_X64,Server2008R2_X64\n"
"\n"
"ren %4\\" W98_DRIVER_BASENAME ".TmpINF " W98_DRIVER_INFNAME "\n"
"ren %5\\" W98_DRIVER_BASENAME ".TmpINF " W98_DRIVER_INFNAME "\n"
"\n"
"%1\\bin\\x86\\SignTool sign /a %2 %3 %4\\" BUS_DRIVER_FILENAME "\n"
"%1\\bin\\x86\\SignTool sign /a %2 %3 %5\\" BUS_DRIVER_FILENAME "\n"
#ifdef	ENABLE_64BIT_DRIVER
"%1\\bin\\x86\\SignTool sign /a %2 %3 %4\\" BUS_DRIVER64_FILENAME "\n"
"%1\\bin\\x86\\SignTool sign /a %2 %3 %5\\" BUS_DRIVER64_FILENAME "\n"
#endif
"%1\\bin\\x86\\SignTool sign /a %2 %3 %4\\" BUS_DRIVER_BASENAME ".cat\n"
"%1\\bin\\x86\\SignTool sign /a %2 %3 %5\\" BUS_DRIVER_BASENAME ".cat\n"
"\n"
"%1\\bin\\x86\\SignTool sign /a %2 %3 %4\\" JOY_DRIVER_FILENAME "\n"
"%1\\bin\\x86\\SignTool sign /a %2 %3 %5\\" JOY_DRIVER_FILENAME "\n"
#ifdef	ENABLE_64BIT_DRIVER
"%1\\bin\\x86\\SignTool sign /a %2 %3 %4\\" JOY_DRIVER64_FILENAME "\n"
"%1\\bin\\x86\\SignTool sign /a %2 %3 %5\\" JOY_DRIVER64_FILENAME "\n"
#endif
"%1\\bin\\x86\\SignTool sign /a %2 %3 %4\\" JOY_DRIVER_BASENAME ".cat\n"
"%1\\bin\\x86\\SignTool sign /a %2 %3 %5\\" JOY_DRIVER_BASENAME ".cat\n"
"\n"
"%1\\bin\\x86\\SignTool sign /a %2 %3 %4\\" W98_DRIVER_FILENAME "\n"
"%1\\bin\\x86\\SignTool sign /a %2 %3 %5\\" W98_DRIVER_FILENAME "\n"
"\n"
"%1\\bin\\x86\\SignTool sign /a %2 %3 %4\\" CPL_APPLET_FILENAME "\n"
"%1\\bin\\x86\\SignTool sign /a %2 %3 %5\\" CPL_APPLET_FILENAME "\n"
"\n"
 ,File);
}


void CreateIOCTLSampleHeader (FILE *File)
{
 fprintf (File,
"#ifndef\tPPJOY_IOCTL_DEVNAME\n"
"#define\tPPJOY_IOCTL_DEVNAME\t\"\\\\\\\\.\\\\" JOY_IOCTL_DEV_NAME "\"\n"
"#endif\n",1);
}

void main (int argc, char **argv)
{
 char	FullPath[1024];
 char	*Filename;
 FILE	*File;

 if (argc>2)
 {
  printf ("Usage: %s [<BAT output directory>]\n");
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

 strcpy (Filename,"CopyProducts.bat");
 File= fopen (FullPath,"w");
 if (!File)
 {
  printf ("Error creating file %s\n",FullPath);
  return;
 }
 CreateCopyProductsBat (File);
 fclose (File);


 strcpy (Filename,"ppjioctl_devname.h");
 File= fopen (FullPath,"w");
 if (!File)
 {
  printf ("Error creating file %s\n",FullPath);
  return;
 }
 CreateIOCTLSampleHeader (File);
 fclose (File);


 strcpy (Filename,"SignDriverFiles.bat");
 File= fopen (FullPath,"w");
 if (!File)
 {
  printf ("Error creating file %s\n",FullPath);
  return;
 }
 CreateSignDriversBat (File);
 fclose (File);


 

 printf ("--- done ---\n");
}
