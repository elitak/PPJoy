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

#include "ResourceStrings.h"

#define	MAX_RES_STRING	512

TCHAR	StringBuf1[MAX_RES_STRING*sizeof(TCHAR)];
TCHAR	StringBuf2[MAX_RES_STRING*sizeof(TCHAR)];
TCHAR	StringBuf3[MAX_RES_STRING*sizeof(TCHAR)];

HINSTANCE	hInstance=	NULL;

TCHAR	*GetResStr1 (UINT ResourceID)
{
// if (!hInstance) hInstance= GetModuleHandle(NULL);
 if (!hInstance) hInstance= AfxGetInstanceHandle();
 if (!LoadString (hInstance,ResourceID,StringBuf1,MAX_RES_STRING))
  *StringBuf1= 0;
 return StringBuf1;
}

TCHAR	*GetResStr2 (UINT ResourceID)
{
// if (!hInstance) hInstance= GetModuleHandle(NULL);
 if (!hInstance) hInstance= AfxGetInstanceHandle();
 if (!LoadString (hInstance,ResourceID,StringBuf2,MAX_RES_STRING))
  *StringBuf2= 0;
 return StringBuf2;
}

TCHAR	*GetResStr3 (UINT ResourceID)
{
// if (!hInstance) hInstance= GetModuleHandle(NULL);
 if (!hInstance) hInstance= AfxGetInstanceHandle();
 if (!LoadString (hInstance,ResourceID,StringBuf3,MAX_RES_STRING))
  *StringBuf3= 0;
 return StringBuf3;
}


