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


#define _WINMM_
#include <windows.h>
#include <math.h>

#define	PI	3.141

MMRESULT __stdcall joyGetPosEx (UINT uJoyID, LPJOYINFOEX pji)
{
 SYSTEMTIME st;		// system time
 
 GetSystemTime(&st);

 pji->dwXpos= (DWORD) ((cos(st.wMilliseconds/(500/PI))+1)*50);
 pji->dwYpos= (DWORD) ((sin(st.wMilliseconds/(500/PI))+1)*50);
 
 pji->dwZpos= st.wMilliseconds/10;
 pji->dwRpos= 100-st.wMilliseconds/10;

 pji->dwUpos= abs(st.wMilliseconds-500)/5;
 pji->dwVpos= 100-abs(st.wMilliseconds-500)/5;

 return JOYERR_NOERROR;
}

MMRESULT __stdcall joyGetDevCapsA (UINT uJoyID, LPJOYCAPS pjc, UINT cbjc)
{
 memset (pjc,0,sizeof(LPJOYCAPS));

 pjc->wXmin= 
 pjc->wYmin= 
 pjc->wZmin= 
 pjc->wRmin= 
 pjc->wUmin= 
 pjc->wVmin= 0; 

 pjc->wXmax= 
 pjc->wYmax= 
 pjc->wZmax= 
 pjc->wRmax= 
 pjc->wUmax=
 pjc->wVmax= 100; 
 
 return JOYERR_NOERROR;
}

typedef int (_cdecl AsyncDLL_Callback_Proto) (int NumAnalog, int *Analog, int AnalogMin, int AnalogMax, int NumDigital, char *Digital);

int _cdecl AsyncDLL_Callback (int NumAnalog, int *Analog, int AnalogMin, int AnalogMax, int NumDigital, char *Digital);

int __stdcall AsyncDLL_InitDLL (AsyncDLL_Callback_Proto CallbackFunc);
int __stdcall AsyncDLL_Cleanup (void);
