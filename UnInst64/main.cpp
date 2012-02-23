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

#include <string.h>

#include "ConfigMan.h"
#include "Debug.h"

int main (int argc, char **argv)
{
 int	Count;
 char	UninstallFlag;

 DebugPrintf (("%d command line arguments:\n",argc))

 UninstallFlag= 0;
 for (Count=0;Count<argc;Count++)
 {
  DebugPrintf (("Argument %d: '%s'\n",Count,argv[Count]))
  if (!_stricmp(argv[Count],"uninstall"))
  {
   DebugPrintf (("Attempting to uninstall joystick devices\n"))
   UninstallFlag= 1;
  }
 }

 if (UninstallFlag) DeleteDevice();

 return 0;
}
