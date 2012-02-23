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


#include <stdio.h>
#include <conio.h>

int
reverse_bits(x)
{
	int y, i;

	y = 0;
	for(i=0; i<8; i++) {
		y = (y<<1) + (x&1);
		x >>= 1;
	}
	return y;
}

void main (void)
{
 int	Count;

 Count= 0;
 printf ("{");
 while (1)
 {

  if (reverse_bits(reverse_bits(Count))!=Count)
   printf ("\n\nReversebits failure for %d\n\n",Count);

  printf ("0x%02X",reverse_bits(Count));

  if (Count<255)
   printf (",");
  else
   break;

  Count++;

  if (!(Count%16))
   printf ("\n ");	 
 }
 printf ("};\n");

 // _getch();
}
