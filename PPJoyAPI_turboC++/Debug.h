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


#ifndef __DEBUG_H__
#define	__DEBUG_H__

#ifdef _DEBUG		// Set by Visual Studio when we have a debug target

#include <winbase.h>

#define	DEBUGBUFSIZE	2048

extern char	DebugTextBuffer[DEBUGBUFSIZE];
void DebugBufferPrint (char *Format,...);

#define	DebugPrintf(__x__)	{ DebugBufferPrint __x__; OutputDebugString (DebugTextBuffer); /*MessageBox (NULL,DebugTextBuffer,"debug",MB_OK);*/ }

#else

#define	DebugPrintf(__x__)	{}

#endif

#endif