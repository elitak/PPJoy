# PPJoy
Windows LPT/Virtual Joystick driver, originally developed by Deon Van Der Westhuysen. See:<br />
http://ppjoy.blogspot.com

This repository is a safe keeping of the PPJoy source code. It also includes some required fixes
due to last minute changes in the source when originally released that broke compilation.<br />


Compile Environment is:<br />

Windows 7 SP1 x64<br />
User with Admin rights<br />
Download the following below. Use your favourite ISO virtual disk tool to mount the ISO images. [Here's](http://www.ltr-data.se/opencode.html/#ImDisk "ImDisk") a free virtual disk tool if you need.<br />
Install in default directories.<br />

1. Install the Windows 7 Driver Development Kit (DDK). Download ISO from [here](http://download.microsoft.com/download/4/A/2/4A25C7D5-EFBE-4182-B6A9-AE6850409A78/GRMWDK_EN_7600_1.ISO "Win7DDK")

2. Install Visual C 2005 Express. Download ISO from [here](http://download.microsoft.com/download/A/9/1/A91D6B2B-A798-47DF-9C7E-A97854B7DD18/VC.iso "VC2005Express")

3. Install Visual C 2005 Express SP1 updates. Download SP1 from [here](https://download.microsoft.com/download/7/7/3/7737290f-98e8-45bf-9075-85cc6ae34bf1/VS80sp1-KB926748-X86-INTL.exe "VC2005ExpressSP1")
 and SP1 Vista/Win7 update from [here](https://download.microsoft.com/download/c/7/d/c7d9b927-f4e6-4ab2-8399-79a2d5cdfac9/VS80sp1-KB932232-X86-ENU.exe "VC2005ExpressSP1VistaUpdate")

4. Install Visual Studio 2008 in Custom mode and make sure to tick/add the extra component "x64 compiler and linker"<br />
   Download 90 day trial ISO of Professional from [here](http://download.microsoft.com/download/8/1/d/81d3f35e-fa03-485b-953b-ff952e402520/VS2008ProEdition90dayTrialENUX1435622.iso "VisualStudio2008Pro90Daytrial")

5. Install Visual Studio 2008 SP1 Update. Download ISO from [here](https://download.microsoft.com/download/a/3/7/a371b6d1-fc5e-44f7-914c-cb452b4043a9/VS2008SP1ENUX1512962.iso "VisualStudio2008SP1")

6. Install NSIS 2 (Nullsoft Scriptable Install System). Download version 2.xx from [here](https://sourceforge.net/projects/nsis/files/NSIS%202/ "NSIS 2.xx")

7. Install InfoZip command line compression tool. Place the extracted zip.exe somewhere in your path or simply in the PPJoy source root folder. Download from: ftp://ftp.info-zip.org/pub/infozip/win32/zip300xn.zip

8. Download the PPJoy Source and extract it somewhere locally.<br />

9. Reboot the system<br />

When the system restarts, open a Command Prompt. Change directory to the root of the PPJoy source.<br />
Run "make_product.bat"<br />

The build process will take a few minutes and the final compiled file is produced in the following relative folder:<br />
"\_\_\_Product\_\_\_\PPJoySetup-0.8.4.6.exe"
